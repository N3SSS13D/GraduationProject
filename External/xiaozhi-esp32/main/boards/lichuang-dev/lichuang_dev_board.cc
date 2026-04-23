#include "wifi_board.h"
#include "codecs/box_audio_codec.h"
#include "display/lcd_display.h"
#include "display/emote_display.h"
#include "ui/gp_debug_display.h"
#include "gp_led_matrix_esp32.h"
#include "transport/gp_led_matrix_transport.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "i2c_device.h"
#include "esp32_camera.h"
#include "mcp_server.h"
#include "settings.h"
#include "system_info.h"

#include <esp_log.h>
#include <esp_heap_caps.h>
#include <esp_http_server.h>
#include <esp_lcd_panel_vendor.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_lvgl_port.h>
#include <lvgl.h>
#include <web_socket.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include <string_view>

#define TAG "LichuangDevBoard"

namespace {
constexpr const char* kSnapshotPrefix = "xiaozhi_screen";
constexpr const char* kSnapshotSettingsNamespace = "debug_snapshot";
constexpr const char* kSnapshotUploadUrlKey = "upload_url";
constexpr const char* kDebugWebsocketSettingsNamespace = "debug_ws";
constexpr const char* kDebugWebsocketUrlKey = "url";
constexpr const char* kDebugPreviewUploadPath = "/debug/preview_image";
constexpr const char* kDebugPreviewStatusPath = "/debug/preview_status";
constexpr const char* kSerialSnapCommand = "snap";
constexpr const char* kSerialSnapUrlCommand = "snap_url";
constexpr const char* kSerialDebugWebsocketCommand = "debug_ws";
constexpr const char* kDebugWebsocketDefaultUrl = "ws://49.140.69.242:8766/debug";
constexpr uint16_t kDebugPreviewServerPort = 8781;
constexpr size_t kDebugPreviewMaxImageBytes = 256U * 1024U;
constexpr uint32_t kBtConfigReplyTimeoutMs = 1200;
constexpr uint32_t kBtConfigInquiryTimeoutMs = 5000;
constexpr uint32_t kBtConfigLinkTimeoutMs = 5000;
constexpr uint32_t kBtConfigIdleBreakMs = 60;
constexpr uint32_t kBtConfigSettleDelayMs = 120;
}

namespace {

std::string ToAsciiLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::string TrimAsciiWhitespace(std::string text) {
    const auto is_space = [](unsigned char ch) {
        return std::isspace(ch) != 0;
    };

    text.erase(text.begin(), std::find_if(text.begin(), text.end(), [&](unsigned char ch) {
        return !is_space(ch);
    }));
    text.erase(std::find_if(text.rbegin(), text.rend(), [&](unsigned char ch) {
        return !is_space(ch);
    }).base(), text.end());
    return text;
}

bool StartsWithCommand(const std::string& text, std::string_view prefix) {
    return (text.size() >= prefix.size()) && (text.compare(0, prefix.size(), prefix.data()) == 0);
}

std::string SanitizeAsciiForLog(std::string text) {
    for (char& ch : text) {
        const unsigned char ascii = static_cast<unsigned char>(ch);
        if ((ascii < 0x20U) && (ch != '\r') && (ch != '\n') && (ch != '\t')) {
            ch = '.';
        }
    }
    return text;
}

bool ReplyContainsOk(const std::string& response) {
    return response.find("OK") != std::string::npos;
}

void LogBtConfigResponse(const std::string& response) {
    std::string remaining = SanitizeAsciiForLog(response);
    size_t offset = 0;

    if (remaining.empty()) {
        ESP_LOGW(TAG, "[BT_CFG] << <no response>");
        return;
    }

    while (offset < remaining.size()) {
        size_t line_end = remaining.find_first_of("\r\n", offset);
        std::string line;

        if (line_end == std::string::npos) {
            line = TrimAsciiWhitespace(remaining.substr(offset));
            offset = remaining.size();
        } else {
            line = TrimAsciiWhitespace(remaining.substr(offset, line_end - offset));
            offset = remaining.find_first_not_of("\r\n", line_end);
            if (offset == std::string::npos) {
                offset = remaining.size();
            }
        }

        if (!line.empty()) {
            ESP_LOGI(TAG, "[BT_CFG] << %s", line.c_str());
        }
    }
}

std::optional<std::string> NormalizeHc05Address(const std::string& address_text) {
    std::string hex_digits;

    for (char ch : address_text) {
        if (std::isxdigit(static_cast<unsigned char>(ch)) == 0) {
            continue;
        }
        hex_digits.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }

    if (hex_digits.size() != 12U) {
        return std::nullopt;
    }

    return hex_digits.substr(0, 4) + "," + hex_digits.substr(4, 2) + "," + hex_digits.substr(6, 6);
}

class Hc05UartConfigurator {
public:
    Hc05UartConfigurator(int uart_port, int tx_gpio, int rx_gpio, uint32_t baudrate)
        : uart_port_(static_cast<uart_port_t>(uart_port)),
          tx_gpio_(tx_gpio),
          rx_gpio_(rx_gpio),
          current_baudrate_(baudrate) {
    }

    ~Hc05UartConfigurator() {
        Close();
    }

    bool Open() {
        esp_err_t err;

        err = uart_driver_install(uart_port_, 1024, 1024, 0, nullptr, 0);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "[BT_CFG] uart_driver_install failed: %s", esp_err_to_name(err));
            return false;
        }

        open_ = true;
        err = uart_set_pin(uart_port_, tx_gpio_, rx_gpio_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "[BT_CFG] uart_set_pin failed: %s", esp_err_to_name(err));
            Close();
            return false;
        }

        if (!ConfigureLocalUart(current_baudrate_)) {
            Close();
            return false;
        }

        ESP_LOGI(TAG,
                 "[BT_CFG] uart ready: port=%d tx=%d rx=%d baud=%lu",
                 static_cast<int>(uart_port_),
                 tx_gpio_,
                 rx_gpio_,
                 static_cast<unsigned long>(current_baudrate_));
        return true;
    }

    void Close() {
        if (!open_) {
            return;
        }

        uart_driver_delete(uart_port_);
        open_ = false;
    }

    uint32_t GetCurrentBaudrate() const {
        return current_baudrate_;
    }

    std::string SendCommand(const std::string& command, uint32_t timeout_ms = kBtConfigReplyTimeoutMs) {
        std::string wire_text = command;
        int written = 0;

        if (!open_) {
            return {};
        }

        wire_text.append("\r\n");
        uart_flush_input(uart_port_);
        ESP_LOGI(TAG, "[BT_CFG] >> %s", command.c_str());

        written = uart_write_bytes(uart_port_, wire_text.data(), wire_text.size());
        if (written != static_cast<int>(wire_text.size())) {
            ESP_LOGW(TAG,
                     "[BT_CFG] short write: expected=%u actual=%d",
                     static_cast<unsigned int>(wire_text.size()),
                     written);
        }

        uart_wait_tx_done(uart_port_, pdMS_TO_TICKS(timeout_ms));
        const std::string response = ReadResponse(timeout_ms);
        LogBtConfigResponse(response);
        return response;
    }

    bool ProbeAtWithRetry(uint8_t max_attempts) {
        uint8_t attempt = 0;

        for (attempt = 0; attempt < max_attempts; ++attempt) {
            if (ReplyContainsOk(SendCommand("AT"))) {
                ESP_LOGI(TAG, "[BT_CFG] probe=ok attempt=%u", static_cast<unsigned int>(attempt + 1));
                return true;
            }
            ESP_LOGW(TAG, "[BT_CFG] probe retry=%u", static_cast<unsigned int>(attempt + 1));
            vTaskDelay(pdMS_TO_TICKS(kBtConfigSettleDelayMs));
        }

        return false;
    }

    bool SendAndVerify(const std::string& command, const std::string& query) {
        const bool ok = ReplyContainsOk(SendCommand(command));
        vTaskDelay(pdMS_TO_TICKS(kBtConfigSettleDelayMs));
        SendCommand(query);
        return ok;
    }

    bool SwitchLocalBaudrate(uint32_t baudrate) {
        return ConfigureLocalUart(baudrate);
    }

    bool SwitchRemoteAndLocalBaud(uint32_t baudrate) {
        std::string command = "AT+UART=" + std::to_string(static_cast<unsigned long>(baudrate)) + ",0,0";
        const bool ok = ReplyContainsOk(SendCommand(command));
        bool reset_ok = false;

        if (!ok) {
            ESP_LOGW(TAG, "[BT_CFG] remote baud switch rejected: %lu", static_cast<unsigned long>(baudrate));
            return false;
        }

        reset_ok = ReplyContainsOk(SendCommand("AT+RESET"));
        if (!reset_ok) {
            ESP_LOGW(TAG, "[BT_CFG] remote reset rejected before baud switch: %lu", static_cast<unsigned long>(baudrate));
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(kBtConfigSettleDelayMs));
        if (!ConfigureLocalUart(baudrate)) {
            return false;
        }
        return true;
    }

    bool BindRemoteAddress(const std::string& address) {
        return SendAndVerify("AT+BIND=" + address, "AT+BIND?");
    }

