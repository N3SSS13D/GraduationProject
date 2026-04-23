#include "gp_led_matrix_esp32.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "transport/gp_led_matrix_transport.h"

#define TAG "GpLedMatrix"

namespace {
constexpr uint8_t kColorOff = 0x00;
constexpr uint8_t kColorRed = 0xE0;
constexpr uint8_t kColorOrange = 0xE4;
constexpr uint8_t kColorYellow = 0xFC;
constexpr uint8_t kColorGreen = 0x1C;
constexpr uint8_t kColorBlue = 0x03;
constexpr uint8_t kColorCyan = 0x1F;
constexpr uint8_t kColorPurple = 0xC3;
constexpr uint8_t kColorWhite = 0xFF;
constexpr uint32_t kStartupLinkTestIntervalMs = 1000;
constexpr uint32_t kReplyPollIntervalMs = 8;
constexpr uint32_t kReplyPollRetries = 12;
constexpr size_t kStatusReplyPayloadBytes = 4;
constexpr uint8_t kMatrixDebugPatternDiamond = 0;
constexpr uint8_t kMatrixDebugPatternCross = 1;
constexpr uint8_t kMatrixDebugPatternJluEmblem = 2;
constexpr uint8_t kPythonDemoFrame[GP_MATRIX_RGB332_FRAME_SIZE] = {
    0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint8_t ExtractRgb888Channel(uint32_t rgb, uint8_t shift) {
    return static_cast<uint8_t>((rgb >> shift) & 0xFFU);
}

void WriteRgb888(uint8_t* dest, uint32_t rgb) {
    dest[0] = ExtractRgb888Channel(rgb, 16);
    dest[1] = ExtractRgb888Channel(rgb, 8);
    dest[2] = ExtractRgb888Channel(rgb, 0);
}

uint8_t BuildMatrixAnimStep(const GpColorDebugState& state) {
    if (state.animation_period_ms <= 900U) {
        return 2U;
    }
    return 1U;
}

uint8_t BuildMatrixGradientSpan(const GpColorDebugState& state) {
    const uint16_t scaled_span = static_cast<uint16_t>(state.dot_size_px) * 2U;
    return static_cast<uint8_t>(std::clamp<uint16_t>(scaled_span, 32U, 120U));
}

uint32_t ResolveMatrixBackgroundRgb888(const GpColorDebugState& state) {
    if (state.has_matrix_background_rgb888) {
        return state.matrix_background_rgb888;
    }
    return 0x000000U;
}

GpMatrixActionPayload BuildSolidAction(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue) {
    GpMatrixActionPayload action = {};

    action.source = kGpMatrixActionSourceLocal;
    action.content = kGpMatrixActionContentSolid;
    action.effect = kGpMatrixEffectStatic;
    action.direction = kGpMatrixDirectionNormal;
    action.color_mode = kGpMatrixColorModeSolid;
    action.brightness = brightness;
    action.primary_r = red;
    action.primary_g = green;
    action.primary_b = blue;
    action.scroll_step = 1;
    action.anim_step = 1;

    return action;
}

GpMatrixActionPayload BuildReleaseAction() {
    GpMatrixActionPayload action = {};

    action.source = kGpMatrixActionSourceLocal;
    action.flags = GP_MATRIX_ACTION_FLAG_REMOTE_RELEASE;

    return action;
}
}

GpLedMatrixEsp32::GpLedMatrixEsp32(std::unique_ptr<GpMatrixTransport> transport, uint8_t brightness)
        : brightness_(brightness),
      sequence_(0),
      success_count_(0),
      failure_count_(0),
      verified_count_(0),
      no_reply_count_(0),
            last_foreground_activity_tick_(0U),
    link_verified_(false),
      remote_override_active_(false),
      has_last_action_(false),
      has_last_state_(false),
            transport_(std::move(transport)),
      last_state_(kDeviceStateUnknown) {
}

void GpLedMatrixEsp32::OnStateChanged() {
    /* Keep the last explicit matrix image until a new image update is requested. */
}

void GpLedMatrixEsp32::RunStartupLinkTest() {
    const GpMatrixActionPayload actions[] = {
        BuildSolidAction(brightness_, 0xFF, 0x00, 0x00),
        BuildSolidAction(brightness_, 0x00, 0xFF, 0x00),
        BuildSolidAction(brightness_, 0x00, 0x00, 0xFF),
    };
    const TickType_t delay_ticks = pdMS_TO_TICKS(kStartupLinkTestIntervalMs);

    NotifyLinkStatus(false, "Bluetooth test\nmatrix self-check");
    for (const auto& action : actions) {
        if (!ShowAction(action)) {
            ESP_LOGW(TAG, "Startup link test action failed");
            NotifyLinkStatus(false, "Bluetooth waiting\nstartup self-check");
            break;
        }
        vTaskDelay(delay_ticks);
    }

    if (!ShowAction(BuildReleaseAction())) {
        ESP_LOGW(TAG, "Startup link test release failed");
        NotifyLinkStatus(false, "Bluetooth waiting\nrelease self-check");
        return;
    }

    NotifyLinkStatus(true, "Bluetooth connected\nstartup self-check");
}

void GpLedMatrixEsp32::SetBrightness(uint8_t brightness) {
    std::lock_guard<std::mutex> lock(mutex_);
    brightness_ = brightness;
}

void GpLedMatrixEsp32::SetLinkStatusCallback(LinkStatusCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    link_status_callback_ = std::move(callback);
}

bool GpLedMatrixEsp32::ShowDebugState(const GpColorDebugState& state) {
    GpMatrixActionPayload action = {};
    const uint32_t background_rgb888 = ResolveMatrixBackgroundRgb888(state);

    if (state.preset == GpColorDebugPreset::kPythonDemo) {
        return ShowRgb332FramePreset(state.preset);
    }

    action.source = kGpMatrixActionSourceLocal;
    action.direction = kGpMatrixDirectionNormal;
    action.brightness = brightness_;
    action.primary_r = ExtractRgb888Channel(state.primary_rgb888, 16);
    action.primary_g = ExtractRgb888Channel(state.primary_rgb888, 8);
    action.primary_b = ExtractRgb888Channel(state.primary_rgb888, 0);
    action.secondary_r = ExtractRgb888Channel(background_rgb888, 16);
    action.secondary_g = ExtractRgb888Channel(background_rgb888, 8);
    action.secondary_b = ExtractRgb888Channel(background_rgb888, 0);
    action.scroll_step = 1U;
    action.anim_step = BuildMatrixAnimStep(state);
    action.gradient_span = BuildMatrixGradientSpan(state);

    if (state.has_secondary) {
        action.secondary_r = ExtractRgb888Channel(state.secondary_rgb888, 16);
        action.secondary_g = ExtractRgb888Channel(state.secondary_rgb888, 8);
        action.secondary_b = ExtractRgb888Channel(state.secondary_rgb888, 0);
        action.flags |= GP_MATRIX_ACTION_FLAG_USE_SECONDARY;
    }

    if (state.preset == GpColorDebugPreset::kScrollSubtitle) {
        action.content = kGpMatrixActionContentGlyph;
        action.glyph_id = 0U;
        action.effect = kGpMatrixEffectTextScroll;
        action.color_mode = kGpMatrixColorModeSolid;
        return ShowAction(action);
    }

    if (state.preset == GpColorDebugPreset::kSolid) {
        action.content = kGpMatrixActionContentSolid;
    } else {
        action.content = kGpMatrixActionContentPattern;
        if (state.preset == GpColorDebugPreset::kCross) {
            action.pattern_id = kMatrixDebugPatternCross;
        } else if (state.preset == GpColorDebugPreset::kJluEmblem) {
            action.pattern_id = kMatrixDebugPatternJluEmblem;
        } else {
            action.pattern_id = kMatrixDebugPatternDiamond;
        }
    }

    if ((state.animation == GpColorDebugAnimation::kGradient) && state.has_secondary) {
        action.effect = kGpMatrixEffectGradient;
        action.color_mode = kGpMatrixColorModeGradient;
        action.flags |= GP_MATRIX_ACTION_FLAG_USE_SECONDARY;
    } else if (state.animation == GpColorDebugAnimation::kPulse) {
        action.effect = kGpMatrixEffectBreath;
        action.color_mode = kGpMatrixColorModeSolid;
    } else {
        action.effect = kGpMatrixEffectStatic;
        action.color_mode = kGpMatrixColorModeSolid;
    }

    return ShowAction(action);
}

bool GpLedMatrixEsp32::ShowAction(const GpMatrixActionPayload& action) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (has_last_action_ && ActionEquals(last_action_, action)) {
        return true;
    }

