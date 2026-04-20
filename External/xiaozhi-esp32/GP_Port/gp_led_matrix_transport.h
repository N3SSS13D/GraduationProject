/*
 * @file gp_led_matrix_transport.h
 * @author GitHub Copilot
 * @date 2026-04-18
 * @version 1.0
 * @brief Matrix transport abstraction for I2C and classic Bluetooth SPP backends.
 */

#ifndef GP_LED_MATRIX_TRANSPORT_H_
#define GP_LED_MATRIX_TRANSPORT_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include <driver/i2c_master.h>

class GpMatrixTransport {
public:
    virtual ~GpMatrixTransport() = default;

    virtual bool WritePacket(const uint8_t* data, size_t length, uint32_t timeout_ms) = 0;
    virtual bool ReadPacket(uint8_t* data, size_t length, uint32_t timeout_ms) = 0;
    virtual std::string DescribeLink() const = 0;
};

std::unique_ptr<GpMatrixTransport> CreateGpMatrixI2cTransport(i2c_master_bus_handle_t i2c_bus,
                                                              uint8_t address,
                                                              uint32_t scl_speed_hz = 100000);

std::unique_ptr<GpMatrixTransport> CreateGpMatrixBtSppTransport(const std::string& local_name,
                                                                const std::string& remote_name,
                                                                const std::string& remote_address,
                                                                const std::string& pin_code,
                                                                uint32_t connect_timeout_ms);

#endif