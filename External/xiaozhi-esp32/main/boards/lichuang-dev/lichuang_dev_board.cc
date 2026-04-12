#include "wifi_board.h"
#include "codecs/box_audio_codec.h"
#include "display/lcd_display.h"
#include "display/emote_display.h"
#include "gp_debug_display.h"
#include "gp_led_matrix_esp32.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "i2c_device.h"
#include "esp32_camera.h"
#include "mcp_server.h"

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_lvgl_port.h>
#include <lvgl.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <optional>

#define TAG "LichuangDevBoard"

namespace {

std::string ToAsciiLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::optional<uint32_t> ParseRgb888(const std::string& text) {
    unsigned int rgb = 0;
    if (std::sscanf(text.c_str(), "#%06x", &rgb) == 1 ||
        std::sscanf(text.c_str(), "0x%06x", &rgb) == 1 ||
        std::sscanf(text.c_str(), "%06x", &rgb) == 1) {
        return static_cast<uint32_t>(rgb & 0xFFFFFFU);
    }
    return std::nullopt;
}

GpColorDebugAnimation ParseAnimationName(const std::string& text) {
    const std::string lowered = ToAsciiLower(text);
    if (lowered == "gradient") {
        return GpColorDebugAnimation::kGradient;
    }
    if (lowered == "pulse") {
        return GpColorDebugAnimation::kPulse;
    }
    return GpColorDebugAnimation::kSolid;
}

const char* ToAnimationName(GpColorDebugAnimation animation) {
    switch (animation) {
    case GpColorDebugAnimation::kGradient:
        return "gradient";
    case GpColorDebugAnimation::kPulse:
        return "pulse";
    case GpColorDebugAnimation::kSolid:
    default:
        return "solid";
    }
}

cJSON* BuildCalculatorResultJson(const std::string& operation, int left, int right, double result) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "operation", operation.c_str());
    cJSON_AddNumberToObject(json, "left", left);
    cJSON_AddNumberToObject(json, "right", right);
    cJSON_AddNumberToObject(json, "result", result);

    char expression[64] = {0};
    const char* op_symbol = operation.c_str();
    if (operation == "add") {
        op_symbol = "+";
    } else if (operation == "subtract") {
        op_symbol = "-";
    } else if (operation == "multiply") {
        op_symbol = "*";
    } else if (operation == "divide") {
        op_symbol = "/";
    } else if (operation == "mod") {
        op_symbol = "%";
    }
    std::snprintf(expression, sizeof(expression), "%d %s %d = %.6g", left, op_symbol, right, result);
    cJSON_AddStringToObject(json, "expression", expression);
    return json;
}

cJSON* BuildDotResultJson(const GpColorDebugState& state) {
    cJSON* json = cJSON_CreateObject();
    char primary[16] = {0};
    std::snprintf(primary, sizeof(primary), "#%06X", static_cast<unsigned int>(state.primary_rgb888 & 0xFFFFFFU));
    cJSON_AddStringToObject(json, "primary_rgb888", primary);
    if (state.has_secondary) {
        char secondary[16] = {0};
        std::snprintf(secondary, sizeof(secondary), "#%06X", static_cast<unsigned int>(state.secondary_rgb888 & 0xFFFFFFU));
        cJSON_AddStringToObject(json, "secondary_rgb888", secondary);
    } else {
        cJSON_AddStringToObject(json, "secondary_rgb888", "");
    }
    cJSON_AddStringToObject(json, "animation", ToAnimationName(state.animation));
    cJSON_AddNumberToObject(json, "size", state.dot_size_px);
    cJSON_AddNumberToObject(json, "duration_ms", state.animation_period_ms);
    cJSON_AddStringToObject(json, "label", state.label.c_str());
    cJSON_AddStringToObject(json, "source", state.source.c_str());
    cJSON_AddStringToObject(json, "transcript", state.transcript.c_str());
    cJSON_AddBoolToObject(json, "applied", true);
    return json;
}

double CalculateResult(const std::string& operation, int left, int right) {
    const std::string lowered = ToAsciiLower(operation);
    if (lowered == "add" || lowered == "+") {
        return static_cast<double>(left + right);
    }
    if (lowered == "subtract" || lowered == "minus" || lowered == "-") {
        return static_cast<double>(left - right);
    }
    if (lowered == "multiply" || lowered == "times" || lowered == "*") {
        return static_cast<double>(left * right);
    }
    if (lowered == "divide" || lowered == "/") {
        if (right == 0) {
            throw std::runtime_error("Division by zero is not allowed");
        }
        return static_cast<double>(left) / static_cast<double>(right);
    }
    if (lowered == "mod" || lowered == "%" || lowered == "modulo") {
        if (right == 0) {
            throw std::runtime_error("Modulo by zero is not allowed");
        }
        return static_cast<double>(left % right);
    }
    throw std::runtime_error("Unsupported calculator operation: " + operation);
}

}

