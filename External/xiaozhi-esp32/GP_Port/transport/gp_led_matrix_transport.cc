#include "transport/gp_led_matrix_transport.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "gp_led_matrix_protocol.h"

#define TAG "GpMatrixTransport"

namespace {
constexpr int kBtUartRxBufferBytes = 1024;
constexpr int kBtUartTxBufferBytes = 1024;
constexpr size_t kBtReadChunkBytes = 64;
constexpr size_t kBtMaxPacketBytes = GP_MATRIX_PACKET_HEADER_SIZE + GP_MATRIX_MAX_CHUNK_DATA + 8U;

class GpMatrixBtUartTransport final : public GpMatrixTransport {
public:
    GpMatrixBtUartTransport(int uart_port,
                            int tx_gpio,
                            int rx_gpio,
                            uint32_t baudrate,
                            std::string local_name,
                            std::string remote_name,
                            std::string pin_code)
        : uart_port_(static_cast<uart_port_t>(uart_port)),
          tx_gpio_(tx_gpio),
          rx_gpio_(rx_gpio),
          baudrate_(baudrate),
          local_name_(std::move(local_name)),
          remote_name_(std::move(remote_name)),
          pin_code_(std::move(pin_code)) {
        uart_config_t uart_config = {};

        uart_config.baud_rate = static_cast<int>(baudrate_);
        uart_config.data_bits = UART_DATA_8_BITS;
        uart_config.parity = UART_PARITY_DISABLE;
        uart_config.stop_bits = UART_STOP_BITS_1;
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_config.rx_flow_ctrl_thresh = 0;
        uart_config.source_clk = UART_SCLK_DEFAULT;

        if (pin_code_.empty()) {
            pin_code_ = "19220309";
        }
        if (local_name_.empty()) {
            local_name_ = "XiaoZhi";
        }
        if (remote_name_.empty()) {
            remote_name_ = "WS2812";
        }

        ESP_ERROR_CHECK(uart_driver_install(uart_port_, kBtUartRxBufferBytes, kBtUartTxBufferBytes, 0, nullptr, 0));
        ESP_ERROR_CHECK(uart_param_config(uart_port_, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(uart_port_, tx_gpio_, rx_gpio_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        ESP_ERROR_CHECK(uart_flush_input(uart_port_));

        ESP_LOGI(TAG,
                 "HC-05 UART ready: port=%d tx=%d rx=%d baud=%lu local=%s remote=%s pin=%s",
                 static_cast<int>(uart_port_),
                 tx_gpio_,
                 rx_gpio_,
                 static_cast<unsigned long>(baudrate_),
                 local_name_.c_str(),
                 remote_name_.c_str(),
                 pin_code_.c_str());
    }

    ~GpMatrixBtUartTransport() override {
        uart_driver_delete(uart_port_);
    }

    bool WritePacket(const uint8_t* data, size_t length, uint32_t timeout_ms) override {
        std::lock_guard<std::mutex> lock(mutex_);
        int bytes_written;

        if ((data == nullptr) || (length == 0U)) {
            return false;
        }

        bytes_written = uart_write_bytes(uart_port_, data, length);
        if (bytes_written != static_cast<int>(length)) {
            ESP_LOGW(TAG,
                     "HC-05 UART short write: expected=%u actual=%d",
                     static_cast<unsigned int>(length),
                     bytes_written);
            return false;
        }

        return uart_wait_tx_done(uart_port_, pdMS_TO_TICKS(timeout_ms)) == ESP_OK;
    }

    bool ReadPacket(uint8_t* data, size_t length, uint32_t timeout_ms) override {
        const TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

        if ((data == nullptr) || (length == 0U)) {
            return false;
        }

        while (xTaskGetTickCount() <= deadline) {
            {
                std::lock_guard<std::mutex> lock(mutex_);

                PumpRxBytes(8U);
                if (TryExtractPacket(data, length)) {
                    return true;
                }
            }

            vTaskDelay(pdMS_TO_TICKS(8));
        }

        return false;
    }

    std::string DescribeLink() const override {
        return std::string("UART ") + local_name_ + "->" + remote_name_;
    }

private:
    uart_port_t uart_port_;
    int tx_gpio_;
    int rx_gpio_;
    uint32_t baudrate_;
    std::string local_name_;
    std::string remote_name_;
    std::string pin_code_;
    std::vector<uint8_t> rx_buffer_;
    std::mutex mutex_;

    void PumpRxBytes(uint32_t timeout_ms) {
        uint8_t rx_chunk[kBtReadChunkBytes];
        int bytes_read;

        bytes_read = uart_read_bytes(uart_port_, rx_chunk, sizeof(rx_chunk), pdMS_TO_TICKS(timeout_ms));
        if (bytes_read > 0) {
            rx_buffer_.insert(rx_buffer_.end(), rx_chunk, rx_chunk + bytes_read);
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
}

std::unique_ptr<GpMatrixTransport> CreateGpMatrixBtUartTransport(int uart_port,
                                                                 int tx_gpio,
                                                                 int rx_gpio,
                                                                 uint32_t baudrate,
                                                                 const std::string& local_name,
                                                                 const std::string& remote_name,
                                                                 const std::string& pin_code) {
    return std::make_unique<GpMatrixBtUartTransport>(uart_port,
                                                     tx_gpio,
                                                     rx_gpio,
                                                     baudrate,
                                                     local_name,
                                                     remote_name,
                                                     pin_code);
}