    uint8_t payload[GP_MATRIX_ACTION_PAYLOAD_BYTES] = {
        action.source,
        action.content,
        action.effect,
        action.direction,
        action.color_mode,
        action.brightness,
        action.primary_r,
        action.primary_g,
        action.primary_b,
        action.secondary_r,
        action.secondary_g,
        action.secondary_b,
        action.pattern_id,
        action.glyph_id,
        action.scroll_step,
        action.anim_step,
        action.gradient_span,
        action.flags,
    };
    const bool sent = SendCommand(kGpMatrixCommandSetAction, payload, sizeof(payload), true);

    if (sent) {
        has_last_action_ = true;
        last_action_ = action;
        remote_override_active_ = ((action.flags & GP_MATRIX_ACTION_FLAG_REMOTE_RELEASE) == 0U);
        if (!remote_override_active_) {
            has_last_state_ = false;
        }
    }

    return sent;
}

bool GpLedMatrixEsp32::ShowRgb332FramePreset(GpColorDebugPreset preset) {
    const uint8_t* frame = ResolveFramePreset(preset);

    if (frame == nullptr) {
        return false;
    }

    return ShowRgb332Frame(frame, GP_MATRIX_RGB332_FRAME_SIZE, kGpMatrixModeSolidFrame);
}

