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
constexpr size_t kReplyMinPayloadBytes = 1U;
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

void PackLayeredFramePayload(std::vector<uint8_t>& frame_payload,
                             const std::vector<GpLedMatrixEsp32::LayeredFrameLayer>& layers) {
    const uint8_t total_layers = static_cast<uint8_t>(layers.size());

    frame_payload.clear();
    frame_payload.reserve(static_cast<size_t>(total_layers) * GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES);

    for (uint8_t layer_seq = 0U; layer_seq < total_layers; ++layer_seq) {
        const auto& layer = layers[layer_seq];
        const uint8_t header_byte = static_cast<uint8_t>((total_layers << 4) | (layer_seq & 0x0FU));
        uint8_t color_buf[GP_MATRIX_BITMAP_LAYER_COLOR_BYTES];

        frame_payload.push_back(header_byte);
        for (size_t row_index = 0; row_index < GP_MATRIX_HEIGHT; ++row_index) {
            frame_payload.push_back(static_cast<uint8_t>(layer.bitmap_rows[row_index] & 0xFFU));
            frame_payload.push_back(static_cast<uint8_t>((layer.bitmap_rows[row_index] >> 8) & 0xFFU));
        }
        WriteRgb888(color_buf, layer.color_rgb888);
        frame_payload.insert(frame_payload.end(), color_buf, color_buf + GP_MATRIX_BITMAP_LAYER_COLOR_BYTES);
    }
}

uint8_t BuildMatrixAnimStep(const GpColorDebugState& state) {
    if (state.animation_period_ms <= 900U) {
        return 2U;
    }
    return 1U;
}

static_assert(sizeof(GpMatrixPacketHeader) == GP_MATRIX_PACKET_HEADER_SIZE,
              "Unexpected GP matrix V2 header size");
static_assert(sizeof(GpMatrixPacketHeaderV3) == GP_MATRIX_PACKET_HEADER_SIZE_V3,
              "Unexpected GP matrix V3 header size");
static_assert(sizeof(GpMatrixFrameChunkPrefix) == GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES,
              "Unexpected GP matrix chunk prefix size");

GpMatrixPacketHeaderV3 BuildPacketHeaderV3(uint8_t flags,
                                           uint8_t sequence,
                                           uint8_t command,
                                           uint8_t payload_length) {
    GpMatrixPacketHeaderV3 header = {};

    header.magic = GP_MATRIX_PROTOCOL_MAGIC;
    header.flags = flags & (GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED | GP_MATRIX_PROTOCOL_FLAG_LOCAL_ONLY);
    header.sequence = sequence;
    header.command = command;
    header.payload_length = payload_length;
    header.header_crc8 = GpMatrixComputeHeaderCrc8(reinterpret_cast<const uint8_t*>(&header),
                                                    GP_MATRIX_PACKET_HEADER_CRC_BYTES_V3);
    return header;
}

GpMatrixPacketHeader BuildPacketHeader(GpMatrixPacketType packet_type,
                                       uint8_t flags,
                                       uint8_t sequence,
                                       uint8_t reply_to_sequence,
                                       uint8_t command,
                                       uint16_t payload_length) {
    GpMatrixPacketHeader header = {};

    header.magic0 = GP_MATRIX_PROTOCOL_MAGIC0;
    header.magic1 = GP_MATRIX_PROTOCOL_MAGIC1;
    header.version = GP_MATRIX_PROTOCOL_VERSION;
    header.header_size = GP_MATRIX_PACKET_HEADER_SIZE;
    header.packet_type = static_cast<uint8_t>(packet_type);
    header.flags = flags;
    header.sequence = sequence;
    header.reply_to_sequence = reply_to_sequence;
    GP_MATRIX_WRITE_LE16(&header.payload_length_lo, payload_length);
    header.command = command;
    header.header_crc8 = GpMatrixComputeHeaderCrc8(reinterpret_cast<const uint8_t*>(&header),
                                                   GP_MATRIX_PACKET_HEADER_CRC_BYTES);
    return header;
}