class Pca9557 : public I2cDevice {
public:
    Pca9557(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : I2cDevice(i2c_bus, addr) {
        WriteReg(0x01, 0x03);
        WriteReg(0x03, 0xf8);
    }

    void SetOutputState(uint8_t bit, uint8_t level) {
        uint8_t data = ReadReg(0x01);
        data = (data & ~(1 << bit)) | (level << bit);
        WriteReg(0x01, data);
    }
};

class CustomAudioCodec : public BoxAudioCodec {
private:
    Pca9557* pca9557_;

public:
    CustomAudioCodec(i2c_master_bus_handle_t i2c_bus, Pca9557* pca9557) 
        : BoxAudioCodec(i2c_bus, 
                       AUDIO_INPUT_SAMPLE_RATE, 
                       AUDIO_OUTPUT_SAMPLE_RATE,
                       AUDIO_I2S_GPIO_MCLK, 
                       AUDIO_I2S_GPIO_BCLK, 
                       AUDIO_I2S_GPIO_WS, 
                       AUDIO_I2S_GPIO_DOUT, 
                       AUDIO_I2S_GPIO_DIN,
                       GPIO_NUM_NC, 
                       AUDIO_CODEC_ES8311_ADDR, 
                       AUDIO_CODEC_ES7210_ADDR, 
                       AUDIO_INPUT_REFERENCE),
          pca9557_(pca9557) {
    }

    virtual void EnableOutput(bool enable) override {
        BoxAudioCodec::EnableOutput(enable);
        if (enable) {
            pca9557_->SetOutputState(1, 1);
        } else {
            pca9557_->SetOutputState(1, 0);
        }
    }
};

class LichuangDevBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_dev_handle_t pca9557_handle_;
    Button boot_button_;
    Display* display_;
    Pca9557* pca9557_;
    Esp32Camera* camera_;

    void InitializeI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)1,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));

        // Initialize PCA9557
        pca9557_ = new Pca9557(i2c_bus_, 0x19);
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = GPIO_NUM_40;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = GPIO_NUM_41;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            // During startup (before connected), pressing BOOT button enters Wi-Fi config mode without reboot
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });

#if CONFIG_USE_DEVICE_AEC
        boot_button_.OnDoubleClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateIdle) {
                app.SetAecMode(app.GetAecMode() == kAecOff ? kAecOnDeviceSide : kAecOff);
            }
        });
