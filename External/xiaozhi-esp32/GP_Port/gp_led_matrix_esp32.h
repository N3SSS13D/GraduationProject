#ifndef GP_LED_MATRIX_ESP32_H_
#define GP_LED_MATRIX_ESP32_H_

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>

#include <driver/i2c_master.h>

#include "device_state.h"
#include "gp_debug_display.h"
#include "i2c_device.h"
#include "led/led.h"
#include "gp_led_matrix_protocol.h"

class GpLedMatrixEsp32 : public Led, protected I2cDevice {
public:
    using LinkStatusCallback = std::function<void(bool online, const std::string& status_text)>;

    GpLedMatrixEsp32(i2c_master_bus_handle_t i2c_bus, uint8_t address, uint8_t brightness = 0x40);

    void OnStateChanged() override;
    void RunStartupLinkTest();
    void SetBrightness(uint8_t brightness);
    void SetLinkStatusCallback(LinkStatusCallback callback);
    bool ShowDebugState(const GpColorDebugState& state);
    bool ShowAction(const GpMatrixActionPayload& action);
    bool ShowRgb332Frame(const uint8_t* frame, size_t length, GpMatrixMode mode = kGpMatrixModeSolidFrame);
    bool ShowGlyphRows(const uint16_t* rows, size_t row_count, uint8_t glyph_count, uint8_t glyph_width, uint8_t glyph_spacing);

private:
    using Rgb332Frame = std::array<uint8_t, GP_MATRIX_RGB332_FRAME_SIZE>;

    uint8_t brightness_;
    uint8_t sequence_;
    uint32_t success_count_;
    uint32_t failure_count_;
    uint32_t verified_count_;
    uint32_t no_reply_count_;
    bool remote_override_active_;
    bool has_last_action_;
    bool has_last_state_;
    GpMatrixActionPayload last_action_;
    DeviceState last_state_;
    std::string last_payload_summary_;
    std::mutex mutex_;
    LinkStatusCallback link_status_callback_;

    static bool ActionEquals(const GpMatrixActionPayload& left, const GpMatrixActionPayload& right);
    static const char* EffectShortName(uint8_t effect);
    static const char* StateShortName(DeviceState state);
    std::string BuildPayloadSummary(uint8_t command, const uint8_t* payload, size_t payload_length) const;
    bool ReadReply(uint8_t expected_sequence, uint8_t expected_command, GpMatrixStatusCode& reply_status);
    bool SendCommand(uint8_t command, const uint8_t* payload, size_t payload_length, bool ack_required = false);
    bool SendState(DeviceState state, const Rgb332Frame& frame, GpMatrixMode mode = kGpMatrixModeSolidFrame);
    std::string BuildStatusText(bool online,
                                uint8_t command,
                                uint8_t sequence,
                                size_t payload_length,
                                esp_err_t err,
                                GpMatrixStatusCode reply_status,
                                bool reply_valid) const;
    void NotifyLinkStatus(bool online, const std::string& status_text);
    Rgb332Frame BuildFrameForState(DeviceState state) const;
    static const char* CommandShortName(uint8_t command);

    static void FillFrame(Rgb332Frame& frame, uint8_t color);
    static void DrawCross(Rgb332Frame& frame, uint8_t color, uint8_t background);
    static void DrawDiamond(Rgb332Frame& frame, uint8_t color, uint8_t background);
    static void DrawBorder(Rgb332Frame& frame, uint8_t color, uint8_t background);
    static void DrawBars(Rgb332Frame& frame, uint8_t first, uint8_t second, uint8_t background);
};

#endif