bool GpLedMatrixEsp32::ShowRgb332Frame(const uint8_t* frame, size_t length, GpMatrixMode mode) {
    std::lock_guard<std::mutex> lock(mutex_);

    if ((frame == nullptr) || (length != GP_MATRIX_RGB332_FRAME_SIZE)) {
        return false;
    }

    return SendStagedFrame(GP_MATRIX_PAYLOAD_FORMAT_RGB332, frame, length, mode, true);
}

bool GpLedMatrixEsp32::SendStagedFrame(uint8_t frame_format,
                                       const uint8_t* frame_data,
                                       size_t frame_length,
                                       GpMatrixMode mode,
                                       bool ack_each_stage) {
    uint8_t start_payload[GP_MATRIX_FRAME_START_PAYLOAD_BYTES] = {
        frame_format,
        GP_MATRIX_WIDTH,
        GP_MATRIX_HEIGHT,
        static_cast<uint8_t>(frame_length & 0xffU),
        static_cast<uint8_t>((frame_length >> 8) & 0xffU),
    };

    if (!SendCommand(kGpMatrixCommandFrameStart, start_payload, sizeof(start_payload), ack_each_stage)) {
        return false;
    }

    for (size_t offset = 0; offset < frame_length; offset += GP_MATRIX_MAX_CHUNK_DATA) {
        const auto chunk_size = static_cast<uint8_t>(std::min(frame_length - offset,
                                                              static_cast<size_t>(GP_MATRIX_MAX_CHUNK_DATA)));
        std::vector<uint8_t> payload(sizeof(GpMatrixFrameChunkPrefix) + chunk_size);
        payload[0] = static_cast<uint8_t>(offset / GP_MATRIX_MAX_CHUNK_DATA);
        payload[1] = chunk_size;
        std::memcpy(payload.data() + sizeof(GpMatrixFrameChunkPrefix), frame_data + offset, chunk_size);
        if (!SendCommand(kGpMatrixCommandFrameChunk, payload.data(), payload.size(), ack_each_stage)) {
            return false;
        }
    }

    uint8_t mode_payload[1] = {static_cast<uint8_t>(mode)};
    return SendCommand(kGpMatrixCommandFrameCommit, mode_payload, sizeof(mode_payload), true);
}

bool GpLedMatrixEsp32::ShowBitmapFrame(const uint16_t* bitmap_rows,
                                       size_t row_count,
                                       uint32_t primary_rgb888,
                                       uint32_t background_rgb888,
                                       GpMatrixMode mode) {
    std::array<uint8_t, GP_MATRIX_BITMAP_RGB888_FRAME_SIZE> frame_payload = {};
    size_t row_index;

    std::lock_guard<std::mutex> lock(mutex_);

    if ((bitmap_rows == nullptr) || (row_count != GP_MATRIX_HEIGHT)) {
        return false;
    }

    for (row_index = 0; row_index < GP_MATRIX_HEIGHT; ++row_index) {
        frame_payload[row_index * 2U] = static_cast<uint8_t>(bitmap_rows[row_index] & 0xFFU);
        frame_payload[row_index * 2U + 1U] = static_cast<uint8_t>((bitmap_rows[row_index] >> 8) & 0xFFU);
    }
    WriteRgb888(frame_payload.data() + GP_MATRIX_BITMAP_ROWS_BYTES, primary_rgb888);
    WriteRgb888(frame_payload.data() + GP_MATRIX_BITMAP_ROWS_BYTES + 3U, background_rgb888);

    /* Compact bitmap frames fit in one chunk, so only the final commit waits for an ACK. */
    return SendStagedFrame(GP_MATRIX_PAYLOAD_FORMAT_BITMAP_RGB888,
                           frame_payload.data(),
                           frame_payload.size(),
                           mode,
                           false);
}

