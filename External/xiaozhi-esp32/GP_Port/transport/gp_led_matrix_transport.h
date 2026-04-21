/*
 * @file gp_led_matrix_transport.h
 * @brief Matrix transport abstraction for the BT_Version local link.
 */

#ifndef GP_LED_MATRIX_TRANSPORT_H_
#define GP_LED_MATRIX_TRANSPORT_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

class GpMatrixTransport {
public:
    virtual ~GpMatrixTransport() = default;

    /* Send one complete protocol packet through the active transport backend. */
    virtual bool WritePacket(const uint8_t* data, size_t length, uint32_t timeout_ms) = 0;

    /* Read one complete protocol packet through the active transport backend. */
    virtual bool ReadPacket(uint8_t* data, size_t length, uint32_t timeout_ms) = 0;

    /* Return a short human-readable description of the active link. */
    virtual std::string DescribeLink() const = 0;
};

/* Create the BT SPP transport used by BT_Version. Unsupported targets return nullptr. */
std::unique_ptr<GpMatrixTransport> CreateGpMatrixBtSppTransport(const std::string& local_name,
                                                                const std::string& remote_name,
                                                                const std::string& remote_address,
                                                                const std::string& pin_code,
                                                                uint32_t connect_timeout_ms);

#endif