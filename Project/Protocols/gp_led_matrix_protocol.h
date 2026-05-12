#ifndef GP_LED_MATRIX_PROTOCOL_H_
#define GP_LED_MATRIX_PROTOCOL_H_

#ifdef __cplusplus
#include <cstdint>
#endif

/* On AI8051U builds, uint8_t/uint16_t/uint32_t come from the local project headers. */

#define GP_MATRIX_READ_LE16(src_bytes) \
    ((uint16_t)((src_bytes)[0]) | ((uint16_t)((src_bytes)[1]) << 8))

#define GP_MATRIX_WRITE_LE16(dest_bytes, value) \
    do { \
        (dest_bytes)[0] = (uint8_t)((value) & 0xFFU); \
        (dest_bytes)[1] = (uint8_t)(((value) >> 8) & 0xFFU); \
    } while (0)

#define GP_MATRIX_PROTOCOL_MAGIC 0x47U

/* Compact 6-byte header: magic + flags + sequence + command + payload_length + header_crc8. */
#define GP_MATRIX_PACKET_HEADER_SIZE 6U
#define GP_MATRIX_PACKET_HEADER_CRC_BYTES (GP_MATRIX_PACKET_HEADER_SIZE - 1U)
#define GP_MATRIX_PROTOCOL_FLAG_IS_REPLY 0x04U

/* Reserved endpoint identifier kept for local link diagnostics on the active transport. */
#define GP_MATRIX_TRANSPORT_ENDPOINT_ID 0x31U

#define GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED 0x01U
#define GP_MATRIX_PROTOCOL_FLAG_LOCAL_ONLY 0x02U

#define GP_MATRIX_ACTION_FLAG_USE_SECONDARY 0x01U
#define GP_MATRIX_ACTION_FLAG_REMOTE_ENABLE 0x02U
#define GP_MATRIX_ACTION_FLAG_REMOTE_RELEASE 0x04U
#define GP_MATRIX_ACTION_FLAG_USE_UPLOADED_GLYPHS 0x08U

#define GP_MATRIX_APPLY_FLAG_PATTERN 0x01U
#define GP_MATRIX_APPLY_FLAG_GLYPH 0x02U

#define GP_MATRIX_PAYLOAD_FORMAT_RGB332 0x01U
#define GP_MATRIX_PAYLOAD_FORMAT_GLYPH_ROWS 0x02U
#define GP_MATRIX_PAYLOAD_FORMAT_BITMAP_RGB888 0x03U
#define GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED 0x04U

/* Animation batches keep compact bitmap frames indexed inside one explicit start/end window. */
#define GP_MATRIX_ANIMATION_MAX_FRAMES 32U
#define GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS 42U
#define GP_MATRIX_ANIMATION_INTERVAL_MS_MIN 1U
#define GP_MATRIX_ANIMATION_INTERVAL_MS_MAX 65535U
#define GP_MATRIX_ANIMATION_FLAG_LOOP 0x01U

#define GP_MATRIX_WIDTH 16U
#define GP_MATRIX_HEIGHT 16U
#define GP_MATRIX_FRAME_PIXELS (GP_MATRIX_WIDTH * GP_MATRIX_HEIGHT)
#define GP_MATRIX_RGB332_FRAME_SIZE GP_MATRIX_FRAME_PIXELS
#define GP_MATRIX_GLYPH_ROWS_SIZE (GP_MATRIX_HEIGHT * 2U)
#define GP_MATRIX_BITMAP_ROWS_BYTES (GP_MATRIX_HEIGHT * 2U)
#define GP_MATRIX_BITMAP_COLOR_BYTES 6U
#define GP_MATRIX_BITMAP_RGB888_FRAME_SIZE (GP_MATRIX_BITMAP_ROWS_BYTES + GP_MATRIX_BITMAP_COLOR_BYTES)

/* Multi-layer bitmap format: each layer = 1-byte header (seq/total) + 32-byte bitmap + 3-byte RGB = 36 bytes.
   An image is the concatenation of N layers. Seq[3:0] is the 0-based layer index, total[7:4] is the layer count. */
#define GP_MATRIX_BITMAP_LAYER_HEADER_BYTES 1U
#define GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES (GP_MATRIX_HEIGHT * 2U)
#define GP_MATRIX_BITMAP_LAYER_COLOR_BYTES 3U
#define GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES \
    (GP_MATRIX_BITMAP_LAYER_HEADER_BYTES + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES + GP_MATRIX_BITMAP_LAYER_COLOR_BYTES)
