#ifndef GP_LED_MATRIX_PROTOCOL_H_
#define GP_LED_MATRIX_PROTOCOL_H_

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif

#define GP_MATRIX_PROTOCOL_MAGIC0 0x47U
#define GP_MATRIX_PROTOCOL_MAGIC1 0x50U
#define GP_MATRIX_PROTOCOL_VERSION 0x01U

#define GP_MATRIX_DEFAULT_I2C_ADDRESS 0x31U

#define GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED 0x01U
#define GP_MATRIX_PROTOCOL_FLAG_LOCAL_ONLY 0x02U

#define GP_MATRIX_ACTION_FLAG_USE_SECONDARY 0x01U
#define GP_MATRIX_ACTION_FLAG_REMOTE_ENABLE 0x02U
#define GP_MATRIX_ACTION_FLAG_REMOTE_RELEASE 0x04U

#define GP_MATRIX_PAYLOAD_FORMAT_RGB332 0x01U
#define GP_MATRIX_PAYLOAD_FORMAT_GLYPH_ROWS 0x02U

#define GP_MATRIX_WIDTH 16U
#define GP_MATRIX_HEIGHT 16U
#define GP_MATRIX_FRAME_PIXELS (GP_MATRIX_WIDTH * GP_MATRIX_HEIGHT)
#define GP_MATRIX_RGB332_FRAME_SIZE GP_MATRIX_FRAME_PIXELS
#define GP_MATRIX_GLYPH_ROWS_SIZE (GP_MATRIX_HEIGHT * 2U)

#define GP_MATRIX_MAX_CHUNK_DATA 64U
#define GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES 128U
#define GP_MATRIX_PACKET_HEADER_SIZE 8U
#define GP_MATRIX_PACKET_OVERHEAD_SIZE (GP_MATRIX_PACKET_HEADER_SIZE + 1U)
#define GP_MATRIX_FRAME_START_PAYLOAD_BYTES 5U
#define GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES 2U
#define GP_MATRIX_SCROLL_START_PAYLOAD_BYTES 5U
#define GP_MATRIX_ACTION_PAYLOAD_BYTES 18U

typedef enum GpMatrixCommand {
    kGpMatrixCommandPing = 0x01,
    kGpMatrixCommandSetBrightness = 0x02,
    kGpMatrixCommandSetMode = 0x03,
    kGpMatrixCommandStateHint = 0x04,
    kGpMatrixCommandSetAction = 0x05,
    kGpMatrixCommandFrameStart = 0x10,
    kGpMatrixCommandFrameChunk = 0x11,
    kGpMatrixCommandFrameCommit = 0x12,
    kGpMatrixCommandScrollGlyphStart = 0x20,
    kGpMatrixCommandScrollGlyphChunk = 0x21,
    kGpMatrixCommandScrollGlyphCommit = 0x22,
    kGpMatrixCommandHeartbeat = 0x30,
    kGpMatrixCommandStatus = 0x31,
    kGpMatrixCommandError = 0x7f,
} GpMatrixCommand;

typedef enum GpMatrixMode {
    kGpMatrixModeSolidFrame = 0x00,
    kGpMatrixModeBlink = 0x01,
    kGpMatrixModeScroll = 0x02,
    kGpMatrixModeBreath = 0x03,
} GpMatrixMode;

typedef enum GpMatrixActionSource {
    kGpMatrixActionSourceLocal = 0x00,
    kGpMatrixActionSourceMcp = 0x01,
} GpMatrixActionSource;

typedef enum GpMatrixActionContent {
    kGpMatrixActionContentSolid = 0x00,
    kGpMatrixActionContentPattern = 0x01,
    kGpMatrixActionContentGlyph = 0x02,
    kGpMatrixActionContentState = 0x03,
    kGpMatrixActionContentFrame = 0x04,
} GpMatrixActionContent;

typedef enum GpMatrixEffect {
    kGpMatrixEffectStatic = 0x00,
    kGpMatrixEffectBreath = 0x01,
    kGpMatrixEffectGradient = 0x02,
    kGpMatrixEffectScrollLeft = 0x03,
    kGpMatrixEffectScrollRight = 0x04,
    kGpMatrixEffectTextScroll = 0x05,
    kGpMatrixEffectFadeIn = 0x06,
    kGpMatrixEffectFadeOut = 0x07,
    kGpMatrixEffectColorCycle = 0x08,
} GpMatrixEffect;

typedef enum GpMatrixDirection {
    kGpMatrixDirectionNormal = 0x00,
    kGpMatrixDirectionRotate180 = 0x01,
    kGpMatrixDirectionRotateCw90 = 0x02,
    kGpMatrixDirectionRotateCcw90 = 0x03,
} GpMatrixDirection;

typedef enum GpMatrixColorMode {
    kGpMatrixColorModeSolid = 0x00,
    kGpMatrixColorModeGradient = 0x01,
} GpMatrixColorMode;

typedef enum GpMatrixStatusCode {
    kGpMatrixStatusOk = 0x00,
    kGpMatrixStatusBusy = 0x01,
    kGpMatrixStatusUnsupported = 0x02,
    kGpMatrixStatusBadChecksum = 0x03,
    kGpMatrixStatusBadSequence = 0x04,
    kGpMatrixStatusBadLength = 0x05,
    kGpMatrixStatusInternalError = 0x7f,
} GpMatrixStatusCode;

typedef struct GpMatrixPacketHeader {
    uint8_t magic0;
    uint8_t magic1;
    uint8_t version;
    uint8_t command;
    uint8_t sequence;
    uint8_t flags;
    uint16_t payload_length;
} GpMatrixPacketHeader;

typedef struct GpMatrixFrameStartPayload {
    uint8_t format;
    uint8_t width;
    uint8_t height;
    uint16_t total_bytes;
} GpMatrixFrameStartPayload;

typedef struct GpMatrixFrameChunkPrefix {
    uint8_t offset;
    uint8_t size;
} GpMatrixFrameChunkPrefix;

typedef struct GpMatrixScrollStartPayload {
    uint8_t glyph_count;
    uint8_t glyph_width;
    uint8_t glyph_spacing;
    uint16_t total_bytes;
} GpMatrixScrollStartPayload;

typedef struct GpMatrixActionPayload {
    uint8_t source;
    uint8_t content;
    uint8_t effect;
    uint8_t direction;
    uint8_t color_mode;
    uint8_t brightness;
    uint8_t primary_r;
    uint8_t primary_g;
    uint8_t primary_b;
    uint8_t secondary_r;
    uint8_t secondary_g;
    uint8_t secondary_b;
    uint8_t pattern_id;
    uint8_t glyph_id;
    uint8_t scroll_step;
    uint8_t anim_step;
    uint8_t gradient_span;
    uint8_t flags;
} GpMatrixActionPayload;

static inline uint8_t GpMatrixComputeChecksum(const uint8_t* data, size_t length) {
    uint8_t checksum = 0;
    size_t index;

    for (index = 0; index < length; ++index) {
        checksum ^= data[index];
    }
    return checksum;
}

#endif