void WriteChunkPrefix(uint8_t* payload, uint16_t offset, uint8_t chunk_size) {
    GP_MATRIX_WRITE_LE16(payload, offset);
    payload[2] = chunk_size;
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
        WriteChunkPrefix(payload.data(), static_cast<uint16_t>(offset), chunk_size);
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
    std::lock_guard<std::mutex> lock(mutex_);

    if ((bitmap_rows == nullptr) || (row_count != GP_MATRIX_HEIGHT)) {
        return false;
    }

    /* Convert legacy 2-color API to layered format internally. */
    std::vector<LayeredFrameLayer> layers(2U);
    for (size_t row_index = 0; row_index < GP_MATRIX_HEIGHT; ++row_index) {
        layers[0].bitmap_rows[row_index] = 0xFFFFU;
        layers[1].bitmap_rows[row_index] = bitmap_rows[row_index];
    }
    layers[0].color_rgb888 = background_rgb888;
    layers[1].color_rgb888 = primary_rgb888;

    return ShowLayeredFrameLocked(layers, mode);
}

bool GpLedMatrixEsp32::ShowLayeredFrame(const std::vector<LayeredFrameLayer>& layers, GpMatrixMode mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    return ShowLayeredFrameLocked(layers, mode);
}

bool GpLedMatrixEsp32::ShowLayeredFrameLocked(const std::vector<LayeredFrameLayer>& layers, GpMatrixMode mode) {
    std::vector<uint8_t> frame_payload;

    if (layers.empty() || (layers.size() > GP_MATRIX_BITMAP_LAYERED_MAX_COLORS)) {
        return false;
    }

    PackLayeredFramePayload(frame_payload, layers);

    /* Use lightweight single-packet command when the layered payload fits.
       Avoids FrameStart/FrameChunk/FrameCommit round-trip overhead. */
    if (frame_payload.size() <= GP_MATRIX_LAYERED_FRAME_MAX_PAYLOAD) {
        return SendCommand(kGpMatrixCommandLayeredFrame,
                           frame_payload.data(),
                           frame_payload.size(),
                           true);
    }

    return SendStagedFrame(GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED,
                           frame_payload.data(),
                           frame_payload.size(),
                           mode,
                           false);
}

bool GpLedMatrixEsp32::ShowLayeredAnimation(const std::vector<std::vector<LayeredFrameLayer>>& frameLayers,
                                            uint16_t frame_interval_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    return ShowLayeredAnimationLocked(frameLayers, frame_interval_ms);
}

bool GpLedMatrixEsp32::ShowLayeredAnimationLocked(const std::vector<std::vector<LayeredFrameLayer>>& frameLayers,
                                                   uint16_t frame_interval_ms) {
    uint8_t start_payload[GP_MATRIX_ANIMATION_START_PAYLOAD_BYTES] = {
        GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED,
        0U,
        0U,
        0U,
        GP_MATRIX_ANIMATION_FLAG_LOOP,
    };
    uint8_t end_payload[GP_MATRIX_ANIMATION_END_PAYLOAD_BYTES] = {0U};
    size_t frame_index;
    uint8_t frame_count;
    uint16_t resolved_interval_ms;

    if (frameLayers.empty() || (frameLayers.size() > GP_MATRIX_ANIMATION_MAX_FRAMES)) {
        return false;
    }

    /* Validate every frame's layer count is consistent and within limits. */
    frame_count = static_cast<uint8_t>(frameLayers.size());
    for (frame_index = 0U; frame_index < frame_count; ++frame_index) {
        if (frameLayers[frame_index].empty()
            || (frameLayers[frame_index].size() > GP_MATRIX_ANIMATION_MAX_LAYERS)) {
            return false;
        }
    }

    resolved_interval_ms = frame_interval_ms;
    if (resolved_interval_ms < GP_MATRIX_ANIMATION_INTERVAL_MS_MIN) {
        resolved_interval_ms = GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS;
    }

    start_payload[1] = frame_count;
    start_payload[2] = static_cast<uint8_t>(resolved_interval_ms & 0xFFU);
    start_payload[3] = static_cast<uint8_t>((resolved_interval_ms >> 8) & 0xFFU);
    if (!SendCommand(kGpMatrixCommandAnimationStart, start_payload, sizeof(start_payload), true)) {
        return false;
    }

    for (frame_index = 0U; frame_index < frame_count; ++frame_index) {
        std::vector<uint8_t> frame_payload;
        std::vector<uint8_t> payload;

        PackLayeredFramePayload(frame_payload, frameLayers[frame_index]);
        payload.reserve(1U + frame_payload.size());
        payload.push_back(static_cast<uint8_t>(frame_index));
        payload.insert(payload.end(), frame_payload.begin(), frame_payload.end());
        if (!SendCommand(kGpMatrixCommandLayeredAnimFrame, payload.data(), payload.size(), true)) {
            return false;
        }
    }

    end_payload[0] = frame_count;
    return SendCommand(kGpMatrixCommandAnimationEnd, end_payload, sizeof(end_payload), true);
}