#endif
    }

    void InitializeSt7789Display() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;
        // 液晶屏控制IO初始化
        ESP_LOGD(TAG, "Install panel IO");
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = GPIO_NUM_NC;
        io_config.dc_gpio_num = GPIO_NUM_39;
        io_config.spi_mode = 2;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        // 初始化液晶屏驱动芯片ST7789
        ESP_LOGD(TAG, "Install LCD driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));
        
        esp_lcd_panel_reset(panel);
        pca9557_->SetOutputState(0, 0);

        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_disp_on_off(panel, true);

    display_ = new GpDebugLcdDisplay(panel_io, panel,
            DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeTouch()
    {
        esp_lcd_touch_handle_t tp;
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = DISPLAY_HEIGHT,
            .y_max = DISPLAY_WIDTH,
            .rst_gpio_num = GPIO_NUM_NC, // Shared with LCD reset
            .int_gpio_num = GPIO_NUM_NC, 
            .levels = {
                .reset = 0,
                .interrupt = 0,
            },
            .flags = {
                .swap_xy = 1,
                .mirror_x = 1,
                .mirror_y = 0,
            },
        };
        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
        tp_io_config.scl_speed_hz = 400000;

        esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle);
        esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp);
        assert(tp);

        /* Add touch input (for selected screen) */
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lv_display_get_default(), 
            .handle = tp,
        };

        if(touch_cfg.disp) {
            lvgl_port_add_touch(&touch_cfg);
        } else {
            ESP_LOGE(TAG, "Touch display is not initialized");
        }
    }

    void InitializeCamera() {
        // Open camera power
        pca9557_->SetOutputState(2, 0);

        static esp_cam_ctlr_dvp_pin_config_t dvp_pin_config = {
            .data_width = CAM_CTLR_DATA_WIDTH_8,
            .data_io = {
                [0] = CAMERA_PIN_D0,
                [1] = CAMERA_PIN_D1,
                [2] = CAMERA_PIN_D2,
                [3] = CAMERA_PIN_D3,
                [4] = CAMERA_PIN_D4,
                [5] = CAMERA_PIN_D5,
                [6] = CAMERA_PIN_D6,
                [7] = CAMERA_PIN_D7,
            },
            .vsync_io = CAMERA_PIN_VSYNC,
            .de_io = CAMERA_PIN_HREF,
            .pclk_io = CAMERA_PIN_PCLK,
            .xclk_io = CAMERA_PIN_XCLK,
        };

        esp_video_init_sccb_config_t sccb_config = {
            .init_sccb = false,
            .i2c_handle = i2c_bus_,
            .freq = 100000,
        };

        esp_video_init_dvp_config_t dvp_config = {
            .sccb_config = sccb_config,
            .reset_pin = CAMERA_PIN_RESET,
            .pwdn_pin = CAMERA_PIN_PWDN,
            .dvp_pin = dvp_pin_config,
            .xclk_freq = XCLK_FREQ_HZ,
        };

        esp_video_init_config_t video_config = {
            .dvp = &dvp_config,
        };

        camera_ = new Esp32Camera(video_config);
    }

    void InitializeTools() {
        auto &mcp_server = McpServer::GetInstance();
        mcp_server.AddTool("self.system.reconfigure_wifi",
            "End this conversation and enter WiFi configuration mode.\n"
            "**CAUTION** You must ask the user to confirm this action.",
            PropertyList(), [this](const PropertyList& properties) {
                EnterWifiConfigMode();
                return true;
            });

        mcp_server.AddTool("self.calculator.calculate",
            "A calculator example tool. Use it for deterministic arithmetic instead of mental math.\n"
            "Supported operations: add, subtract, multiply, divide, mod.",
            PropertyList({
                Property("operation", kPropertyTypeString),
                Property("left", kPropertyTypeInteger),
                Property("right", kPropertyTypeInteger)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                const std::string operation = properties["operation"].value<std::string>();
                const int left = properties["left"].value<int>();
                const int right = properties["right"].value<int>();
                const double result = CalculateResult(operation, left, right);
                return BuildCalculatorResultJson(ToAsciiLower(operation), left, right, result);
            });

        mcp_server.AddTool("self.screen.debug_dot.show",
            "Show or update the debug color dot on screen.\n"
            "Use RGB888 strings such as #FF6600. Animation can be solid, gradient, or pulse.",
            PropertyList({
                Property("primary_rgb888", kPropertyTypeString),
                Property("secondary_rgb888", kPropertyTypeString, std::string("")),
                Property("animation", kPropertyTypeString, std::string("solid")),
                Property("size", kPropertyTypeInteger, 28, 12, 58),
                Property("duration_ms", kPropertyTypeInteger, 1400, 300, 4000),
                Property("label", kPropertyTypeString, std::string("mcp")),
                Property("transcript", kPropertyTypeString, std::string("")),
                Property("source", kPropertyTypeString, std::string("mcp"))
            }),
            [this](const PropertyList& properties) -> ReturnValue {
                auto* debug_display = dynamic_cast<GpDebugLcdDisplay*>(display_);
                if (debug_display == nullptr) {
                    throw std::runtime_error("Current display does not support debug dot control");
                }

                const std::string primary_text = properties["primary_rgb888"].value<std::string>();
                const auto primary_rgb = ParseRgb888(primary_text);
                if (!primary_rgb.has_value()) {
                    throw std::runtime_error("primary_rgb888 must be a RGB888 string like #RRGGBB");
                }

                GpColorDebugState state;
                state.primary_rgb888 = *primary_rgb;
                state.rgb888_text = primary_text;
                state.dot_size_px = static_cast<uint16_t>(properties["size"].value<int>());
                state.animation_period_ms = static_cast<uint16_t>(properties["duration_ms"].value<int>());
                state.animation = ParseAnimationName(properties["animation"].value<std::string>());
                state.label = properties["label"].value<std::string>();
                state.transcript = properties["transcript"].value<std::string>();
                state.source = properties["source"].value<std::string>();

                const std::string secondary_text = properties["secondary_rgb888"].value<std::string>();
                if (!secondary_text.empty()) {
                    const auto secondary_rgb = ParseRgb888(secondary_text);
                    if (!secondary_rgb.has_value()) {
                        throw std::runtime_error("secondary_rgb888 must be empty or a RGB888 string like #RRGGBB");
                    }
                    state.secondary_rgb888 = *secondary_rgb;
                    state.has_secondary = true;
                }

                debug_display->ApplyColorDebugState(state);
                return BuildDotResultJson(state);
            });
    }

public:
    LichuangDevBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        InitializeSpi();
        InitializeSt7789Display();
        InitializeTouch();
        InitializeButtons();
        InitializeCamera();
        InitializeTools();

        GetBacklight()->RestoreBrightness();
    }

    virtual AudioCodec* GetAudioCodec() override {
        static CustomAudioCodec audio_codec(
            i2c_bus_, 
            pca9557_);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual Led* GetLed() override {
        static GpLedMatrixEsp32 led_matrix(i2c_bus_, GP_MATRIX_I2C_ADDRESS, GP_MATRIX_DEFAULT_BRIGHTNESS);
        return &led_matrix;
    }
    
    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }

    virtual Camera* GetCamera() override {
        return camera_;
    }
};

DECLARE_BOARD(LichuangDevBoard);
