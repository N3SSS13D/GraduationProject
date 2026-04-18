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
#include "settings.h"
#include "system_info.h"

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_lvgl_port.h>
#include <lvgl.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>

#define TAG "LichuangDevBoard"

namespace {
constexpr const char* kSnapshotPrefix = "xiaozhi_screen";
constexpr const char* kSnapshotSettingsNamespace = "debug_snapshot";
constexpr const char* kSnapshotUploadUrlKey = "upload_url";
constexpr const char* kSerialSnapCommand = "snap";
constexpr const char* kSerialSnapUrlCommand = "snap_url";
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
    static constexpr uint32_t kDebugCommandTaskStackWords = 8192;
    static constexpr UBaseType_t kDebugCommandTaskPriority = 2;
    static constexpr size_t kDebugCommandQueueLength = 6;
    static constexpr uint32_t kSerialCommandTaskStackWords = 4096;
    static constexpr UBaseType_t kSerialCommandTaskPriority = 1;
    static constexpr size_t kSerialCommandBufferSize = 256;

    struct DebugCommand {
        enum class Type : uint8_t {
            kApplyMatrixState = 0,
            kCaptureSnapshot = 1,
        };

        Type type = Type::kApplyMatrixState;
        GpColorDebugState state;
        int quality = 50;
    };

    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_dev_handle_t pca9557_handle_;
    Button boot_button_;
    Display* display_;
    Pca9557* pca9557_;
    Esp32Camera* camera_;
    std::unique_ptr<GpLedMatrixEsp32> led_matrix_;
    QueueHandle_t debug_command_queue_ = nullptr;
    uint32_t last_debug_snapshot_sequence_ = 0;
    std::mutex debug_snapshot_mutex_;
    std::mutex debug_snapshot_upload_mutex_;
    bool debug_snapshot_upload_in_progress_ = false;

    static std::string GetCompiledDebugSnapshotUploadUrl() {
        return TrimAsciiWhitespace(GP_DEBUG_SNAPSHOT_DEFAULT_UPLOAD_URL);
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

    static void LogDebugSnapshotUploadUrl(const char* reason) {
        const std::string current_url = GetDebugSnapshotUploadUrl();
        const std::string default_url = GetCompiledDebugSnapshotUploadUrl();

        ESP_LOGI(TAG, "[%s] snapshot upload url=%s", reason, current_url.empty() ? "<empty>" : current_url.c_str());
        if (!default_url.empty()) {
            ESP_LOGI(TAG, "[%s] compiled default snapshot upload url=%s", reason, default_url.c_str());
        }
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

        ESP_LOGI(TAG, "Serial debug command ready: snap | snap <quality> | snap_url get | set <url> | clear | reset | help");
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

    void InitializeI2c() {
        // The matrix link uses external 3.3V pullups, so keep the ESP32-side bus pullup configuration explicit.
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

        led_matrix_ = std::make_unique<GpLedMatrixEsp32>(i2c_bus_, GP_MATRIX_I2C_ADDRESS, GP_MATRIX_DEFAULT_BRIGHTNESS);
        if (debug_display == nullptr) {
            return;
        }

        debug_display->ApplyAiLinkStatus(false, "AI8051U INIT\nwaiting image update");
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
