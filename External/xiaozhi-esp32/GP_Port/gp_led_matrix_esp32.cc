#include "gp_led_matrix_esp32.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include <esp_log.h>

#include "application.h"

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
}

GpLedMatrixEsp32::GpLedMatrixEsp32(i2c_master_bus_handle_t i2c_bus, uint8_t address, uint8_t brightness)
    : I2cDevice(i2c_bus, address), brightness_(brightness), sequence_(0) {
    SetBrightness(brightness_);
}

void GpLedMatrixEsp32::OnStateChanged() {
    auto state = Application::GetInstance().GetDeviceState();
    auto frame = BuildFrameForState(state);
    SendState(state, frame);
}

void GpLedMatrixEsp32::SetBrightness(uint8_t brightness) {
    std::lock_guard<std::mutex> lock(mutex_);
    brightness_ = brightness;
    uint8_t payload[1] = {brightness_};
    SendCommand(kGpMatrixCommandSetBrightness, payload, sizeof(payload));
}

bool GpLedMatrixEsp32::ShowRgb332Frame(const uint8_t* frame, size_t length, GpMatrixMode mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    GpMatrixFrameStartPayload start_payload = {
        GP_MATRIX_PAYLOAD_FORMAT_RGB332,
        GP_MATRIX_WIDTH,
        GP_MATRIX_HEIGHT,
        static_cast<uint16_t>(length)
    };
    if (!SendCommand(kGpMatrixCommandFrameStart,
            reinterpret_cast<const uint8_t*>(&start_payload), sizeof(start_payload))) {
        return false;
    }

    for (size_t offset = 0; offset < length; offset += GP_MATRIX_MAX_CHUNK_DATA) {
        const auto chunk_size = static_cast<uint8_t>(std::min(length - offset, static_cast<size_t>(GP_MATRIX_MAX_CHUNK_DATA)));
        std::vector<uint8_t> payload(sizeof(GpMatrixFrameChunkPrefix) + chunk_size);
        payload[0] = static_cast<uint8_t>(offset / GP_MATRIX_MAX_CHUNK_DATA);
        payload[1] = chunk_size;
        std::memcpy(payload.data() + sizeof(GpMatrixFrameChunkPrefix), frame + offset, chunk_size);
        if (!SendCommand(kGpMatrixCommandFrameChunk, payload.data(), payload.size())) {
            return false;
        }
    }

    uint8_t mode_payload[1] = {static_cast<uint8_t>(mode)};
    return SendCommand(kGpMatrixCommandFrameCommit, mode_payload, sizeof(mode_payload));
}

bool GpLedMatrixEsp32::ShowGlyphRows(const uint16_t* rows, size_t row_count, uint8_t glyph_count, uint8_t glyph_width, uint8_t glyph_spacing) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto total_bytes = static_cast<uint16_t>(row_count * sizeof(uint16_t));
    GpMatrixScrollStartPayload start_payload = {
        glyph_count,
        glyph_width,
        glyph_spacing,
        total_bytes
    };
    if (!SendCommand(kGpMatrixCommandScrollGlyphStart,
            reinterpret_cast<const uint8_t*>(&start_payload), sizeof(start_payload))) {
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

bool GpLedMatrixEsp32::SendCommand(uint8_t command, const uint8_t* payload, size_t payload_length, bool ack_required) {
    GpMatrixPacketHeader header = {
        GP_MATRIX_PROTOCOL_MAGIC0,
        GP_MATRIX_PROTOCOL_MAGIC1,
        GP_MATRIX_PROTOCOL_VERSION,
        command,
        sequence_++,
        static_cast<uint8_t>(ack_required ? GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED : 0),
        static_cast<uint16_t>(payload_length)
    };

    std::vector<uint8_t> buffer(sizeof(header) + payload_length + 1);
    std::memcpy(buffer.data(), &header, sizeof(header));
    if (payload_length > 0 && payload != nullptr) {
        std::memcpy(buffer.data() + sizeof(header), payload, payload_length);
    }
    buffer.back() = GpMatrixComputeChecksum(buffer.data(), buffer.size() - 1);

    auto err = i2c_master_transmit(i2c_device_, buffer.data(), buffer.size(), 100);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "I2C command 0x%02x failed: %s", command, esp_err_to_name(err));
        return false;
    }
    return true;
}

bool GpLedMatrixEsp32::SendState(DeviceState state, const Rgb332Frame& frame, GpMatrixMode mode) {
    uint8_t payload[2] = {static_cast<uint8_t>(state), brightness_};
    if (!SendCommand(kGpMatrixCommandStateHint, payload, sizeof(payload))) {
        return false;
    }
    return ShowRgb332Frame(frame.data(), frame.size(), mode);
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