private:
    bool ConfigureLocalUart(uint32_t baudrate) {
        uart_config_t config = {};
        esp_err_t err;

        config.baud_rate = static_cast<int>(baudrate);
        config.data_bits = UART_DATA_8_BITS;
        config.parity = UART_PARITY_DISABLE;
        config.stop_bits = UART_STOP_BITS_1;
        config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        config.rx_flow_ctrl_thresh = 0;
        config.source_clk = UART_SCLK_DEFAULT;

        err = uart_param_config(uart_port_, &config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "[BT_CFG] uart_param_config failed: %s", esp_err_to_name(err));
            return false;
        }

        err = uart_flush_input(uart_port_);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "[BT_CFG] uart_flush_input failed: %s", esp_err_to_name(err));
            return false;
        }

        current_baudrate_ = baudrate;
        ESP_LOGI(TAG, "[BT_CFG] local baud=%lu", static_cast<unsigned long>(baudrate));
        return true;
    }

    std::string ReadResponse(uint32_t timeout_ms) {
        std::array<uint8_t, 96> chunk = {};
        std::string response;
        TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);
        TickType_t last_rx_tick = 0;
        bool received_any = false;

        while (xTaskGetTickCount() < deadline) {
            const int bytes_read = uart_read_bytes(uart_port_,
                                                   chunk.data(),
                                                   chunk.size(),
                                                   pdMS_TO_TICKS(20));
            if (bytes_read > 0) {
                response.append(reinterpret_cast<const char*>(chunk.data()),
                                static_cast<size_t>(bytes_read));
                last_rx_tick = xTaskGetTickCount();
                received_any = true;
                continue;
            }

            if (received_any &&
                ((xTaskGetTickCount() - last_rx_tick) >= pdMS_TO_TICKS(kBtConfigIdleBreakMs))) {
                break;
            }
        }

        return response;
    }

    uart_port_t uart_port_;
    int tx_gpio_ = UART_PIN_NO_CHANGE;
    int rx_gpio_ = UART_PIN_NO_CHANGE;
    uint32_t current_baudrate_ = 0;
    bool open_ = false;
};

std::optional<uint32_t> ParseRgb888(const std::string& text) {
    unsigned int rgb = 0;
    if (std::sscanf(text.c_str(), "#%06x", &rgb) == 1 ||
        std::sscanf(text.c_str(), "0x%06x", &rgb) == 1 ||
        std::sscanf(text.c_str(), "%06x", &rgb) == 1) {
        return static_cast<uint32_t>(rgb & 0xFFFFFFU);
    }
    return std::nullopt;
}

std::optional<std::array<uint8_t, GP_MATRIX_RGB332_FRAME_SIZE>> ParseRgb332FrameHex(const std::string& text) {
    std::array<uint8_t, GP_MATRIX_RGB332_FRAME_SIZE> frame = {};
    std::string hex_digits;

    hex_digits.reserve(text.size());
    for (char ch : text) {
        if (std::isxdigit(static_cast<unsigned char>(ch)) != 0) {
            hex_digits.push_back(ch);
        }
    }

    if (hex_digits.size() != (GP_MATRIX_RGB332_FRAME_SIZE * 2U)) {
        return std::nullopt;
    }

    for (size_t index = 0; index < frame.size(); ++index) {
        unsigned int value = 0;

        if (std::sscanf(hex_digits.substr(index * 2U, 2U).c_str(), "%2x", &value) != 1) {
            return std::nullopt;
        }
        frame[index] = static_cast<uint8_t>(value & 0xFFU);
    }

    return frame;
}

std::optional<std::array<uint16_t, GP_MATRIX_HEIGHT>> ParseMatrixBitmapRowsHex(const std::string& text) {
    std::array<uint16_t, GP_MATRIX_HEIGHT> rows = {};
    std::string hex_digits;

    hex_digits.reserve(text.size());
    for (char ch : text) {
        if (std::isxdigit(static_cast<unsigned char>(ch)) != 0) {
            hex_digits.push_back(ch);
        }
    }

    if (hex_digits.size() != (GP_MATRIX_HEIGHT * 4U)) {
        return std::nullopt;
    }

    for (size_t index = 0; index < rows.size(); ++index) {
        unsigned int value = 0;

        if (std::sscanf(hex_digits.substr(index * 4U, 4U).c_str(), "%4x", &value) != 1) {
            return std::nullopt;
        }
        rows[index] = static_cast<uint16_t>(value & 0xFFFFU);
    }

    return rows;
}

uint8_t Rgb888ToRgb332(uint32_t rgb888) {
    const uint8_t red = static_cast<uint8_t>((rgb888 >> 16) & 0xFFU);
    const uint8_t green = static_cast<uint8_t>((rgb888 >> 8) & 0xFFU);
    const uint8_t blue = static_cast<uint8_t>(rgb888 & 0xFFU);

    return static_cast<uint8_t>((red & 0xE0U) | ((green >> 3) & 0x1CU) | ((blue >> 6) & 0x03U));
}