#define GP_MATRIX_BITMAP_LAYERED_MAX_COLORS 16U
#define GP_MATRIX_BITMAP_LAYERED_MAX_FRAME_SIZE (GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES * GP_MATRIX_BITMAP_LAYERED_MAX_COLORS)
#define GP_MATRIX_ANIMATION_MAX_LAYERS 4U
#define GP_MATRIX_ANIMATION_LAYERED_MAX_FRAME_SIZE \
    (GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES * GP_MATRIX_ANIMATION_MAX_LAYERS)

#define GP_MATRIX_MAX_CHUNK_DATA 160U
/* Max layered frame bytes that fit in one LayeredFrame command packet (4 layers). */
#define GP_MATRIX_LAYERED_FRAME_MAX_PAYLOAD (GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES * GP_MATRIX_ANIMATION_MAX_LAYERS)
#define GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES 256U
#define GP_MATRIX_PACKET_TRAILER_SIZE 2U
#define GP_MATRIX_PACKET_OVERHEAD_SIZE (GP_MATRIX_PACKET_HEADER_SIZE + GP_MATRIX_PACKET_TRAILER_SIZE)
#define GP_MATRIX_FRAME_START_PAYLOAD_BYTES 5U
#define GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES 3U
#define GP_MATRIX_ANIMATION_START_PAYLOAD_BYTES 5U
#define GP_MATRIX_ANIMATION_FRAME_PREFIX_BYTES 1U
#define GP_MATRIX_ANIMATION_END_PAYLOAD_BYTES 1U
#define GP_MATRIX_SCROLL_START_PAYLOAD_BYTES 5U
#define GP_MATRIX_ACTION_PAYLOAD_BYTES 28U
#define GP_MATRIX_TIME_SYNC_PAYLOAD_BYTES 6U
#define GP_MATRIX_REPLY_PAYLOAD_BYTES 3U
#define GP_MATRIX_DEBUG_LED_PAYLOAD_BYTES 1U
#define GP_MATRIX_DEBUG_LED_FLOW_PAYLOAD_BYTES 1U
#define GP_MATRIX_PACKET_MAX_PAYLOAD_BYTES (GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES + GP_MATRIX_MAX_CHUNK_DATA)
#define GP_MATRIX_PACKET_MAX_SIZE (GP_MATRIX_PACKET_OVERHEAD_SIZE + GP_MATRIX_PACKET_MAX_PAYLOAD_BYTES)
#define GP_MATRIX_DEBUG_LED_CLEAR 0xFFU
#define GP_MATRIX_DEBUG_LED_MAX_INDEX 7U
#define GP_MATRIX_DEBUG_LED_FLOW_STOP 0x00U
#define GP_MATRIX_DEBUG_LED_FLOW_START 0x01U

typedef enum GpMatrixCommand {
    kGpMatrixCommandPing = 0x01,
    kGpMatrixCommandSetBrightness = 0x02,
    kGpMatrixCommandSetMode = 0x03,
    kGpMatrixCommandStateHint = 0x04,
    kGpMatrixCommandSetAction = 0x05,
    kGpMatrixCommandSetDebugLed = 0x06,
    kGpMatrixCommandSetDebugLedFlow = 0x07,
    kGpMatrixCommandSetTime = 0x08,
    kGpMatrixCommandRequestCachedBitmap = 0x09,
    kGpMatrixCommandFrameStart = 0x10,
    kGpMatrixCommandFrameChunk = 0x11,
    kGpMatrixCommandFrameCommit = 0x12,
    kGpMatrixCommandAnimationStart = 0x13,
    kGpMatrixCommandAnimationFrame = 0x14,
    kGpMatrixCommandAnimationEnd = 0x15,
    /* Lightweight single-packet layered frame: payload = raw BITMAP_LAYERED bytes.
       Implicit: format=0x04, 16x16, mode=SolidFrame. No FrameStart/Chunk/Commit needed. */
    kGpMatrixCommandLayeredFrame = 0x18,
    /* Lightweight animation frame: payload = [frame_index:1][layered_data:N].
       Eliminates separate AnimationStart overhead for simple looping animations. */
    kGpMatrixCommandLayeredAnimFrame = 0x19,
    kGpMatrixCommandScrollGlyphStart = 0x20,
    kGpMatrixCommandScrollGlyphChunk = 0x21,
    kGpMatrixCommandScrollGlyphCommit = 0x22,
    kGpMatrixCommandHeartbeat = 0x30,
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
    kGpMatrixEffectRowReveal = 0x09,
    kGpMatrixEffectRowHide = 0x0A,
    kGpMatrixEffectGradientReveal = 0x0B,
} GpMatrixEffect;