bool GpLedMatrixEsp32::ShowGlyphRows(const uint16_t* rows, size_t row_count, uint8_t glyph_count, uint8_t glyph_width, uint8_t glyph_spacing) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto total_bytes = static_cast<uint16_t>(row_count * sizeof(uint16_t));
    uint8_t start_payload[GP_MATRIX_SCROLL_START_PAYLOAD_BYTES] = {
        glyph_count,
        glyph_width,
        glyph_spacing,
        static_cast<uint8_t>(total_bytes & 0xffU),
        static_cast<uint8_t>((total_bytes >> 8) & 0xffU),
    };
    if (!SendCommand(kGpMatrixCommandScrollGlyphStart, start_payload, sizeof(start_payload))) {
        return false;
    }

    const auto* byte_rows = reinterpret_cast<const uint8_t*>(rows);
    for (size_t offset = 0; offset < total_bytes; offset += GP_MATRIX_MAX_CHUNK_DATA) {
        const auto chunk_size = static_cast<uint8_t>(std::min(static_cast<size_t>(total_bytes - offset), static_cast<size_t>(GP_MATRIX_MAX_CHUNK_DATA)));
        std::vector<uint8_t> payload(sizeof(GpMatrixFrameChunkPrefix) + chunk_size);
        payload[0] = static_cast<uint8_t>(offset / GP_MATRIX_MAX_CHUNK_DATA);
        payload[1] = chunk_size;
        std::memcpy(payload.data() + sizeof(GpMatrixFrameChunkPrefix), byte_rows + offset, chunk_size);
        if (!SendCommand(kGpMatrixCommandScrollGlyphChunk, payload.data(), payload.size())) {
            return false;
        }
    }

    return SendCommand(kGpMatrixCommandScrollGlyphCommit, nullptr, 0);
}

bool GpLedMatrixEsp32::SendBtDebugLedCommand(uint8_t led_index) {
    std::lock_guard<std::mutex> lock(mutex_);

    return SendDebugLedCommandLocked(led_index, true, true);
}

bool GpLedMatrixEsp32::TrySendBackgroundDebugLedCommand(uint8_t led_index, uint32_t quiet_window_ms) {
    const uint32_t now_ticks = static_cast<uint32_t>(xTaskGetTickCount());
    const uint32_t quiet_window_ticks = static_cast<uint32_t>(pdMS_TO_TICKS(quiet_window_ms));
    std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);

    if (!lock.owns_lock()) {
        return false;
    }

    if ((last_foreground_activity_tick_ != 0U)
        && ((now_ticks - last_foreground_activity_tick_) < quiet_window_ticks)) {
        return false;
    }

    return SendDebugLedCommandLocked(led_index, false, false);
}

bool GpLedMatrixEsp32::SendDebugLedCommandLocked(uint8_t led_index, bool ack_required, bool track_activity) {
    uint8_t payload[GP_MATRIX_DEBUG_LED_PAYLOAD_BYTES] = {led_index};

    if ((led_index != GP_MATRIX_DEBUG_LED_CLEAR) && (led_index > GP_MATRIX_DEBUG_LED_MAX_INDEX)) {
        return false;
    }

    return SendCommand(kGpMatrixCommandSetDebugLed, payload, sizeof(payload), ack_required, track_activity);
}

bool GpLedMatrixEsp32::SetDebugLedFlow(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    uint8_t payload[GP_MATRIX_DEBUG_LED_FLOW_PAYLOAD_BYTES] = {
        static_cast<uint8_t>(enable ? GP_MATRIX_DEBUG_LED_FLOW_START : GP_MATRIX_DEBUG_LED_FLOW_STOP)
    };

    return SendCommand(kGpMatrixCommandSetDebugLedFlow, payload, sizeof(payload), true);
}

