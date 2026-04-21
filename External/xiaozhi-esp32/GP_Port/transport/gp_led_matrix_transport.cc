#include "transport/gp_led_matrix_transport.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <soc/soc_caps.h>

#if defined(SOC_BT_CLASSIC_SUPPORTED) && SOC_BT_CLASSIC_SUPPORTED
#include <esp_bt.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_spp_api.h>
#endif

#include "gp_led_matrix_protocol.h"

#define TAG "GpMatrixTransport"

namespace {
constexpr EventBits_t kBtStackReadyBit = BIT0;
constexpr EventBits_t kBtConnectedBit = BIT1;
constexpr uint32_t kBtSppDefaultScn = 1;
constexpr uint32_t kBtDiscoveryDuration = 10;
constexpr size_t kBtMaxPacketBytes = GP_MATRIX_PACKET_HEADER_SIZE + GP_MATRIX_MAX_CHUNK_DATA + 8U;

#if defined(SOC_BT_CLASSIC_SUPPORTED) && SOC_BT_CLASSIC_SUPPORTED
class GpMatrixBtSppTransport final : public GpMatrixTransport {
public:
    GpMatrixBtSppTransport(std::string local_name,
                           std::string remote_name,
                           std::string remote_address,
                           std::string pin_code,
                           uint32_t connect_timeout_ms)
        : local_name_(std::move(local_name)),
          remote_name_(std::move(remote_name)),
          remote_address_(std::move(remote_address)),
          pin_code_(std::move(pin_code)),
          connect_timeout_ms_(connect_timeout_ms),
          event_group_(xEventGroupCreate()) {
        if (pin_code_.empty()) {
            pin_code_ = "1234";
        }
        if (local_name_.empty()) {
            local_name_ = "XiaoZhi-Matrix";
        }
        if (remote_name_.empty()) {
            remote_name_ = "HC-05";
        }

        instance_ = this;
        InitializeBtStack();
    }

    ~GpMatrixBtSppTransport() override {
        instance_ = nullptr;
        if (event_group_ != nullptr) {
            vEventGroupDelete(event_group_);
            event_group_ = nullptr;
        }
    }

    bool WritePacket(const uint8_t* data, size_t length, uint32_t timeout_ms) override {
        std::lock_guard<std::mutex> lock(mutex_);

        if ((data == nullptr) || (length == 0U)) {
            return false;
        }
        if (!EnsureConnected(timeout_ms)) {
            return false;
        }
        return esp_spp_write(connection_handle_, length, const_cast<uint8_t*>(data)) == ESP_OK;
    }

    bool ReadPacket(uint8_t* data, size_t length, uint32_t timeout_ms) override {
        TickType_t deadline;

        if ((data == nullptr) || (length == 0U)) {
            return false;
        }
        if (!EnsureConnected(timeout_ms)) {
            return false;
        }

        deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);
        while (xTaskGetTickCount() <= deadline) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (TryExtractPacket(data, length)) {
                    return true;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(8));
        }

        return false;
    }

    std::string DescribeLink() const override {
        if (!remote_address_.empty()) {
            return std::string("SPP ") + remote_address_;
        }
        return std::string("SPP ") + remote_name_;
    }