typedef enum GpMatrixTimelinePath {
    kGpMatrixTimelinePathLinear = 0x00,
    kGpMatrixTimelinePathEaseInOut = 0x01,
    kGpMatrixTimelinePathBreathCurve = 0x02,
} GpMatrixTimelinePath;

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

typedef enum GpMatrixReplyDetail {
    kGpMatrixReplyDetailNone = 0x00,
    kGpMatrixReplyDetailHeaderCrc = 0x01,
    kGpMatrixReplyDetailPacketCrc = 0x02,
    kGpMatrixReplyDetailHeaderSize = 0x03,
    kGpMatrixReplyDetailPacketType = 0x04,
    kGpMatrixReplyDetailPayloadLength = 0x05,
    kGpMatrixReplyDetailChunkOffset = 0x06,
    kGpMatrixReplyDetailChunkSize = 0x07,
} GpMatrixReplyDetail;

/* Compact 6-byte header. flags: [reserved:5][is_reply:1][local_only:1][ack_req:1] */
typedef struct GpMatrixPacketHeader {
    uint8_t magic;
    uint8_t flags;
    uint8_t sequence;
    uint8_t command;
    uint8_t payload_length;  /* 0..255 */
    uint8_t header_crc8;     /* CRC8 of bytes 0..4 */
} GpMatrixPacketHeader;

typedef struct GpMatrixFrameStartPayload {
    uint8_t format;
    uint8_t width;
    uint8_t height;
    uint8_t total_bytes_lo;
    uint8_t total_bytes_hi;
} GpMatrixFrameStartPayload;

typedef struct GpMatrixFrameChunkPrefix {
    uint8_t byte_offset_lo;
    uint8_t byte_offset_hi;
    uint8_t size;
} GpMatrixFrameChunkPrefix;

typedef struct GpMatrixAnimationStartPayload {
    uint8_t format;
    uint8_t frame_count;
    uint8_t frame_interval_ms_lo;
    uint8_t frame_interval_ms_hi;
    uint8_t flags;
} GpMatrixAnimationStartPayload;

typedef struct GpMatrixAnimationFramePrefix {
    uint8_t frame_index;
} GpMatrixAnimationFramePrefix;

typedef struct GpMatrixAnimationEndPayload {
    uint8_t frame_count;
} GpMatrixAnimationEndPayload;

typedef struct GpMatrixScrollStartPayload {
    uint8_t glyph_count;
    uint8_t glyph_width;
    uint8_t glyph_spacing;
    uint8_t total_bytes_lo;
    uint8_t total_bytes_hi;
} GpMatrixScrollStartPayload;

/* Replies use the source command in the header and keep the payload compact. */
typedef struct GpMatrixReplyPayload {
    uint8_t status;
    uint8_t detail;
    uint8_t current_mode;
} GpMatrixReplyPayload;

typedef struct GpMatrixTimeSyncPayload {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} GpMatrixTimeSyncPayload;

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
    uint8_t frame_interval_ms_lo;
    uint8_t frame_interval_ms_hi;
    uint8_t timeline_duration_ms_lo;
    uint8_t timeline_duration_ms_hi;
    uint8_t timeline_repeat_delay_ms_lo;
    uint8_t timeline_repeat_delay_ms_hi;
    uint8_t timeline_repeat_count;
    uint8_t timeline_path;
    uint8_t animation_flags;
    uint8_t apply_flags;
} GpMatrixActionPayload;

#ifdef __cplusplus
static inline uint8_t GpMatrixComputeHeaderCrc8(const uint8_t* packet_bytes, uint16_t byte_count) {
    uint8_t crc = 0U;
    uint16_t index = 0U;
    uint8_t bit_index = 0U;

    for (index = 0U; index < byte_count; ++index) {
        crc ^= packet_bytes[index];
        for (bit_index = 0U; bit_index < 8U; ++bit_index) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1U) ^ 0x07U);
            } else {
                crc <<= 1U;
            }
        }
    }
    return crc;
}

static inline uint16_t GpMatrixComputePacketCrc16(const uint8_t* packet_bytes, uint16_t byte_count) {
    uint16_t crc = 0xFFFFU;
    uint16_t index = 0U;
    uint8_t bit_index = 0U;

    for (index = 0U; index < byte_count; ++index) {
        crc ^= packet_bytes[index];
        for (bit_index = 0U; bit_index < 8U; ++bit_index) {
            if ((crc & 0x0001U) != 0U) {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            } else {
                crc >>= 1U;
            }
        }
    }
    return crc;
}
#else
uint8_t GpMatrixComputeHeaderCrc8(const uint8_t *packetBytes, uint16_t byteCount);
uint16_t GpMatrixComputePacketCrc16(const uint8_t *packetBytes, uint16_t byteCount);
#endif

#endif