bool GpLedMatrixEsp32::SendCommand(uint8_t command,
                                   const uint8_t* payload,
                                   size_t payload_length,
                                   bool ack_required,
                                   bool track_activity) {
    std::vector<uint8_t> buffer(GP_MATRIX_PACKET_HEADER_SIZE + payload_length + 1);
    GpMatrixStatusCode reply_status;
    bool reply_valid;
    uint8_t sequence;

    if (track_activity) {
        last_foreground_activity_tick_ = static_cast<uint32_t>(xTaskGetTickCount());
    }

    buffer[0] = GP_MATRIX_PROTOCOL_MAGIC0;
    buffer[1] = GP_MATRIX_PROTOCOL_MAGIC1;
    buffer[2] = GP_MATRIX_PROTOCOL_VERSION;
    buffer[3] = command;
    buffer[4] = sequence_++;
    buffer[5] = static_cast<uint8_t>(ack_required ? GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED : 0U);
    buffer[6] = static_cast<uint8_t>(payload_length & 0xffU);
    buffer[7] = static_cast<uint8_t>((payload_length >> 8) & 0xffU);
    sequence = buffer[4];

    if (payload_length > 0 && payload != nullptr) {
        std::memcpy(buffer.data() + GP_MATRIX_PACKET_HEADER_SIZE, payload, payload_length);
    }
    buffer.back() = GpMatrixComputeChecksum(buffer.data(), buffer.size() - 1);
    last_payload_summary_ = BuildPayloadSummary(command, payload, payload_length);
    if ((command == kGpMatrixCommandSetDebugLed) && !ack_required) {
        ESP_LOGD(TAG,
                 "[GP_TX] bg cmd=%s seq=%u len=%u %s",
                 CommandShortName(command),
                 static_cast<unsigned int>(sequence),
                 static_cast<unsigned int>(payload_length),
                 last_payload_summary_.c_str());
    } else {
        ESP_LOGI(TAG,
                 "[GP_TX] cmd=%s seq=%u len=%u %s",
                 CommandShortName(command),
                 static_cast<unsigned int>(sequence),
                 static_cast<unsigned int>(payload_length),
                 last_payload_summary_.c_str());
    }

    if ((transport_ == nullptr) || !transport_->WritePacket(buffer.data(), buffer.size(), 100)) {
        link_verified_ = false;
        failure_count_++;
        ESP_LOGW(TAG, "Matrix transport write failed for command 0x%02x", command);
        NotifyLinkStatus(false, BuildStatusText(false, command, sequence, payload_length, ESP_FAIL, kGpMatrixStatusInternalError, false));
        return false;
    }

    success_count_++;
    if (!ack_required) {
        return true;
    }

    reply_valid = ReadReply(sequence, command, reply_status);
    if (!reply_valid) {
        link_verified_ = false;
        no_reply_count_++;
        failure_count_++;
        NotifyLinkStatus(false, BuildStatusText(false, command, sequence, payload_length, ESP_ERR_TIMEOUT, kGpMatrixStatusInternalError, false));
        return false;
    }

    verified_count_++;
    if (reply_status != kGpMatrixStatusOk) {
        link_verified_ = false;
        failure_count_++;
        NotifyLinkStatus(false, BuildStatusText(false, command, sequence, payload_length, ESP_FAIL, reply_status, true));
        return false;
    }

    link_verified_ = true;
    NotifyLinkStatus(true, BuildStatusText(true, command, sequence, payload_length, ESP_OK, reply_status, true));
    return true;
}

