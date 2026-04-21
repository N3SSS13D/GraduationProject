#ifndef GP_LED_MATRIX_ESP32_H_
#define GP_LED_MATRIX_ESP32_H_

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "device_state.h"
#include "ui/gp_debug_display.h"
#include "led/led.h"
#include "gp_led_matrix_protocol.h"

class GpMatrixTransport;

class GpLedMatrixEsp32 : public Led {
public:
    using LinkStatusCallback = std::function<void(bool online, const std::string& status_text)>;

    /* Create the ESP32-side matrix driver around the active transport backend. */
    GpLedMatrixEsp32(std::unique_ptr<GpMatrixTransport> transport, uint8_t brightness = 0x40);

    /* Keep the standard LED interface satisfied without auto-overwriting explicit matrix content. */
    void OnStateChanged() override;

    /* Send a short startup self-check sequence through the active matrix transport. */
    void RunStartupLinkTest();

    /* Update the default matrix brightness used for later action generation. */
    void SetBrightness(uint8_t brightness);

    /* Register the UI callback used to show current matrix link state. */
    void SetLinkStatusCallback(LinkStatusCallback callback);

    /* Convert one GP debug-menu state object into a matrix action and send it. */
    bool ShowDebugState(const GpColorDebugState& state);

    /* Send one already-built matrix action payload to the AI8051U side. */
    bool ShowAction(const GpMatrixActionPayload& action);

    /* Send one full RGB332 frame through the current matrix transport. */
    bool ShowRgb332Frame(const uint8_t* frame, size_t length, GpMatrixMode mode = kGpMatrixModeSolidFrame);

    /* Send one packed glyph-row buffer for subtitle-style rendering. */
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
    std::unique_ptr<GpMatrixTransport> transport_;
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