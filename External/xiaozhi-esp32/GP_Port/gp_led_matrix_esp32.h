#ifndef GP_LED_MATRIX_ESP32_H_
#define GP_LED_MATRIX_ESP32_H_

#include <array>
#include <cstdint>
#include <mutex>

#include <driver/i2c_master.h>

#include "device_state.h"
#include "i2c_device.h"
#include "led/led.h"
#include "gp_led_matrix_protocol.h"

class GpLedMatrixEsp32 : public Led, protected I2cDevice {
public:
    GpLedMatrixEsp32(i2c_master_bus_handle_t i2c_bus, uint8_t address, uint8_t brightness = 0x40);

    void OnStateChanged() override;
    void SetBrightness(uint8_t brightness);
    bool ShowRgb332Frame(const uint8_t* frame, size_t length, GpMatrixMode mode = kGpMatrixModeSolidFrame);
    bool ShowGlyphRows(const uint16_t* rows, size_t row_count, uint8_t glyph_count, uint8_t glyph_width, uint8_t glyph_spacing);

private:
    using Rgb332Frame = std::array<uint8_t, GP_MATRIX_RGB332_FRAME_SIZE>;

    uint8_t brightness_;
    uint8_t sequence_;
    std::mutex mutex_;

    bool SendCommand(uint8_t command, const uint8_t* payload, size_t payload_length, bool ack_required = false);
    bool SendState(DeviceState state, const Rgb332Frame& frame, GpMatrixMode mode = kGpMatrixModeSolidFrame);
    Rgb332Frame BuildFrameForState(DeviceState state) const;

    static void FillFrame(Rgb332Frame& frame, uint8_t color);
    static void DrawCross(Rgb332Frame& frame, uint8_t color, uint8_t background);
    static void DrawDiamond(Rgb332Frame& frame, uint8_t color, uint8_t background);
    static void DrawBorder(Rgb332Frame& frame, uint8_t color, uint8_t background);
    static void DrawBars(Rgb332Frame& frame, uint8_t first, uint8_t second, uint8_t background);
};

#endif