bool GpLedMatrixEsp32::ReadReply(uint8_t expected_sequence, uint8_t expected_command, GpMatrixStatusCode& reply_status) {
    std::array<uint8_t, GP_MATRIX_PACKET_HEADER_SIZE + kStatusReplyPayloadBytes + 1> reply = {};

    reply_status = kGpMatrixStatusInternalError;
    vTaskDelay(pdMS_TO_TICKS(4));
    for (uint32_t attempt = 0; attempt < kReplyPollRetries; ++attempt) {
        if ((transport_ != nullptr) && transport_->ReadPacket(reply.data(), reply.size(), 100)) {
            if ((reply[0] != GP_MATRIX_PROTOCOL_MAGIC0)
                || (reply[1] != GP_MATRIX_PROTOCOL_MAGIC1)
                || (reply[2] != GP_MATRIX_PROTOCOL_VERSION)) {
                continue;
            }
            if ((reply[3] != kGpMatrixCommandStatus) && (reply[3] != kGpMatrixCommandError)) {
                continue;
            }
            if (reply[4] != expected_sequence) {
                continue;
            }
            if ((reply[6] != kStatusReplyPayloadBytes) || (reply[7] != 0U)) {
                continue;
            }
            if (GpMatrixComputeChecksum(reply.data(), reply.size() - 1U) != reply.back()) {
                continue;
            }
            if (reply[9] != expected_command) {
                continue;
            }

            reply_status = static_cast<GpMatrixStatusCode>(reply[8]);
            ESP_LOGI(TAG,
                     "[GP_RX] cmd=%s seq=%u reply=%s st=%u",
                     CommandShortName(expected_command),
                     static_cast<unsigned int>(expected_sequence),
                     (reply[3] == kGpMatrixCommandStatus) ? "status" : "error",
                     static_cast<unsigned int>(reply_status));
            return true;
        }

        vTaskDelay(pdMS_TO_TICKS(kReplyPollIntervalMs));
    }

    ESP_LOGW(TAG,
             "[GP_RX] cmd=%s seq=%u timeout",
             CommandShortName(expected_command),
             static_cast<unsigned int>(expected_sequence));
    return false;
}

bool GpLedMatrixEsp32::ActionEquals(const GpMatrixActionPayload& left, const GpMatrixActionPayload& right) {
    return std::memcmp(&left, &right, sizeof(GpMatrixActionPayload)) == 0;
}

const char* GpLedMatrixEsp32::EffectShortName(uint8_t effect) {
    switch (effect) {
    case kGpMatrixEffectStatic:
        return "static";
    case kGpMatrixEffectBreath:
        return "breath";
    case kGpMatrixEffectGradient:
        return "grad";
    case kGpMatrixEffectScrollLeft:
        return "left";
    case kGpMatrixEffectScrollRight:
        return "right";
    case kGpMatrixEffectColorCycle:
        return "cycle";
    case kGpMatrixEffectTextScroll:
        return "scroll";
    default:
        return "fx";
    }
}

const char* GpLedMatrixEsp32::StateShortName(DeviceState state) {
    switch (state) {
    case kDeviceStateIdle:
        return "idle";
    case kDeviceStateConnecting:
        return "conn";
    case kDeviceStateListening:
        return "listen";
    case kDeviceStateSpeaking:
        return "speak";
    case kDeviceStateActivating:
        return "activ";
    case kDeviceStateStarting:
        return "start";
    default:
        return "state";
    }
}

std::string GpLedMatrixEsp32::BuildPayloadSummary(uint8_t command, const uint8_t* payload, size_t payload_length) const {
    char buffer[96] = {0};

    if ((command == kGpMatrixCommandSetAction) && (payload != nullptr) && (payload_length == GP_MATRIX_ACTION_PAYLOAD_BYTES)) {
        const char* content_name = "pattern";
        if (payload[1] == kGpMatrixActionContentSolid) {
            content_name = "solid";
        } else if (payload[1] == kGpMatrixActionContentGlyph) {
            content_name = "glyph";
        }

        std::snprintf(buffer,
                      sizeof(buffer),
                      "%s %s #%02X%02X%02X",
                      content_name,
                      EffectShortName(payload[2]),
                      static_cast<unsigned int>(payload[6]),
                      static_cast<unsigned int>(payload[7]),
                      static_cast<unsigned int>(payload[8]));
        return buffer;
    }

    if ((command == kGpMatrixCommandStateHint) && (payload != nullptr) && (payload_length >= 1U)) {
        std::snprintf(buffer, sizeof(buffer), "state %s", StateShortName(static_cast<DeviceState>(payload[0])));
        return buffer;
    }

    if ((command == kGpMatrixCommandSetBrightness) && (payload != nullptr) && (payload_length == 1U)) {
        std::snprintf(buffer, sizeof(buffer), "brightness 0x%02X", static_cast<unsigned int>(payload[0]));
        return buffer;
    }

    if ((command == kGpMatrixCommandSetDebugLed) && (payload != nullptr)
        && (payload_length == GP_MATRIX_DEBUG_LED_PAYLOAD_BYTES)) {
        if (payload[0] == GP_MATRIX_DEBUG_LED_CLEAR) {
            std::snprintf(buffer, sizeof(buffer), "dbgled clear");
        } else {
            std::snprintf(buffer, sizeof(buffer), "dbgled %u", static_cast<unsigned int>(payload[0]));
        }
        return buffer;
    }

    if ((command == kGpMatrixCommandSetDebugLedFlow) && (payload != nullptr)
        && (payload_length == GP_MATRIX_DEBUG_LED_FLOW_PAYLOAD_BYTES)) {
        std::snprintf(buffer,
                      sizeof(buffer),
                      "dbgflow %s",
                      (payload[0] == GP_MATRIX_DEBUG_LED_FLOW_START) ? "start" : "stop");
        return buffer;
    }

    if ((command == kGpMatrixCommandFrameStart) && (payload != nullptr) && (payload_length == GP_MATRIX_FRAME_START_PAYLOAD_BYTES)) {
        std::snprintf(buffer,
                      sizeof(buffer),
                      "frame %ux%u %uB",
                      static_cast<unsigned int>(payload[1]),
                      static_cast<unsigned int>(payload[2]),
                      static_cast<unsigned int>(payload[3] | (payload[4] << 8)));
        return buffer;
    }

    std::snprintf(buffer, sizeof(buffer), "%s len=%u", CommandShortName(command), static_cast<unsigned int>(payload_length));
    return buffer;
}