bool GpLedMatrixEsp32::ShowBitmapAnimation(const std::vector<BitmapAnimationFrame>& frames,
                                           uint16_t frame_interval_ms) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (frames.empty() || (frames.size() > GP_MATRIX_ANIMATION_MAX_FRAMES)) {
        return false;
    }

    /* Convert legacy 2-color animation to layered format internally. */
    std::vector<std::vector<LayeredFrameLayer>> layeredFrames;
    layeredFrames.reserve(frames.size());
    for (const auto& frame : frames) {
        std::vector<LayeredFrameLayer> frameLayers(2U);
        for (size_t row_index = 0; row_index < GP_MATRIX_HEIGHT; ++row_index) {
            frameLayers[0].bitmap_rows[row_index] = 0xFFFFU;
            frameLayers[1].bitmap_rows[row_index] = frame.bitmap_rows[row_index];
        }
        frameLayers[0].color_rgb888 = frame.background_rgb888;
        frameLayers[1].color_rgb888 = frame.primary_rgb888;
        layeredFrames.push_back(std::move(frameLayers));
    }

    return ShowLayeredAnimationLocked(layeredFrames, frame_interval_ms);
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
        WriteChunkPrefix(payload.data(), static_cast<uint16_t>(offset), chunk_size);
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
    const uint8_t flags = static_cast<uint8_t>(ack_required ? GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED : 0U);
    const bool use_v3 = ((command == kGpMatrixCommandLayeredFrame)
                         || (command == kGpMatrixCommandLayeredAnimFrame));
    const uint8_t actual_header_size = static_cast<uint8_t>(use_v3
        ? GP_MATRIX_PACKET_HEADER_SIZE_V3 : GP_MATRIX_PACKET_HEADER_SIZE);
    std::vector<uint8_t> buffer(static_cast<size_t>(actual_header_size)
                                + payload_length + GP_MATRIX_PACKET_TRAILER_SIZE);
    GpMatrixStatusCode reply_status;
    GpMatrixPacketHeader header = {};
    uint16_t packet_crc;
    bool reply_valid;
    uint8_t sequence;

    if (track_activity) {
        last_foreground_activity_tick_ = static_cast<uint32_t>(xTaskGetTickCount());
    }

    if (payload_length > GP_MATRIX_PACKET_MAX_PAYLOAD_BYTES) {
        ESP_LOGW(TAG,
                 "Matrix payload too large for command 0x%02x: len=%u",
                 command,
                 static_cast<unsigned int>(payload_length));
        return false;
    }

    sequence = sequence_++;

    /* Use V3 compact header (6 bytes) for lightweight commands, V2 for legacy. */
    if (use_v3)
    {
        const auto header_v3 = BuildPacketHeaderV3(flags, sequence, command,
                                                    static_cast<uint8_t>(payload_length));
        std::memcpy(buffer.data(), &header_v3, sizeof(header_v3));
        if (payload_length > 0 && payload != nullptr) {
            std::memcpy(buffer.data() + GP_MATRIX_PACKET_HEADER_SIZE_V3, payload, payload_length);
        }
        packet_crc = GpMatrixComputePacketCrc16(buffer.data(),
                                                 static_cast<uint16_t>(GP_MATRIX_PACKET_HEADER_SIZE_V3
                                                                       + payload_length));
        GP_MATRIX_WRITE_LE16(buffer.data() + GP_MATRIX_PACKET_HEADER_SIZE_V3 + payload_length,
                              packet_crc);
    }
    else
    {
        header = BuildPacketHeader(kGpMatrixPacketTypeRequest,
                                   flags,
                                   sequence,
                                   0U,
                                   command,
                                   static_cast<uint16_t>(payload_length));
        std::memcpy(buffer.data(), &header, sizeof(header));
        if (payload_length > 0 && payload != nullptr) {
            std::memcpy(buffer.data() + GP_MATRIX_PACKET_HEADER_SIZE, payload, payload_length);
        }
        packet_crc = GpMatrixComputePacketCrc16(buffer.data(),
                                                 static_cast<uint16_t>(buffer.size() - GP_MATRIX_PACKET_TRAILER_SIZE));
        GP_MATRIX_WRITE_LE16(buffer.data() + buffer.size() - GP_MATRIX_PACKET_TRAILER_SIZE, packet_crc);
    }
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
    std::array<uint8_t, GP_MATRIX_PACKET_MAX_SIZE> reply = {};
    size_t reply_length = 0U;

    reply_status = kGpMatrixStatusInternalError;
    vTaskDelay(pdMS_TO_TICKS(4));
    for (uint32_t attempt = 0; attempt < kReplyPollRetries; ++attempt) {
        if ((transport_ != nullptr) && transport_->ReadPacket(reply.data(), reply.size(), &reply_length, 100)) {
            const auto* header = reinterpret_cast<const GpMatrixPacketHeader*>(reply.data());
            uint16_t payload_length;
            uint16_t packet_crc;
            uint8_t reply_detail;

            if (reply_length < GP_MATRIX_PACKET_OVERHEAD_SIZE) {
                continue;
            }
            if ((header->magic0 != GP_MATRIX_PROTOCOL_MAGIC0)
                || (header->magic1 != GP_MATRIX_PROTOCOL_MAGIC1)
                || (header->version != GP_MATRIX_PROTOCOL_VERSION)) {
                continue;
            }
            if ((header->header_size != GP_MATRIX_PACKET_HEADER_SIZE)
                || (header->packet_type != kGpMatrixPacketTypeReply)) {
                continue;
            }
            if (GpMatrixComputeHeaderCrc8(reply.data(), GP_MATRIX_PACKET_HEADER_CRC_BYTES) != header->header_crc8) {
                continue;
            }
            payload_length = GP_MATRIX_READ_LE16(&header->payload_length_lo);
            if ((static_cast<size_t>(header->header_size) + payload_length + GP_MATRIX_PACKET_TRAILER_SIZE) != reply_length) {
                continue;
            }
            if ((header->reply_to_sequence != expected_sequence) || (header->command != expected_command)) {
                continue;
            }
            if (payload_length < kReplyMinPayloadBytes) {
                continue;
            }

            packet_crc = GpMatrixComputePacketCrc16(reply.data(),
                                                    static_cast<uint16_t>(reply_length - GP_MATRIX_PACKET_TRAILER_SIZE));
            if (packet_crc != GP_MATRIX_READ_LE16(reply.data() + reply_length - GP_MATRIX_PACKET_TRAILER_SIZE)) {
                continue;
            }

            reply_status = static_cast<GpMatrixStatusCode>(reply[GP_MATRIX_PACKET_HEADER_SIZE]);
            reply_detail = (payload_length > 1U) ? reply[GP_MATRIX_PACKET_HEADER_SIZE + 1U] : 0U;
            ESP_LOGI(TAG,
                     "[GP_RX] cmd=%s seq=%u st=%u detail=%u",
                     CommandShortName(expected_command),
                     static_cast<unsigned int>(expected_sequence),
                     static_cast<unsigned int>(reply_status),
                     static_cast<unsigned int>(reply_detail));
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

    if ((command == kGpMatrixCommandAnimationStart) && (payload != nullptr)
        && (payload_length == GP_MATRIX_ANIMATION_START_PAYLOAD_BYTES)) {
        const unsigned int interval_ms = static_cast<unsigned int>(payload[2] | (payload[3] << 8));

        std::snprintf(buffer,
                      sizeof(buffer),
                      "anim %u@%ums",
                      static_cast<unsigned int>(payload[1]),
                      interval_ms);
        return buffer;
    }

    if ((command == kGpMatrixCommandAnimationFrame) && (payload != nullptr)
        && (payload_length >= GP_MATRIX_ANIMATION_FRAME_PREFIX_BYTES + GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)) {
        const unsigned int layer_count = static_cast<unsigned int>(payload[1] >> 4);
        std::snprintf(buffer,
                      sizeof(buffer),
                      "anim frame %u layers=%u len=%u",
                      static_cast<unsigned int>(payload[0]),
                      layer_count,
                      static_cast<unsigned int>(payload_length));
        return buffer;
    }

    if ((command == kGpMatrixCommandAnimationEnd) && (payload != nullptr)
        && (payload_length == GP_MATRIX_ANIMATION_END_PAYLOAD_BYTES)) {
        std::snprintf(buffer,
                      sizeof(buffer),
                      "anim end %u",
                      static_cast<unsigned int>(payload[0]));
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
    case kGpMatrixCommandAnimationStart:
        return "astr";
    case kGpMatrixCommandAnimationFrame:
        return "afrm";
    case kGpMatrixCommandAnimationEnd:
        return "aend";
    case kGpMatrixCommandLayeredFrame:
        return "lfr";
    case kGpMatrixCommandLayeredAnimFrame:
        return "lafr";
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