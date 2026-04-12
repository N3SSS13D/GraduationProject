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

#define GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED 0x01U

#define GP_MATRIX_PAYLOAD_FORMAT_RGB332 0x01U
#define GP_MATRIX_PAYLOAD_FORMAT_GLYPH_ROWS 0x02U

#define GP_MATRIX_WIDTH 16U
#define GP_MATRIX_HEIGHT 16U
#define GP_MATRIX_FRAME_PIXELS (GP_MATRIX_WIDTH * GP_MATRIX_HEIGHT)
#define GP_MATRIX_RGB332_FRAME_SIZE GP_MATRIX_FRAME_PIXELS
#define GP_MATRIX_GLYPH_ROWS_SIZE (GP_MATRIX_HEIGHT * 2U)

#define GP_MATRIX_MAX_CHUNK_DATA 192U

typedef enum GpMatrixCommand {
    kGpMatrixCommandPing = 0x01,
    kGpMatrixCommandSetBrightness = 0x02,
    kGpMatrixCommandSetMode = 0x03,
    kGpMatrixCommandStateHint = 0x04,
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

static inline uint8_t GpMatrixComputeChecksum(const uint8_t* data, size_t length) {
    uint8_t checksum = 0;
    size_t index;

    for (index = 0; index < length; ++index) {
        checksum ^= data[index];
    }
    return checksum;
}

#endif