std::string GpLedMatrixEsp32::BuildStatusText(bool online,
                                              uint8_t command,
                                              uint8_t sequence,
                                              size_t payload_length,
                                              esp_err_t err,
                                              GpMatrixStatusCode reply_status,
                                              bool reply_valid) const {
    char buffer[96] = {0};
    const char* err_text = "fail";

    if (err == ESP_ERR_TIMEOUT) {
        err_text = "timeout";
    } else if (err == ESP_OK) {
        err_text = "ok";
    }

    if (online) {
        std::snprintf(buffer,
                      sizeof(buffer),
                      "Bluetooth connected\n%s s%02u r%02u\n%s",
                      CommandShortName(command),
                      static_cast<unsigned int>(sequence),
                      static_cast<unsigned int>(reply_status),
                      last_payload_summary_.c_str());
    } else {
        if (reply_valid) {
            std::snprintf(buffer,
                          sizeof(buffer),
                          "Bluetooth error\n%s s%02u r%02u\n%s",
                          CommandShortName(command),
                          static_cast<unsigned int>(sequence),
                          static_cast<unsigned int>(reply_status),
                          last_payload_summary_.c_str());
        } else {
            std::snprintf(buffer,
                          sizeof(buffer),
                          "Bluetooth waiting\n%s s%02u %s\n%s",
                          CommandShortName(command),
                          static_cast<unsigned int>(sequence),
                          err_text,
                          last_payload_summary_.c_str());
        }
    }

    (void)payload_length;
    return buffer;
}

std::string GpLedMatrixEsp32::BuildHeartbeatStatusText(uint8_t led_index) const {
    char buffer[64] = {0};

    std::snprintf(buffer,
                  sizeof(buffer),
                  "%s\nLED %u",
                  link_verified_ ? "Bluetooth connected" : "Bluetooth waiting",
                  static_cast<unsigned int>(led_index));
    return buffer;
}

void GpLedMatrixEsp32::NotifyLinkStatus(bool online, const std::string& status_text) {
    if (link_status_callback_) {
        link_status_callback_(online, status_text);
    }
}

const char* GpLedMatrixEsp32::CommandShortName(uint8_t command) {
    switch (command) {
    case kGpMatrixCommandPing:
        return "ping";
    case kGpMatrixCommandSetBrightness:
        return "bri";
    case kGpMatrixCommandSetMode:
        return "mode";
    case kGpMatrixCommandStateHint:
        return "state";
    case kGpMatrixCommandSetAction:
        return "act";
    case kGpMatrixCommandSetDebugLed:
        return "dled";
    case kGpMatrixCommandSetDebugLedFlow:
        return "dflw";
    case kGpMatrixCommandFrameStart:
        return "fstr";
    case kGpMatrixCommandFrameChunk:
        return "fchk";
    case kGpMatrixCommandFrameCommit:
        return "fcom";
    case kGpMatrixCommandScrollGlyphStart:
        return "gstr";
    case kGpMatrixCommandScrollGlyphChunk:
        return "gchk";
    case kGpMatrixCommandScrollGlyphCommit:
        return "gcom";
    case kGpMatrixCommandHeartbeat:
        return "beat";
    default:
        return "cmd";
    }
}