private:
    static GpMatrixBtSppTransport* instance_;
    static bool bt_stack_initialized_;

    std::string local_name_;
    std::string remote_name_;
    std::string remote_address_;
    std::string pin_code_;
    uint32_t connect_timeout_ms_;
    EventGroupHandle_t event_group_ = nullptr;
    std::vector<uint8_t> rx_buffer_;
    std::mutex mutex_;
    esp_bd_addr_t remote_bda_ = {0};
    uint32_t connection_handle_ = 0;
    bool connected_ = false;
    bool discovery_started_ = false;
    bool remote_found_ = false;

    static bool ParseRemoteAddress(const std::string& text, esp_bd_addr_t address) {
        unsigned int values[6] = {0};

        if (std::sscanf(text.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
                        &values[0],
                        &values[1],
                        &values[2],
                        &values[3],
                        &values[4],
                        &values[5]) != 6) {
            return false;
        }

        for (size_t index = 0; index < 6; ++index) {
            address[index] = static_cast<uint8_t>(values[index]);
        }
        return true;
    }

    static void BtGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param) {
        if (instance_ != nullptr) {
            instance_->HandleGapEvent(event, param);
        }
    }

    static void BtSppCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
        if (instance_ != nullptr) {
            instance_->HandleSppEvent(event, param);
        }
    }

    void InitializeBtStack() {
        esp_bt_pin_code_t pin_code = {0};
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

        if (bt_stack_initialized_) {
            xEventGroupSetBits(event_group_, kBtStackReadyBit);
            return;
        }

        ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
        ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));
        ESP_ERROR_CHECK(esp_bluedroid_init());
        ESP_ERROR_CHECK(esp_bluedroid_enable());
        ESP_ERROR_CHECK(esp_bt_gap_register_callback(BtGapCallback));
        ESP_ERROR_CHECK(esp_spp_register_callback(BtSppCallback));
        ESP_ERROR_CHECK(esp_bt_dev_set_device_name(local_name_.c_str()));

        std::memcpy(pin_code, pin_code_.data(), std::min(pin_code_.size(), sizeof(pin_code)));
        ESP_ERROR_CHECK(esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED,
                                           std::min(pin_code_.size(), sizeof(pin_code)),
                                           pin_code));
        ESP_ERROR_CHECK(esp_spp_init(ESP_SPP_MODE_CB));

        bt_stack_initialized_ = true;
    }

    bool EnsureConnected(uint32_t timeout_ms) {
        EventBits_t bits;

        if (event_group_ == nullptr) {
            return false;
        }

        bits = xEventGroupWaitBits(event_group_,
                                   kBtStackReadyBit,
                                   pdFALSE,
                                   pdTRUE,
                                   pdMS_TO_TICKS(timeout_ms));
        if ((bits & kBtStackReadyBit) == 0U) {
            return false;
        }
        if (connected_) {
            return true;
        }

        StartConnectionAttempt();
        bits = xEventGroupWaitBits(event_group_,
                                   kBtConnectedBit,
                                   pdFALSE,
                                   pdTRUE,
                                   pdMS_TO_TICKS(connect_timeout_ms_));
        return (bits & kBtConnectedBit) != 0U;
    }

    void StartConnectionAttempt() {
        if (discovery_started_) {
            return;
        }

        xEventGroupClearBits(event_group_, kBtConnectedBit);
        rx_buffer_.clear();
        discovery_started_ = true;
        remote_found_ = false;

        if (ParseRemoteAddress(remote_address_, remote_bda_)) {
            ESP_LOGI(TAG, "SPP direct connect to %s", remote_address_.c_str());
            if (esp_spp_connect(ESP_SPP_SEC_NONE,
                                ESP_SPP_ROLE_MASTER,
                                kBtSppDefaultScn,
                                remote_bda_) != ESP_OK) {
                discovery_started_ = false;
            }
            return;
        }

        ESP_LOGI(TAG, "SPP discover remote name=%s", remote_name_.c_str());
        if (esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY,
                                       kBtDiscoveryDuration,
                                       0) != ESP_OK) {
            discovery_started_ = false;
        }
    }

    void HandleGapEvent(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param) {
        if (param == nullptr) {
            return;
        }

        switch (event) {
        case ESP_BT_GAP_DISC_RES_EVT:
            HandleDiscoveryResult(param->disc_res);
            break;
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            if ((param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED)
                && remote_found_) {
                (void)esp_spp_start_discovery(remote_bda_);
            } else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
                discovery_started_ = false;
            }
            break;
        default:
            break;
        }
    }

    void HandleDiscoveryResult(const esp_bt_gap_cb_param_t::disc_res_param& disc_res) {
        size_t name_length;
        std::string remote_name;

        for (int index = 0; index < disc_res.num_prop; ++index) {
            const auto& property = disc_res.prop[index];

            if ((property.type == ESP_BT_GAP_DEV_PROP_BDNAME) && (property.val != nullptr)) {
                name_length = static_cast<size_t>(property.len);
                remote_name.assign(static_cast<const char*>(property.val),
                                   static_cast<const char*>(property.val) + name_length);
                break;
            }
        }

        if (remote_name != remote_name_) {
            return;
        }

        std::memcpy(remote_bda_, disc_res.bda, sizeof(remote_bda_));
        remote_found_ = true;
        ESP_LOGI(TAG, "SPP discovered %s", remote_name_.c_str());
        (void)esp_bt_gap_cancel_discovery();
    }

    void HandleSppEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
        if (param == nullptr) {
            return;
        }

        switch (event) {
        case ESP_SPP_INIT_EVT:
            xEventGroupSetBits(event_group_, kBtStackReadyBit);
            break;
        case ESP_SPP_DISCOVERY_COMP_EVT:
            if ((param->disc_comp.status == ESP_SPP_SUCCESS) && (param->disc_comp.scn_num > 0U)) {
                (void)esp_spp_connect(ESP_SPP_SEC_NONE,
                                      ESP_SPP_ROLE_MASTER,
                                      param->disc_comp.scn[0],
                                      remote_bda_);
            } else {
                discovery_started_ = false;
            }
            break;
        case ESP_SPP_OPEN_EVT:
            connection_handle_ = param->open.handle;
            connected_ = true;
            discovery_started_ = false;
            xEventGroupSetBits(event_group_, kBtConnectedBit);
            ESP_LOGI(TAG, "SPP connected handle=%lu", static_cast<unsigned long>(connection_handle_));
            break;
        case ESP_SPP_CLOSE_EVT:
            connection_handle_ = 0;
            connected_ = false;
            discovery_started_ = false;
            xEventGroupClearBits(event_group_, kBtConnectedBit);
            ESP_LOGW(TAG, "SPP disconnected");
            break;
        case ESP_SPP_DATA_IND_EVT:
            {
                std::lock_guard<std::mutex> lock(mutex_);

                rx_buffer_.insert(rx_buffer_.end(),
                                  param->data_ind.data,
                                  param->data_ind.data + param->data_ind.len);
            }
            break;
        default:
            break;
        }
    }

    bool TryExtractPacket(uint8_t* data, size_t length) {
        uint16_t payload_length;
        size_t packet_length;

        while (!rx_buffer_.empty()) {
            if ((rx_buffer_.size() >= 2U)
                && (rx_buffer_[0] == GP_MATRIX_PROTOCOL_MAGIC0)
                && (rx_buffer_[1] == GP_MATRIX_PROTOCOL_MAGIC1)) {
                break;
            }
            rx_buffer_.erase(rx_buffer_.begin());
        }

        if (rx_buffer_.size() < GP_MATRIX_PACKET_HEADER_SIZE) {
            return false;
        }
        if (rx_buffer_[2] != GP_MATRIX_PROTOCOL_VERSION) {
            rx_buffer_.erase(rx_buffer_.begin());
            return false;
        }

        payload_length = static_cast<uint16_t>(rx_buffer_[6])
            | (static_cast<uint16_t>(rx_buffer_[7]) << 8);
        packet_length = GP_MATRIX_PACKET_HEADER_SIZE + payload_length + 1U;
        if ((packet_length > kBtMaxPacketBytes) || (packet_length > length)) {
            rx_buffer_.erase(rx_buffer_.begin());
            return false;
        }
        if (rx_buffer_.size() < packet_length) {
            return false;
        }

        std::memcpy(data, rx_buffer_.data(), packet_length);
        rx_buffer_.erase(rx_buffer_.begin(), rx_buffer_.begin() + packet_length);
        return true;
    }
};

GpMatrixBtSppTransport* GpMatrixBtSppTransport::instance_ = nullptr;
bool GpMatrixBtSppTransport::bt_stack_initialized_ = false;
#endif
}

std::unique_ptr<GpMatrixTransport> CreateGpMatrixBtSppTransport(const std::string& local_name,
                                                                const std::string& remote_name,
                                                                const std::string& remote_address,
                                                                const std::string& pin_code,
                                                                uint32_t connect_timeout_ms) {
#if defined(SOC_BT_CLASSIC_SUPPORTED) && SOC_BT_CLASSIC_SUPPORTED
    return std::make_unique<GpMatrixBtSppTransport>(local_name,
                                                    remote_name,
                                                    remote_address,
                                                    pin_code,
                                                    connect_timeout_ms);
#else
    (void)local_name;
    (void)remote_name;
    (void)remote_address;
    (void)pin_code;
    (void)connect_timeout_ms;
    ESP_LOGE(TAG, "Classic Bluetooth SPP is not supported by the current target");
    return nullptr;
#endif
}