std::array<uint8_t, GP_MATRIX_RGB332_FRAME_SIZE> BuildRgb332FrameFromBitmapRows(
    const std::array<uint16_t, GP_MATRIX_HEIGHT>& rows,
    uint32_t primary_rgb888) {
    std::array<uint8_t, GP_MATRIX_RGB332_FRAME_SIZE> frame = {};
    const uint8_t primary_rgb332 = Rgb888ToRgb332(primary_rgb888);

    for (size_t row = 0; row < GP_MATRIX_HEIGHT; ++row) {
        for (size_t column = 0; column < GP_MATRIX_WIDTH; ++column) {
            const bool enabled = ((rows[row] >> (15U - column)) & 0x0001U) != 0U;
            frame[row * GP_MATRIX_WIDTH + column] = enabled ? primary_rgb332 : 0U;
        }
    }

    return frame;
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

const char* ToPresetName(GpColorDebugPreset preset) {
    switch (preset) {
    case GpColorDebugPreset::kDiamond:
        return "diamond";
    case GpColorDebugPreset::kCross:
        return "cross";
    case GpColorDebugPreset::kJluEmblem:
        return "jlu_emblem";
    case GpColorDebugPreset::kPythonDemo:
        return "python_demo";
    case GpColorDebugPreset::kScrollSubtitle:
        return "scroll_subtitle";
    case GpColorDebugPreset::kSolid:
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

cJSON* BuildMatrixFrameResultJson(const char* preset,
                                  const char* frame_hex,
                                  const char* bitmap_rows_hex,
                                  const char* primary_rgb888,
                                  bool applied,
                                  const char* source,
                                  const char* transcript) {
    cJSON* json = cJSON_CreateObject();

    cJSON_AddStringToObject(json, "preset", preset != nullptr ? preset : "");
    cJSON_AddStringToObject(json, "frame_rgb332_hex", frame_hex != nullptr ? frame_hex : "");
    cJSON_AddStringToObject(json, "bitmap_rows_hex", bitmap_rows_hex != nullptr ? bitmap_rows_hex : "");
    cJSON_AddStringToObject(json, "primary_rgb888", primary_rgb888 != nullptr ? primary_rgb888 : "");
    cJSON_AddNumberToObject(json, "width", GP_MATRIX_WIDTH);
    cJSON_AddNumberToObject(json, "height", GP_MATRIX_HEIGHT);
    cJSON_AddBoolToObject(json, "applied", applied);
    cJSON_AddStringToObject(json, "source", source != nullptr ? source : "mcp");
    cJSON_AddStringToObject(json, "transcript", transcript != nullptr ? transcript : "");
    return json;
}

cJSON* BuildPreviewFetchResultJson(const char* url,
                                   size_t bytes,
                                   bool fetched,
                                   const char* source,
                                   const char* transcript) {
    cJSON* json = cJSON_CreateObject();

    cJSON_AddStringToObject(json, "url", url != nullptr ? url : "");
    cJSON_AddNumberToObject(json, "bytes", static_cast<double>(bytes));
    cJSON_AddBoolToObject(json, "fetched", fetched);
    cJSON_AddStringToObject(json, "source", source != nullptr ? source : "mcp");
    cJSON_AddStringToObject(json, "transcript", transcript != nullptr ? transcript : "");
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
    static constexpr uint32_t kDebugCommandTaskStackWords = 8192;
    static constexpr UBaseType_t kDebugCommandTaskPriority = 2;
    static constexpr size_t kDebugCommandQueueLength = 6;
    static constexpr uint32_t kSerialCommandTaskStackWords = 4096;
    static constexpr UBaseType_t kSerialCommandTaskPriority = 1;
    static constexpr size_t kSerialCommandBufferSize = 256;
    static constexpr uint32_t kBtBridgeTaskStackWords = 4096;
    static constexpr UBaseType_t kBtBridgeTaskPriority = 1;
    static constexpr uint32_t kBtBridgeStartDelayMs = 1200;
    static constexpr uint32_t kBtBridgeTickMs = 1000;

    struct DebugCommand {
        enum class Type : uint8_t {
            kApplyMatrixState = 0,
            kCaptureSnapshot = 1,
            kSendDebugWebsocketMessage = 2,
        };

        Type type = Type::kApplyMatrixState;
        GpColorDebugState state;
        int quality = 50;
        std::string text;
    };

    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_dev_handle_t pca9557_handle_;
    Button boot_button_;
    Display* display_;
    Pca9557* pca9557_;
    Esp32Camera* camera_;
    std::unique_ptr<GpLedMatrixEsp32> led_matrix_;
    QueueHandle_t debug_command_queue_ = nullptr;
    httpd_handle_t debug_preview_http_server_ = nullptr;
    uint32_t last_debug_snapshot_sequence_ = 0;
    std::mutex debug_snapshot_mutex_;
    std::mutex debug_snapshot_upload_mutex_;
    std::mutex debug_preview_status_mutex_;
    std::mutex debug_websocket_mutex_;
    std::mutex debug_websocket_status_mutex_;
    bool debug_snapshot_upload_in_progress_ = false;
    bool debug_preview_online_ = false;
    bool debug_websocket_connected_ = false;
    size_t last_debug_preview_bytes_ = 0;
    std::string debug_preview_status_text_ = "HTTP preview init";
    std::string debug_websocket_status_text_ = "Debug WS init";
    uint32_t bt_transport_baudrate_ = GP_MATRIX_BT_UART_DATA_BAUDRATE;
    std::unique_ptr<WebSocket> debug_websocket_;

    /* Reuse the existing debug-menu link panel to surface Wi-Fi preview status updates. */
    void UpdateDebugPreviewStatus(bool online, const std::string& status_text, size_t image_bytes = 0U) {
        auto* debug_display = dynamic_cast<GpDebugLcdDisplay*>(display_);

        {
            std::lock_guard<std::mutex> lock(debug_preview_status_mutex_);
            debug_preview_online_ = online;
            debug_preview_status_text_ = status_text;
            if (image_bytes > 0U) {
                last_debug_preview_bytes_ = image_bytes;
            }
        }

        if (debug_display != nullptr) {
            debug_display->ApplyAiLinkStatus(online, status_text);
        }
    }

    void UpdateDebugWebsocketStatus(bool connected, const std::string& status_text) {
        {
            std::lock_guard<std::mutex> lock(debug_websocket_status_mutex_);
            debug_websocket_connected_ = connected;
            debug_websocket_status_text_ = status_text;
        }

        ESP_LOGI(TAG, "[DBG_WS] %s", status_text.c_str());
    }

    static std::string GetCompiledDebugSnapshotUploadUrl() {
        return TrimAsciiWhitespace(GP_DEBUG_SNAPSHOT_DEFAULT_UPLOAD_URL);
    }

    static std::string GetConfiguredDebugWebsocketUrl() {
        Settings settings(kDebugWebsocketSettingsNamespace, false);
        return settings.GetString(kDebugWebsocketUrlKey, "");
    }

    static std::string GetEffectiveDebugWebsocketUrl() {
        const std::string configured_url = TrimAsciiWhitespace(GetConfiguredDebugWebsocketUrl());

        if (!configured_url.empty()) {
            return configured_url;
        }
        return kDebugWebsocketDefaultUrl;
    }

    static void SetDebugWebsocketUrl(const std::string& url) {
        Settings settings(kDebugWebsocketSettingsNamespace, true);

        if (url.empty()) {
            settings.EraseKey(kDebugWebsocketUrlKey);
            return;
        }

        settings.SetString(kDebugWebsocketUrlKey, url);
    }

    static std::string GetDebugSnapshotUploadUrl() {
        Settings settings(kSnapshotSettingsNamespace, false);
        return settings.GetString(kSnapshotUploadUrlKey, "");
    }

    static void SetDebugSnapshotUploadUrl(const std::string& url) {
        Settings settings(kSnapshotSettingsNamespace, true);

        if (url.empty()) {
            settings.EraseKey(kSnapshotUploadUrlKey);
            return;
        }

        settings.SetString(kSnapshotUploadUrlKey, url);
    }

    static cJSON* BuildDebugSnapshotUploadUrlJson() {
        const std::string url = GetDebugSnapshotUploadUrl();
        cJSON* json = cJSON_CreateObject();

        cJSON_AddStringToObject(json, "upload_url", url.c_str());
        cJSON_AddBoolToObject(json, "configured", !url.empty());
        cJSON_AddStringToObject(json, "compiled_default_upload_url", GetCompiledDebugSnapshotUploadUrl().c_str());
        return json;
    }

    static cJSON* BuildDebugSnapshotCaptureJson(const std::string& status_text, int quality) {
        const std::string upload_url = GetDebugSnapshotUploadUrl();
        cJSON* json = cJSON_CreateObject();

        cJSON_AddBoolToObject(json, "accepted", status_text == "Uploading snapshot...");
        cJSON_AddStringToObject(json, "status", status_text.c_str());
        cJSON_AddNumberToObject(json, "quality", quality);
        cJSON_AddStringToObject(json, "upload_url", upload_url.c_str());
        return json;
    }

    static esp_err_t HandleDebugPreviewStatus(httpd_req_t* req) {
        auto* self = static_cast<LichuangDevBoard*>(req->user_ctx);
        cJSON* json = cJSON_CreateObject();
        char* response_text = nullptr;
        bool online = false;
        size_t image_bytes = 0U;
        std::string status_text = "HTTP preview unavailable";

        if (self != nullptr) {
            std::lock_guard<std::mutex> lock(self->debug_preview_status_mutex_);
            online = self->debug_preview_online_;
            image_bytes = self->last_debug_preview_bytes_;
            status_text = self->debug_preview_status_text_;
        }

        /* The host bridge polls this endpoint before its automatic startup upload. */
        cJSON_AddBoolToObject(json, "ready", true);
        cJSON_AddStringToObject(json, "feature", "debug_preview");
        cJSON_AddBoolToObject(json, "online", online);
        cJSON_AddNumberToObject(json, "last_image_bytes", static_cast<double>(image_bytes));
        cJSON_AddStringToObject(json, "status_text", status_text.c_str());
        response_text = cJSON_PrintUnformatted(json);
        cJSON_Delete(json);

        httpd_resp_set_type(req, "application/json");
        if (response_text == nullptr) {
            return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to encode preview status");
        }

        esp_err_t result = httpd_resp_sendstr(req, response_text);
        cJSON_free(response_text);
        return result;
    }

    static esp_err_t HandleDebugPreviewUpload(httpd_req_t* req) {
        auto* self = static_cast<LichuangDevBoard*>(req->user_ctx);
        auto* lvgl_display = (self == nullptr) ? nullptr : dynamic_cast<LvglDisplay*>(self->display_);
        size_t total_read = 0;
        char* image_data = nullptr;
        int bytes_read = 0;

        if ((self == nullptr) || (lvgl_display == nullptr)) {
            return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Display is not ready");
        }

        if ((req->content_len <= 0) || (static_cast<size_t>(req->content_len) > kDebugPreviewMaxImageBytes)) {
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid image size");
        }

        image_data = static_cast<char*>(heap_caps_malloc(static_cast<size_t>(req->content_len), MALLOC_CAP_8BIT));
        if (image_data == nullptr) {
            return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        }

        while (total_read < static_cast<size_t>(req->content_len)) {
            bytes_read = httpd_req_recv(req,
                                        image_data + total_read,
                                        static_cast<size_t>(req->content_len) - total_read);
            if (bytes_read <= 0) {
                heap_caps_free(image_data);
                return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read request body");
            }
            total_read += static_cast<size_t>(bytes_read);
        }

        lvgl_display->SetPreviewImage(std::make_unique<LvglAllocatedImage>(image_data, total_read));
        self->UpdateDebugPreviewStatus(true,
                           "HTTP preview image received",
                           total_read);
        ESP_LOGI(TAG, "[DBG_HTTP] received preview image bytes=%u", static_cast<unsigned int>(total_read));

        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, "{\"accepted\":true}");
    }

    void EnsureDebugPreviewHttpServer() {
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        httpd_uri_t preview_uri = {};
        httpd_uri_t status_uri = {};

        if (debug_preview_http_server_ != nullptr) {
            return;
        }

        config.server_port = kDebugPreviewServerPort;
        config.max_open_sockets = 4;

        preview_uri.uri = kDebugPreviewUploadPath;
        preview_uri.method = HTTP_POST;
        preview_uri.handler = &LichuangDevBoard::HandleDebugPreviewUpload;
        preview_uri.user_ctx = this;

        status_uri.uri = kDebugPreviewStatusPath;
        status_uri.method = HTTP_GET;
        status_uri.handler = &LichuangDevBoard::HandleDebugPreviewStatus;
        status_uri.user_ctx = this;

        if (httpd_start(&debug_preview_http_server_, &config) != ESP_OK) {
            debug_preview_http_server_ = nullptr;
            ESP_LOGE(TAG, "[DBG_HTTP] failed to start preview server on port %u", kDebugPreviewServerPort);
            return;
        }

        httpd_register_uri_handler(debug_preview_http_server_, &preview_uri);
        httpd_register_uri_handler(debug_preview_http_server_, &status_uri);
        UpdateDebugPreviewStatus(true, "HTTP preview waiting for image");
        ESP_LOGI(TAG,
                 "[DBG_HTTP] preview server ready: POST %s GET %s port=%u",
                 kDebugPreviewUploadPath,
                 kDebugPreviewStatusPath,
                 kDebugPreviewServerPort);
    }

    static void LogDebugSnapshotUploadUrl(const char* reason) {
        const std::string current_url = GetDebugSnapshotUploadUrl();
        const std::string default_url = GetCompiledDebugSnapshotUploadUrl();

        ESP_LOGI(TAG, "[%s] snapshot upload url=%s", reason, current_url.empty() ? "<empty>" : current_url.c_str());
        if (!default_url.empty()) {
            ESP_LOGI(TAG, "[%s] compiled default snapshot upload url=%s", reason, default_url.c_str());
        }
    }

    void LogDebugWebsocketStatus(const char* reason) {
        bool connected = false;
        std::string runtime_status;
        const std::string configured_url = GetConfiguredDebugWebsocketUrl();
        const std::string effective_url = GetEffectiveDebugWebsocketUrl();

        {
            std::lock_guard<std::mutex> lock(debug_websocket_status_mutex_);
            connected = debug_websocket_connected_;
            runtime_status = debug_websocket_status_text_;
        }

        ESP_LOGI(TAG,
                 "[%s] debug_ws configured_url=%s",
                 reason,
                 configured_url.empty() ? "<empty>" : configured_url.c_str());
        ESP_LOGI(TAG, "[%s] debug_ws effective_url=%s", reason, effective_url.c_str());
        ESP_LOGI(TAG,
                 "[%s] debug_ws connected=%s status=%s",
                 reason,
                 connected ? "true" : "false",
                 runtime_status.c_str());
    }

    void CloseDebugWebsocket() {
        std::lock_guard<std::mutex> lock(debug_websocket_mutex_);

        debug_websocket_.reset();
        UpdateDebugWebsocketStatus(false, "Debug WS closed");
    }

    static std::string BuildDebugWebsocketHelloMessage() {
        cJSON* root = cJSON_CreateObject();
        char* json_text = nullptr;
        std::string message;

        cJSON_AddStringToObject(root, "type", "hello");
        cJSON_AddStringToObject(root, "role", "ai_debug_client");
        cJSON_AddStringToObject(root, "transport", "debug_websocket");
        cJSON_AddStringToObject(root, "device_id", SystemInfo::GetMacAddress().c_str());
        cJSON_AddStringToObject(root, "client_id", Board::GetInstance().GetUuid().c_str());

        json_text = cJSON_PrintUnformatted(root);
        if (json_text != nullptr) {
            message = json_text;
            cJSON_free(json_text);
        }
        cJSON_Delete(root);
        return message;
    }

    static std::string BuildDebugWebsocketTouchMessage(const GpColorDebugState& state, bool request_pattern_draw) {
        cJSON* root = cJSON_CreateObject();
        char* json_text = nullptr;
        char primary_rgb888[16] = {0};
        std::string message;

        std::snprintf(primary_rgb888,
                      sizeof(primary_rgb888),
                      "#%06X",
                      static_cast<unsigned int>(state.primary_rgb888 & 0xFFFFFFU));
        cJSON_AddStringToObject(root,
                                "type",
                                request_pattern_draw ? "draw_random_pattern_request" : "touch_state_update");
        cJSON_AddStringToObject(root, "source", state.source.c_str());
        cJSON_AddStringToObject(root, "transcript", state.transcript.c_str());
        cJSON_AddStringToObject(root, "preset", ToPresetName(state.preset));
        cJSON_AddStringToObject(root, "animation", ToAnimationName(state.animation));
        cJSON_AddStringToObject(root, "primary_rgb888", primary_rgb888);
        cJSON_AddStringToObject(root, "label", state.label.c_str());
        cJSON_AddNumberToObject(root, "size", state.dot_size_px);
        cJSON_AddNumberToObject(root, "duration_ms", state.animation_period_ms);

        if (state.has_secondary) {
            char secondary_rgb888[16] = {0};

            std::snprintf(secondary_rgb888,
                          sizeof(secondary_rgb888),
                          "#%06X",
                          static_cast<unsigned int>(state.secondary_rgb888 & 0xFFFFFFU));
            cJSON_AddStringToObject(root, "secondary_rgb888", secondary_rgb888);
        }

        json_text = cJSON_PrintUnformatted(root);
        if (json_text != nullptr) {
            message = json_text;
            cJSON_free(json_text);
        }
        cJSON_Delete(root);
        return message;
    }

    bool EnsureDebugWebsocketConnected() {
        std::lock_guard<std::mutex> lock(debug_websocket_mutex_);
        auto network = Board::GetInstance().GetNetwork();
        const std::string url = GetEffectiveDebugWebsocketUrl();

        if ((debug_websocket_ != nullptr) && debug_websocket_->IsConnected()) {
            return true;
        }

        if (network == nullptr) {
            UpdateDebugWebsocketStatus(false, "Debug WS network unavailable");
            return false;
        }

        debug_websocket_.reset();
        debug_websocket_ = network->CreateWebSocket(1);
        if (debug_websocket_ == nullptr) {
            UpdateDebugWebsocketStatus(false, "Debug WS create failed");
            return false;
        }

        debug_websocket_->SetHeader("Device-Id", SystemInfo::GetMacAddress().c_str());
        debug_websocket_->SetHeader("Client-Id", Board::GetInstance().GetUuid().c_str());
        debug_websocket_->SetHeader("X-Debug-Transport", "preview");
        debug_websocket_->OnData([this](const char* data, size_t len, bool binary) {
            this->HandleDebugWebsocketMessage(data, len, binary);
        });
        debug_websocket_->OnDisconnected([this]() {
            UpdateDebugWebsocketStatus(false, "Debug WS disconnected");
        });

        ESP_LOGI(TAG, "[DBG_WS] connecting url=%s", url.c_str());
        if (!debug_websocket_->Connect(url.c_str())) {
            ESP_LOGE(TAG,
                     "[DBG_WS] connect failed url=%s code=%d",
                     url.c_str(),
                     debug_websocket_->GetLastError());
            UpdateDebugWebsocketStatus(false, "Debug WS connect failed");
            debug_websocket_.reset();
            return false;
        }

        UpdateDebugWebsocketStatus(true, "Debug WS connected");
        const std::string hello_message = BuildDebugWebsocketHelloMessage();
        if (!hello_message.empty()) {
            debug_websocket_->Send(hello_message);
            ESP_LOGI(TAG, "[DBG_WS] tx %s", SanitizeAsciiForLog(hello_message).c_str());
        }

        return true;
    }

    bool SendDebugWebsocketMessage(const std::string& message) {
        if (!EnsureDebugWebsocketConnected()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(debug_websocket_mutex_);
        if ((debug_websocket_ == nullptr) || !debug_websocket_->IsConnected()) {
            UpdateDebugWebsocketStatus(false, "Debug WS send skipped: not connected");
            return false;
        }
        if (!debug_websocket_->Send(message)) {
            UpdateDebugWebsocketStatus(false, "Debug WS send failed");
            ESP_LOGE(TAG, "[DBG_WS] send failed payload=%s", SanitizeAsciiForLog(message).c_str());
            return false;
        }

        UpdateDebugWebsocketStatus(true, "Debug WS message sent");
        ESP_LOGI(TAG, "[DBG_WS] tx %s", SanitizeAsciiForLog(message).c_str());
        return true;
    }

    void HandleDebugWebsocketMessage(const char* data, size_t len, bool binary) {
        const std::string text(data != nullptr ? std::string(data, len) : std::string());
        cJSON* root = nullptr;

        if (binary) {
            ESP_LOGW(TAG, "[DBG_WS] binary payload is not supported bytes=%u", static_cast<unsigned int>(len));
            return;
        }

        ESP_LOGI(TAG, "[DBG_WS] rx %s", SanitizeAsciiForLog(text).c_str());
        root = cJSON_Parse(text.c_str());
        if (root == nullptr) {
            ESP_LOGW(TAG, "[DBG_WS] invalid json payload");
            return;
        }

        const auto* type = cJSON_GetObjectItem(root, "type");
        if (!cJSON_IsString(type)) {
            ESP_LOGW(TAG, "[DBG_WS] missing type field");
            cJSON_Delete(root);
            return;
        }

        if (std::strcmp(type->valuestring, "hello") == 0) {
            UpdateDebugWebsocketStatus(true, "Debug WS hello received");
            cJSON_Delete(root);
            return;
        }

        if (std::strcmp(type->valuestring, "ack") == 0) {
            UpdateDebugWebsocketStatus(true, "Debug WS ack received");
            cJSON_Delete(root);
            return;
        }

        if (std::strcmp(type->valuestring, "matrix_pattern_result") == 0) {
            const auto* bitmap_rows_hex = cJSON_GetObjectItem(root, "bitmap_rows_hex");
            const auto* primary_rgb888 = cJSON_GetObjectItem(root, "primary_rgb888");
            const auto* background_rgb888 = cJSON_GetObjectItem(root, "background_rgb888");
            const auto* transcript = cJSON_GetObjectItem(root, "transcript");
            uint32_t background_rgb = 0x000000U;

            if (!cJSON_IsString(bitmap_rows_hex) || !cJSON_IsString(primary_rgb888)) {
                ESP_LOGW(TAG, "[DBG_WS] matrix_pattern_result missing bitmap/color fields");
                cJSON_Delete(root);
                return;
            }

            const auto bitmap_rows = ParseMatrixBitmapRowsHex(bitmap_rows_hex->valuestring);
            const auto primary_rgb = ParseRgb888(primary_rgb888->valuestring);
            if (!bitmap_rows.has_value() || !primary_rgb.has_value()) {
                ESP_LOGW(TAG, "[DBG_WS] invalid matrix_pattern_result payload");
                cJSON_Delete(root);
                return;
            }

            if (cJSON_IsString(background_rgb888)) {
                const auto parsed_background = ParseRgb888(background_rgb888->valuestring);
                if (parsed_background.has_value()) {
                    background_rgb = *parsed_background;
                }
            }

            const std::array<uint16_t, GP_MATRIX_HEIGHT> scheduled_rows = *bitmap_rows;
            const uint32_t scheduled_primary_rgb = *primary_rgb;
            const uint32_t scheduled_background_rgb = background_rgb;
            const std::string transcript_text = cJSON_IsString(transcript) ? transcript->valuestring : "";

            Application::GetInstance().Schedule([this,
                                                 scheduled_rows,
                                                 scheduled_primary_rgb,
                                                 scheduled_background_rgb,
                                                 transcript_text]() {
                auto* debug_display = dynamic_cast<GpDebugLcdDisplay*>(display_);
                auto* matrix_led = led_matrix_.get();
                bool led_forwarded = false;

                if (debug_display != nullptr) {
                    debug_display->ApplyMatrixBitmapPreview(scheduled_rows,
                                                            scheduled_primary_rgb,
                                                            scheduled_background_rgb);
                }

                if (matrix_led != nullptr) {
                    led_forwarded = matrix_led->ShowBitmapFrame(scheduled_rows.data(),
                                                                scheduled_rows.size(),
                                                                scheduled_primary_rgb,
                                                                scheduled_background_rgb,
                                                                kGpMatrixModeSolidFrame);
                    if (!led_forwarded) {
                        ESP_LOGW(TAG, "[DBG_WS] failed to relay pattern to LED side over Bluetooth");
                    }
                }

                if (display_ != nullptr) {
                    display_->ShowNotification(led_forwarded ? "WS pattern relayed" : "WS pattern preview only", 1500);
                    if (!transcript_text.empty()) {
                        display_->SetChatMessage("system", transcript_text.c_str());
                    }
                }
            });
            UpdateDebugWebsocketStatus(true, "Debug WS pattern received");
            cJSON_Delete(root);
            return;
        }

        ESP_LOGW(TAG, "[DBG_WS] unsupported message type=%s", type->valuestring);
        cJSON_Delete(root);
    }

    static bool ResetDebugSnapshotUploadUrlToCompiledDefault() {
        const std::string default_url = GetCompiledDebugSnapshotUploadUrl();

        if (default_url.empty()) {
            return false;
        }

        SetDebugSnapshotUploadUrl(default_url);
        return true;
    }

    static void EnsureDefaultDebugSnapshotUploadUrl() {
        const std::string current_url = GetDebugSnapshotUploadUrl();
        const std::string default_url = GetCompiledDebugSnapshotUploadUrl();

        if (!current_url.empty()) {
            ESP_LOGI(TAG, "Snapshot upload URL already configured: %s", current_url.c_str());
            return;
        }
        if (default_url.empty()) {
            ESP_LOGI(TAG, "No compiled default snapshot upload URL configured");
            return;
        }

        SetDebugSnapshotUploadUrl(default_url);
        ESP_LOGI(TAG, "Applied compiled default snapshot upload URL: %s", default_url.c_str());
    }

    static bool UploadSnapshotToHttp(const std::string& png_data,
                                     int width,
                                     int height,
                                     int quality,
                                     uint32_t sequence,
                                     std::string* error_message) {
        const std::string upload_url = GetDebugSnapshotUploadUrl();
        auto network = Board::GetInstance().GetNetwork();
        std::string response;

        if (upload_url.empty()) {
            if (error_message != nullptr) {
                *error_message = "Snapshot upload URL is not configured";
            }
            return false;
        }

        if (network == nullptr) {
            if (error_message != nullptr) {
                *error_message = "Network interface is not available";
            }
            return false;
        }

        auto http = network->CreateHttp(3);
        http->SetHeader("Device-Id", SystemInfo::GetMacAddress().c_str());
        http->SetHeader("Client-Id", Board::GetInstance().GetUuid().c_str());
        http->SetHeader("Content-Type", "image/png");
        http->SetHeader("Transfer-Encoding", "chunked");
        http->SetHeader("X-Snapshot-Width", std::to_string(width));
        http->SetHeader("X-Snapshot-Height", std::to_string(height));
        http->SetHeader("X-Snapshot-Quality", std::to_string(quality));
        http->SetHeader("X-Snapshot-Sequence", std::to_string(static_cast<unsigned long>(sequence)));
        http->SetHeader("X-Snapshot-Prefix", kSnapshotPrefix);

        if (!http->Open("POST", upload_url)) {
            if (error_message != nullptr) {
                *error_message = "Failed to open snapshot upload URL";
            }
            return false;
        }

        http->Write(png_data.data(), png_data.size());
        http->Write("", 0);

        if (http->GetStatusCode() != 200) {
            response = http->ReadAll();
            http->Close();
            if (error_message != nullptr) {
                *error_message = "Snapshot upload HTTP status " + std::to_string(http->GetStatusCode()) + ": " + response;
            }
            return false;
        }

        response = http->ReadAll();
        http->Close();
        ESP_LOGI(TAG, "Snap upload HTTP response: %s", response.c_str());
        return true;
    }

    bool FetchPreviewImageFromHttp(const std::string& image_url,
                                   std::string* error_message,
                                   size_t* image_bytes) {
        auto network = Board::GetInstance().GetNetwork();
        auto* lvgl_display = dynamic_cast<LvglDisplay*>(display_);
        std::string response;
        char* image_data = nullptr;
        int status_code = 0;

        if (image_url.empty()) {
            if (error_message != nullptr) {
                *error_message = "Preview image URL is required";
            }
            return false;
        }

        if (network == nullptr) {
            if (error_message != nullptr) {
                *error_message = "Network interface is not available";
            }
            return false;
        }

        if (lvgl_display == nullptr) {
            if (error_message != nullptr) {
                *error_message = "Display does not support preview image rendering";
            }
            return false;
        }

        auto http = network->CreateHttp(3);
        http->SetHeader("Accept", "image/png, image/jpeg");

        if (!http->Open("GET", image_url)) {
            if (error_message != nullptr) {
                *error_message = "Failed to open preview image URL";
            }
            return false;
        }

        response = http->ReadAll();
        status_code = http->GetStatusCode();
        http->Close();

        if (status_code != 200) {
            if (error_message != nullptr) {
                *error_message = "Preview fetch HTTP status " + std::to_string(status_code) + ": " + response;
            }
            return false;
        }

        if (response.empty()) {
            if (error_message != nullptr) {
                *error_message = "Preview image response is empty";
            }
            return false;
        }

        if (response.size() > kDebugPreviewMaxImageBytes) {
            if (error_message != nullptr) {
                *error_message = "Preview image exceeds maximum size";
            }
            return false;
        }

        image_data = static_cast<char*>(heap_caps_malloc(response.size(), MALLOC_CAP_8BIT));
        if (image_data == nullptr) {
            if (error_message != nullptr) {
                *error_message = "Out of memory while storing preview image";
            }
            return false;
        }

        std::memcpy(image_data, response.data(), response.size());
        lvgl_display->SetPreviewImage(std::make_unique<LvglAllocatedImage>(image_data, response.size()));
        UpdateDebugPreviewStatus(true, "HTTP preview image fetched", response.size());
        ESP_LOGI(TAG,
                 "[DBG_HTTP] fetched preview image url=%s bytes=%u",
                 image_url.c_str(),
                 static_cast<unsigned int>(response.size()));

        if (image_bytes != nullptr) {
            *image_bytes = response.size();
        }
        return true;
    }

    void FinishDebugSnapshotUpload(const std::string& notify_text) {
        ESP_LOGI(TAG, "Snap upload finished: %s", notify_text.c_str());
        std::lock_guard<std::mutex> lock(debug_snapshot_upload_mutex_);
        debug_snapshot_upload_in_progress_ = false;
    }

    void RunDebugSnapshotUpload(int quality) {
        std::string png_data;
        std::string error_message;
        std::string notify_text = "Snapshot upload failed";
        bool upload_ok = false;
        uint32_t sequence = 0;
        int width = 0;
        int height = 0;

        ESP_LOGI(TAG, "Snap upload start: quality=%d", quality);
        auto* lvgl_display = dynamic_cast<LvglDisplay*>(display_);
        if (lvgl_display == nullptr) {
            error_message = "Current display does not support PNG snapshots";
            goto cleanup;
        }

        if (!lvgl_display->SnapshotToPng(png_data)) {
            error_message = "Failed to capture PNG snapshot";
            goto cleanup;
        }
        ESP_LOGI(TAG, "Snap upload captured PNG: %u bytes", static_cast<unsigned int>(png_data.size()));

        width = display_->width();
        height = display_->height();
        {
            std::lock_guard<std::mutex> lock(debug_snapshot_mutex_);
            last_debug_snapshot_sequence_ += 1U;
            sequence = last_debug_snapshot_sequence_;
        }

        upload_ok = UploadSnapshotToHttp(png_data, width, height, quality, sequence, &error_message);
        if (upload_ok) {
            notify_text = "Snapshot uploaded to HTTP";
            ESP_LOGI(TAG, "Snap upload HTTP sent: seq=%lu width=%d height=%d",
                static_cast<unsigned long>(sequence), width, height);
        }

cleanup:
        if (!upload_ok && !error_message.empty()) {
            notify_text = error_message;
            ESP_LOGE(TAG, "Snap upload failed: %s", error_message.c_str());
        }

        FinishDebugSnapshotUpload(notify_text);
    }

    static void RunDebugCommandTask(void* task_parameter) {
        auto* self = static_cast<LichuangDevBoard*>(task_parameter);
        DebugCommand* command = nullptr;

        if ((self == nullptr) || (self->debug_command_queue_ == nullptr)) {
            vTaskDelete(nullptr);
            return;
        }

        while (true) {
            command = nullptr;
            if (xQueueReceive(self->debug_command_queue_, &command, portMAX_DELAY) != pdTRUE) {
                continue;
            }

            std::unique_ptr<DebugCommand> holder(command);
            if (command == nullptr) {
                continue;
            }

            switch (command->type) {
            case DebugCommand::Type::kApplyMatrixState:
                if (self->led_matrix_ != nullptr) {
                    self->led_matrix_->ShowDebugState(command->state);
                }
                break;
            case DebugCommand::Type::kCaptureSnapshot:
                self->RunDebugSnapshotUpload(command->quality);
                break;
            case DebugCommand::Type::kSendDebugWebsocketMessage:
                self->SendDebugWebsocketMessage(command->text);
                break;
            default:
                break;
            }
        }
    }

    bool EnqueueDebugCommand(std::unique_ptr<DebugCommand> command) {
        DebugCommand* raw_command = nullptr;

        if ((debug_command_queue_ == nullptr) || (command == nullptr)) {
            return false;
        }

        raw_command = command.get();
        if (xQueueSend(debug_command_queue_, &raw_command, 0) != pdTRUE) {
            ESP_LOGW(TAG, "Debug command queue is full");
            return false;
        }

        command.release();
        return true;
    }

    bool QueueMatrixDebugState(const GpColorDebugState& state) {
        auto command = std::make_unique<DebugCommand>();

        command->type = DebugCommand::Type::kApplyMatrixState;
        command->state = state;
        return EnqueueDebugCommand(std::move(command));
    }

    std::string QueueDebugSnapshotCapture(int quality) {
        auto command = std::make_unique<DebugCommand>();
        bool busy = false;

        {
            std::lock_guard<std::mutex> lock(debug_snapshot_upload_mutex_);
            if (debug_snapshot_upload_in_progress_) {
                busy = true;
            } else {
                debug_snapshot_upload_in_progress_ = true;
            }
        }

        if (busy) {
            return "Snapshot upload busy";
        }

        command->type = DebugCommand::Type::kCaptureSnapshot;
        command->quality = quality;
        if (!EnqueueDebugCommand(std::move(command))) {
            {
                std::lock_guard<std::mutex> lock(debug_snapshot_upload_mutex_);
                debug_snapshot_upload_in_progress_ = false;
            }
            ESP_LOGE(TAG, "Snap upload queue failed");
            return "Snapshot queue failed";
        }

        ESP_LOGI(TAG, "Snap upload queued");
        return "Uploading snapshot...";
    }

    std::string QueueDebugWebsocketTouchCommand(const GpColorDebugState& state, bool request_pattern_draw) {
        auto command = std::make_unique<DebugCommand>();

        command->type = DebugCommand::Type::kSendDebugWebsocketMessage;
        command->text = BuildDebugWebsocketTouchMessage(state, request_pattern_draw);
        if (command->text.empty()) {
            ESP_LOGW(TAG, "[DBG_WS] failed to build touch websocket payload");
            return "Debug WS payload failed";
        }
        if (!EnqueueDebugCommand(std::move(command))) {
            ESP_LOGW(TAG, "[DBG_WS] queue failed");
            return "Debug WS queue failed";
        }

        ESP_LOGI(TAG,
                 "[DBG_WS] queued type=%s transcript=%s",
                 request_pattern_draw ? "draw_random_pattern_request" : "touch_state_update",
                 state.transcript.c_str());
        return request_pattern_draw ? "Debug WS draw request queued" : "Debug WS state queued";
    }

    void InitializeDebugCommandTask() {
        BaseType_t task_created;

        debug_command_queue_ = xQueueCreate(kDebugCommandQueueLength, sizeof(DebugCommand*));
        if (debug_command_queue_ == nullptr) {
            ESP_LOGW(TAG, "Failed to create debug command queue");
            return;
        }

        task_created = xTaskCreate(
            &LichuangDevBoard::RunDebugCommandTask,
            "dbg_cmd_worker",
            kDebugCommandTaskStackWords,
            this,
            kDebugCommandTaskPriority,
            nullptr);
        if (task_created != pdPASS) {
            vQueueDelete(debug_command_queue_);
            debug_command_queue_ = nullptr;
            ESP_LOGW(TAG, "Failed to create debug command task");
        }
    }

    void ShowSerialCommandNotification(const std::string& text) {
        Application::GetInstance().Schedule([this, text]() {
            if (display_ != nullptr) {
                display_->ShowNotification(text.c_str(), 1600);
            }
        });
    }

    void HandleSerialDebugCommand(std::string line) {
        std::string argument_text;
        std::string command;
        int snap_quality = 50;
        const std::string help_text =
            "snap | snap <quality> | snap_url get | set <url> | clear | reset | help";
        const std::string ws_help_text =
            "debug_ws get | status | set <url> | clear | close | help";

        line = TrimAsciiWhitespace(std::move(line));
        if (line.empty()) {
            return;
        }
        if (StartsWithCommand(line, kSerialSnapCommand) &&
            ((line.size() == std::strlen(kSerialSnapCommand)) || std::isspace(static_cast<unsigned char>(line[std::strlen(kSerialSnapCommand)])))) {
            argument_text = TrimAsciiWhitespace(line.substr(std::strlen(kSerialSnapCommand)));
            if (!argument_text.empty()) {
                char extra = '\0';
                if ((std::sscanf(argument_text.c_str(), "%d %c", &snap_quality, &extra) != 1) ||
                    (snap_quality < 0) || (snap_quality > 95)) {
                    ESP_LOGW(TAG, "Invalid snap quality: %s", argument_text.c_str());
                    ESP_LOGI(TAG, "Usage: snap | snap <0..95>");
                    return;
                }
            }

            command = QueueDebugSnapshotCapture(snap_quality);
            ESP_LOGI(TAG, "Serial command triggered snapshot: %s", command.c_str());
            return;
        }

        if (!StartsWithCommand(line, kSerialSnapUrlCommand)) {
            if (!StartsWithCommand(line, kSerialDebugWebsocketCommand)) {
                return;
            }

            argument_text = TrimAsciiWhitespace(line.substr(std::strlen(kSerialDebugWebsocketCommand)));
            if (argument_text.empty() || argument_text == "help") {
                ESP_LOGI(TAG, "Serial command: %s", ws_help_text.c_str());
                LogDebugWebsocketStatus("serial_help");
                return;
            }

            if (argument_text == "get" || argument_text == "status") {
                LogDebugWebsocketStatus(argument_text == "get" ? "serial_get" : "serial_status");
                ShowSerialCommandNotification("Debug WS status printed");
                return;
            }

            if (argument_text == "clear") {
                SetDebugWebsocketUrl("");
                CloseDebugWebsocket();
                LogDebugWebsocketStatus("serial_clear");
                ShowSerialCommandNotification("Debug WS URL cleared");
                return;
            }

            if (argument_text == "close") {
                CloseDebugWebsocket();
                LogDebugWebsocketStatus("serial_close");
                ShowSerialCommandNotification("Debug WS closed");
                return;
            }

            if (StartsWithCommand(argument_text, "set ")) {
                command = TrimAsciiWhitespace(argument_text.substr(4));
                if (command.empty()) {
                    ESP_LOGW(TAG, "Serial command set failed: debug ws url is empty");
                    ESP_LOGI(TAG, "Usage: %s", ws_help_text.c_str());
                    return;
                }
                SetDebugWebsocketUrl(command);
                CloseDebugWebsocket();
                LogDebugWebsocketStatus("serial_set");
                ShowSerialCommandNotification("Debug WS URL updated");
                return;
            }

            ESP_LOGW(TAG, "Unknown serial command: %s", line.c_str());
            ESP_LOGI(TAG, "Usage: %s", ws_help_text.c_str());
            return;
        }

        argument_text = TrimAsciiWhitespace(line.substr(std::strlen(kSerialSnapUrlCommand)));
        if (argument_text.empty() || argument_text == "help") {
            ESP_LOGI(TAG, "Serial command: %s", help_text.c_str());
            LogDebugSnapshotUploadUrl("serial_help");
            return;
        }

        if (argument_text == "get") {
            LogDebugSnapshotUploadUrl("serial_get");
            ShowSerialCommandNotification("Snap URL printed to serial");
            return;
        }

        if (argument_text == "clear") {
            SetDebugSnapshotUploadUrl("");
            ESP_LOGI(TAG, "Serial command cleared snapshot upload URL");
            LogDebugSnapshotUploadUrl("serial_clear");
            ShowSerialCommandNotification("Snap URL cleared");
            return;
        }

        if (argument_text == "reset") {
            if (ResetDebugSnapshotUploadUrlToCompiledDefault()) {
                ESP_LOGI(TAG, "Serial command restored compiled default snapshot upload URL");
                LogDebugSnapshotUploadUrl("serial_reset");
                ShowSerialCommandNotification("Snap URL reset");
            } else {
                ESP_LOGW(TAG, "Serial command reset failed: no compiled default snapshot upload URL");
                ShowSerialCommandNotification("No default Snap URL");
            }
            return;
        }

        if (StartsWithCommand(argument_text, "set ")) {
            command = TrimAsciiWhitespace(argument_text.substr(4));
            if (command.empty()) {
                ESP_LOGW(TAG, "Serial command set failed: url is empty");
                ESP_LOGI(TAG, "Usage: %s", help_text.c_str());
                return;
            }
            SetDebugSnapshotUploadUrl(command);
            ESP_LOGI(TAG, "Serial command set snapshot upload URL: %s", command.c_str());
            LogDebugSnapshotUploadUrl("serial_set");
            ShowSerialCommandNotification("Snap URL updated");
            return;
        }

        ESP_LOGW(TAG, "Unknown serial command: %s", line.c_str());
        ESP_LOGI(TAG, "Usage: %s", help_text.c_str());
    }

    static void RunSerialCommandTask(void* task_parameter) {
        auto* self = static_cast<LichuangDevBoard*>(task_parameter);
        char buffer[kSerialCommandBufferSize] = {0};

        if (self == nullptr) {
            vTaskDelete(nullptr);
            return;
        }

        ESP_LOGI(TAG,
                 "Serial debug command ready: snap | snap <quality> | snap_url get | set <url> | clear | reset | help | debug_ws get | status | set <url> | clear | close | help");
        while (true) {
            if (std::fgets(buffer, sizeof(buffer), stdin) == nullptr) {
                clearerr(stdin);
                vTaskDelay(pdMS_TO_TICKS(150));
                continue;
            }
            self->HandleSerialDebugCommand(buffer);
        }
    }

    void InitializeSerialDebugCommands() {
        BaseType_t task_created;

        task_created = xTaskCreate(
            &LichuangDevBoard::RunSerialCommandTask,
            "dbg_serial_cmd",
            kSerialCommandTaskStackWords,
            this,
            kSerialCommandTaskPriority,
            nullptr);
        if (task_created != pdPASS) {
            ESP_LOGW(TAG, "Failed to create serial debug command task");
        }
    }

    static void RunBluetoothBridgeTask(void* task_parameter) {
        auto* self = static_cast<LichuangDevBoard*>(task_parameter);
        uint8_t led_index = 0U;

        if ((self == nullptr) || (self->led_matrix_ == nullptr)) {
            vTaskDelete(nullptr);
            return;
        }

        vTaskDelay(pdMS_TO_TICKS(kBtBridgeStartDelayMs));
        self->led_matrix_->RunStartupLinkTest();

        while (true) {
            if (!self->led_matrix_->SendBtDebugLedCommand(led_index)) {
                ESP_LOGW(TAG, "P2 LED chase command failed: led=%u", static_cast<unsigned int>(led_index));
            }

            led_index = static_cast<uint8_t>((led_index + 1U) % (GP_MATRIX_DEBUG_LED_MAX_INDEX + 1U));
            vTaskDelay(pdMS_TO_TICKS(kBtBridgeTickMs));
        }
    }

    void InitializeBluetoothBridgeTask() {
        BaseType_t task_created;

        task_created = xTaskCreate(
            &LichuangDevBoard::RunBluetoothBridgeTask,
            "bt_led_bridge",
            kBtBridgeTaskStackWords,
            this,
            kBtBridgeTaskPriority,
            nullptr);
        if (task_created != pdPASS) {
            ESP_LOGW(TAG, "Failed to create Bluetooth bridge task");
        }
    }

    uint32_t ConfigureBluetoothModule() {
        Hc05UartConfigurator configurator(GP_MATRIX_BT_UART_PORT,
                                          GP_MATRIX_BT_UART_TX_PIN,
                                          GP_MATRIX_BT_UART_RX_PIN,
                                          GP_MATRIX_BT_UART_AT_BAUDRATE);
        bool sequence_ok = true;
        std::optional<std::string> remote_address;

        if (!configurator.Open()) {
            ESP_LOGW(TAG, "[BT_CFG] configure skipped: uart init failed");
            return GP_MATRIX_BT_UART_DATA_BAUDRATE;
        }

        remote_address = NormalizeHc05Address(GP_MATRIX_BT_REMOTE_ADDRESS);
        if (!remote_address.has_value()) {
            ESP_LOGW(TAG, "[BT_CFG] invalid remote address: %s", GP_MATRIX_BT_REMOTE_ADDRESS);
            return GP_MATRIX_BT_UART_DATA_BAUDRATE;
        }

        if (!configurator.ProbeAtWithRetry(3)) {
            ESP_LOGI(TAG, "[BT_CFG] probe timeout, skip AT setup and switch local baud to data mode");
            if (!configurator.SwitchLocalBaudrate(GP_MATRIX_BT_UART_DATA_BAUDRATE)) {
                ESP_LOGW(TAG, "[BT_CFG] local baud switch failed after probe timeout");
            }
            return configurator.GetCurrentBaudrate();
        }

        configurator.SendCommand("AT+VERSION?");
        sequence_ok &= configurator.SendAndVerify("AT+ROLE=1", "AT+ROLE?");
        sequence_ok &= configurator.SendAndVerify(std::string("AT+NAME=") + GP_MATRIX_BT_LOCAL_NAME,
                                                  "AT+NAME?");
        sequence_ok &= configurator.SendAndVerify(std::string("AT+PSWD=") + GP_MATRIX_BT_PIN_CODE,
                                                  "AT+PSWD?");
        sequence_ok &= configurator.SendAndVerify("AT+CMODE=0", "AT+CMODE?");

        ESP_LOGI(TAG, "[BT_CFG] remote=%s addr=%s", GP_MATRIX_BT_REMOTE_NAME, remote_address->c_str());
        sequence_ok &= configurator.BindRemoteAddress(*remote_address);
        sequence_ok &= configurator.SendAndVerify("AT+UART=" + std::to_string(static_cast<unsigned long>(GP_MATRIX_BT_UART_DATA_BAUDRATE)) + ",0,0",
                                                  "AT+UART?");
        sequence_ok &= configurator.SwitchRemoteAndLocalBaud(GP_MATRIX_BT_UART_DATA_BAUDRATE);
        ESP_LOGI(TAG,
                 "[BT_CFG] sequence=%s final_baud=%lu local=%s remote=%s addr=%s pin=%s",
                 sequence_ok ? "ok" : "partial",
                 static_cast<unsigned long>(configurator.GetCurrentBaudrate()),
                 GP_MATRIX_BT_LOCAL_NAME,
                 GP_MATRIX_BT_REMOTE_NAME,
                 remote_address->c_str(),
                 GP_MATRIX_BT_PIN_CODE);
        return configurator.GetCurrentBaudrate();
    }

    void InitializeI2c() {
        /* Audio codec, touch panel, camera SCCB, and the PCA9557 all share this local I2C bus. */
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)1,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 0,
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

        mcp_server.AddTool("self.screen.matrix_16x16.draw",
            "Draw one 16x16 matrix frame on the LED side. Use preset=python_demo, frame_rgb332_hex with 256 RGB332 bytes, or compact bitmap_rows_hex plus one primary_rgb888 color.",
            PropertyList({
                Property("preset", kPropertyTypeString, std::string("")),
                Property("frame_rgb332_hex", kPropertyTypeString, std::string("")),
                Property("bitmap_rows_hex", kPropertyTypeString, std::string("")),
                Property("primary_rgb888", kPropertyTypeString, std::string("")),
                Property("source", kPropertyTypeString, std::string("mcp")),
                Property("transcript", kPropertyTypeString, std::string(""))
            }),
            [this](const PropertyList& properties) -> ReturnValue {
                auto* matrix_led = led_matrix_.get();
                auto* debug_display = dynamic_cast<GpDebugLcdDisplay*>(display_);
                const std::string preset = ToAsciiLower(properties["preset"].value<std::string>());
                const std::string frame_hex = properties["frame_rgb332_hex"].value<std::string>();
                const std::string bitmap_rows_hex = properties["bitmap_rows_hex"].value<std::string>();
                const std::string primary_text = properties["primary_rgb888"].value<std::string>();
                const std::string source = properties["source"].value<std::string>();
                const std::string transcript = properties["transcript"].value<std::string>();
                bool applied = false;
                std::string resolved_frame_hex = frame_hex;
                std::string resolved_primary_text = primary_text;

                if (matrix_led == nullptr) {
                    throw std::runtime_error("LED matrix transport is not initialized");
                }

                if (!preset.empty()) {
                    if (preset == "python_demo") {
                        applied = matrix_led->ShowRgb332FramePreset(GpColorDebugPreset::kPythonDemo);
                    } else {
                        throw std::runtime_error("Unsupported 16x16 preset: " + preset);
                    }
                } else if (!frame_hex.empty()) {
                    const auto frame = ParseRgb332FrameHex(frame_hex);

                    if (!frame.has_value()) {
                        throw std::runtime_error("frame_rgb332_hex must contain exactly 256 RGB332 bytes encoded as 512 hex characters");
                    }
                    applied = matrix_led->ShowRgb332Frame(frame->data(), frame->size(), kGpMatrixModeSolidFrame);
                } else if (!bitmap_rows_hex.empty()) {
                    const auto bitmap_rows = ParseMatrixBitmapRowsHex(bitmap_rows_hex);
                    const auto primary_rgb = ParseRgb888(primary_text);
                    const auto frame = bitmap_rows.has_value() && primary_rgb.has_value()
                        ? BuildRgb332FrameFromBitmapRows(*bitmap_rows, *primary_rgb)
                        : std::array<uint8_t, GP_MATRIX_RGB332_FRAME_SIZE> {};

                    if (!bitmap_rows.has_value()) {
                        throw std::runtime_error("bitmap_rows_hex must contain exactly 16 rows encoded as 64 hex characters");
                    }
                    if (!primary_rgb.has_value()) {
                        throw std::runtime_error("primary_rgb888 must be a RGB888 string like #RRGGBB");
                    }

                    if (debug_display != nullptr) {
                        debug_display->ApplyMatrixBitmapPreview(*bitmap_rows, *primary_rgb, 0x000000U);
                    }
                    applied = matrix_led->ShowBitmapFrame(bitmap_rows->data(),
                                                          bitmap_rows->size(),
                                                          *primary_rgb,
                                                          0x000000U,
                                                          kGpMatrixModeSolidFrame);

                    char rgb_text[16] = {0};
                    std::snprintf(rgb_text, sizeof(rgb_text), "#%06X", static_cast<unsigned int>(*primary_rgb & 0xFFFFFFU));
                    resolved_primary_text = rgb_text;

                    {
                        std::string compact_hex;
                        compact_hex.reserve(frame.size() * 2U);
                        for (uint8_t value : frame) {
                            char byte_text[3] = {0};
                            std::snprintf(byte_text, sizeof(byte_text), "%02x", static_cast<unsigned int>(value));
                            compact_hex += byte_text;
                        }
                        resolved_frame_hex = compact_hex;
                    }
                } else {
                    throw std::runtime_error("Either preset, frame_rgb332_hex, or bitmap_rows_hex plus primary_rgb888 is required");
                }

                if (!applied) {
                    throw std::runtime_error("16x16 frame draw failed");
                }

                return BuildMatrixFrameResultJson(preset.c_str(),
                                                  resolved_frame_hex.c_str(),
                                                  bitmap_rows_hex.c_str(),
                                                  resolved_primary_text.c_str(),
                                                  applied,
                                                  source.c_str(),
                                                  transcript.c_str());
            });

        mcp_server.AddTool("self.screen.preview_image.fetch_http",
            "Fetch one PNG or JPEG from a host HTTP URL and show it in the debug preview area.",
            PropertyList({
                Property("url", kPropertyTypeString),
                Property("source", kPropertyTypeString, std::string("host_http")),
                Property("transcript", kPropertyTypeString, std::string(""))
            }),
            [this](const PropertyList& properties) -> ReturnValue {
                const std::string url = properties["url"].value<std::string>();
                const std::string source = properties["source"].value<std::string>();
                const std::string transcript = properties["transcript"].value<std::string>();
                std::string error_message;
                size_t image_bytes = 0;

                if (!FetchPreviewImageFromHttp(url, &error_message, &image_bytes)) {
                    throw std::runtime_error(error_message);
                }

                return BuildPreviewFetchResultJson(url.c_str(),
                                                   image_bytes,
                                                   true,
                                                   source.c_str(),
                                                   transcript.c_str());
            });

        mcp_server.AddTool("self.screen.debug_snapshot.set_upload_url",
            "Set or clear the HTTP upload URL used by the local Snap button. Use an empty string to clear it.",
            PropertyList({
                Property("url", kPropertyTypeString, std::string(""))
            }),
            [](const PropertyList& properties) -> ReturnValue {
                const std::string url = properties["url"].value<std::string>();
                SetDebugSnapshotUploadUrl(url);
                return BuildDebugSnapshotUploadUrlJson();
            });

        mcp_server.AddTool("self.screen.debug_snapshot.get_upload_url",
            "Return the HTTP upload URL used by the local Snap button.",
            PropertyList(),
            [](const PropertyList& properties) -> ReturnValue {
                return BuildDebugSnapshotUploadUrlJson();
            });

        mcp_server.AddTool("self.screen.debug_snapshot.capture",
            "Trigger the local Snap flow and upload the screenshot to the configured HTTP receiver.",
            PropertyList({
                Property("quality", kPropertyTypeInteger, 50, 0, 95)
            }),
            [this](const PropertyList& properties) -> ReturnValue {
                const int quality = properties["quality"].value<int>();
                const std::string status_text = QueueDebugSnapshotCapture(quality);
                return BuildDebugSnapshotCaptureJson(status_text, quality);
            });

    }

    void InitializeLedMatrix() {
        auto* debug_display = dynamic_cast<GpDebugLcdDisplay*>(display_);
        std::unique_ptr<GpMatrixTransport> transport;

        bt_transport_baudrate_ = ConfigureBluetoothModule();

        transport = CreateGpMatrixBtUartTransport(GP_MATRIX_BT_UART_PORT,
                                                  GP_MATRIX_BT_UART_TX_PIN,
                                                  GP_MATRIX_BT_UART_RX_PIN,
                                                  bt_transport_baudrate_,
                                                  GP_MATRIX_BT_LOCAL_NAME,
                                                  GP_MATRIX_BT_REMOTE_NAME,
                                                  GP_MATRIX_BT_PIN_CODE);

        led_matrix_ = std::make_unique<GpLedMatrixEsp32>(std::move(transport), GP_MATRIX_DEFAULT_BRIGHTNESS);
        if (debug_display == nullptr) {
            InitializeBluetoothBridgeTask();
            return;
        }

        debug_display->ApplyAiLinkStatus(false, "Bluetooth init\nXiaoZhi to WS2812");
        led_matrix_->SetLinkStatusCallback([debug_display](bool online, const std::string& status_text) {
            Application::GetInstance().Schedule([debug_display, online, status_text]() {
                debug_display->ApplyAiLinkStatus(online, status_text);
            });
        });
        debug_display->SetMatrixDebugStateCallback([this](const GpColorDebugState& state) {
            return QueueMatrixDebugState(state);
        });
        debug_display->SetDebugSnapshotCallback([this]() {
            return QueueDebugSnapshotCapture(50);
        });
        debug_display->SetTouchCommandCallback([this](const GpColorDebugState& state, bool request_pattern_draw) {
            const std::string status_text = QueueDebugWebsocketTouchCommand(state, request_pattern_draw);

            if (request_pattern_draw) {
                ShowSerialCommandNotification(status_text);
            }
        });
        InitializeBluetoothBridgeTask();
    }

public:
    LichuangDevBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        EnsureDefaultDebugSnapshotUploadUrl();
        InitializeI2c();
        InitializeSpi();
        InitializeSt7789Display();
        InitializeTouch();
        InitializeButtons();
        InitializeCamera();
        InitializeDebugCommandTask();
        InitializeTools();
        InitializeLedMatrix();
        InitializeSerialDebugCommands();
        UpdateDebugPreviewStatus(false, "HTTP preview waiting for Wi-Fi");
        LogDebugWebsocketStatus("boot");
        SetNetworkEventCallback([this](NetworkEvent event, const std::string& data) {
            (void)data;

            switch (event) {
            case NetworkEvent::Connected:
                Application::GetInstance().Schedule([this]() {
                    EnsureDebugPreviewHttpServer();
                });
                break;
            case NetworkEvent::Connecting:
                UpdateDebugPreviewStatus(false, "HTTP preview waiting for Wi-Fi");
                break;
            case NetworkEvent::Disconnected:
                UpdateDebugPreviewStatus(false, "HTTP preview Wi-Fi disconnected");
                break;
            case NetworkEvent::WifiConfigModeEnter:
                UpdateDebugPreviewStatus(false, "HTTP preview Wi-Fi config mode");
                break;
            default:
                break;
            }
        });
        LogDebugSnapshotUploadUrl("startup");

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
        return led_matrix_.get();
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