bool GpLedMatrixEsp32::SendState(DeviceState state, const Rgb332Frame& frame, GpMatrixMode mode) {
    uint8_t payload[2] = {static_cast<uint8_t>(state), brightness_};
    if (!SendCommand(kGpMatrixCommandStateHint, payload, sizeof(payload))) {
        return false;
    }
    if (!ShowRgb332Frame(frame.data(), frame.size(), mode)) {
        return false;
    }

    has_last_state_ = true;
    last_state_ = state;
    return true;
}

GpLedMatrixEsp32::Rgb332Frame GpLedMatrixEsp32::BuildFrameForState(DeviceState state) const {
    Rgb332Frame frame;

    switch (state) {
        case kDeviceStateStarting:
            DrawDiamond(frame, kColorBlue, kColorOff);
            break;
        case kDeviceStateWifiConfiguring:
            DrawCross(frame, kColorBlue, kColorOff);
            break;
        case kDeviceStateIdle:
            FillFrame(frame, kColorOff);
            break;
        case kDeviceStateConnecting:
            DrawBorder(frame, kColorYellow, kColorOff);
            break;
        case kDeviceStateListening:
        case kDeviceStateAudioTesting:
            DrawBars(frame, kColorOrange, kColorRed, kColorOff);
            break;
        case kDeviceStateSpeaking:
            DrawBars(frame, kColorGreen, kColorCyan, kColorOff);
            break;
        case kDeviceStateUpgrading:
            DrawDiamond(frame, kColorCyan, kColorOff);
            break;
        case kDeviceStateActivating:
            DrawDiamond(frame, kColorPurple, kColorOff);
            break;
        case kDeviceStateFatalError:
            DrawCross(frame, kColorRed, kColorOff);
            break;
        case kDeviceStateUnknown:
        default:
            DrawBorder(frame, kColorWhite, kColorOff);
            break;
    }

    return frame;
}

void GpLedMatrixEsp32::FillFrame(Rgb332Frame& frame, uint8_t color) {
    frame.fill(color);
}

void GpLedMatrixEsp32::DrawCross(Rgb332Frame& frame, uint8_t color, uint8_t background) {
    FillFrame(frame, background);
    for (size_t y = 0; y < GP_MATRIX_HEIGHT; ++y) {
        for (size_t x = 0; x < GP_MATRIX_WIDTH; ++x) {
            if (x == y || x + y == GP_MATRIX_WIDTH - 1) {
                frame[y * GP_MATRIX_WIDTH + x] = color;
            }
        }
    }
}

void GpLedMatrixEsp32::DrawDiamond(Rgb332Frame& frame, uint8_t color, uint8_t background) {
    FillFrame(frame, background);
    constexpr int center = GP_MATRIX_WIDTH / 2;
    for (int y = 0; y < static_cast<int>(GP_MATRIX_HEIGHT); ++y) {
        for (int x = 0; x < static_cast<int>(GP_MATRIX_WIDTH); ++x) {
            const int distance = std::abs(center - x) + std::abs(center - y);
            if (distance <= center - 1) {
                frame[y * GP_MATRIX_WIDTH + x] = color;
            }
        }
    }
}

void GpLedMatrixEsp32::DrawBorder(Rgb332Frame& frame, uint8_t color, uint8_t background) {
    FillFrame(frame, background);
    for (size_t y = 0; y < GP_MATRIX_HEIGHT; ++y) {
        for (size_t x = 0; x < GP_MATRIX_WIDTH; ++x) {
            if (x == 0 || y == 0 || x == GP_MATRIX_WIDTH - 1 || y == GP_MATRIX_HEIGHT - 1) {
                frame[y * GP_MATRIX_WIDTH + x] = color;
            }
        }
    }
}

void GpLedMatrixEsp32::DrawBars(Rgb332Frame& frame, uint8_t first, uint8_t second, uint8_t background) {
    FillFrame(frame, background);
    for (size_t y = 0; y < GP_MATRIX_HEIGHT; ++y) {
        const uint8_t color = (y % 4U < 2U) ? first : second;
        for (size_t x = 3; x < GP_MATRIX_WIDTH - 3; ++x) {
            frame[y * GP_MATRIX_WIDTH + x] = color;
        }
    }
}

const uint8_t* GpLedMatrixEsp32::ResolveFramePreset(GpColorDebugPreset preset) {
    switch (preset) {
    case GpColorDebugPreset::kPythonDemo:
        return kPythonDemoFrame;
    default:
        return nullptr;
    }
}