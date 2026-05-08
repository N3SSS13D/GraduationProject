#include "config.h"
#include "gp_led_action.h"
#include "gp_led_matrix_ai8051u.h"
#include "gp_led_matrix_bt_debug.h"
#include "port.h"

#define GP_MATRIX_PROTOCOL_MIN_PACKET_LENGTH GP_MATRIX_PACKET_OVERHEAD_SIZE
#define GP_MATRIX_UART2_STREAM_IDLE_RESET_TICKS 100U

/* UART2 receive data is captured in the ISR and assembled into protocol packets in the main loop. */
static GpLedMatrixAi8051uContext xdata *g_gpMatrixCtx = 0;
static const uint8_t xdata *g_gpMatrixPayload = 0;
static uint16_t xdata g_gpMatrixPayloadLength = 0U;
static uint16_t xdata g_gpMatrixLoopIndex = 0U;
static uint16_t xdata g_gpMatrixChunkOffset = 0U;
static uint16_t xdata g_gpMatrixTotalBytes = 0U;
static uint8_t xdata g_gpMatrixSequence = 0U;
static uint8_t xdata g_gpMatrixCommand = 0U;
static uint16_t xdata g_gpMatrixPacketLength = 0U;
static uint8_t xdata g_gpMatrixChunkSize = 0U;
static uint8_t xdata g_gpMatrixCopyLength = 0U;
static uint8_t xdata g_gpMatrixFlags = 0U;
static uint8_t xdata g_gpMatrixReplyPayload[GP_MATRIX_REPLY_PAYLOAD_BYTES];
static uint8_t xdata g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailNone;
static GpMatrixActionPayload xdata g_gpMatrixAction;
static uint16_t xdata g_gpMatrixStreamLength = 0U;
static uint16_t xdata g_gpMatrixExpectedPacketLength = 0U;
static uint8_t xdata g_gpMatrixStreamIdleTicks = 0U;

uint8_t GpMatrixComputeHeaderCrc8(const uint8_t *packetBytes, uint16_t byteCount)
{
    uint8_t crc;
    uint16_t index;
    uint8_t bitIndex;

    crc = 0U;
    for (index = 0U; index < byteCount; ++index)
    {
        crc ^= packetBytes[index];
        for (bitIndex = 0U; bitIndex < 8U; ++bitIndex)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1U) ^ 0x07U);
            }
            else
            {
                crc <<= 1U;
            }
        }
    }

    return crc;
}

uint16_t GpMatrixComputePacketCrc16(const uint8_t *packetBytes, uint16_t byteCount)
{
    uint16_t crc;
    uint16_t index;
    uint8_t bitIndex;

    crc = 0xFFFFU;
    for (index = 0U; index < byteCount; ++index)
    {
        crc ^= packetBytes[index];
        for (bitIndex = 0U; bitIndex < 8U; ++bitIndex)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

static void GpLedMatrixAi8051u_CopyBytes(uint8_t xdata *dest, const uint8_t xdata *src, uint16_t length)
{
    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < length; ++g_gpMatrixLoopIndex)
    {
        dest[g_gpMatrixLoopIndex] = src[g_gpMatrixLoopIndex];
    }
}

static uint16_t GpLedMatrixAi8051u_ReadLe16(const uint8_t *srcBytes)
{
    return GP_MATRIX_READ_LE16(srcBytes);
}

static void GpLedMatrixAi8051u_WriteLe16(uint8_t xdata *destBytes, uint16_t value)
{
    GP_MATRIX_WRITE_LE16(destBytes, value);
}

static void GpLedMatrixAi8051u_ResetFrameTransfer(GpLedMatrixAi8051uContext xdata *context)
{
    context->expectedBytes = 0U;
    context->receivedBytes = 0U;
    context->frameFormat = 0U;
}

static void GpLedMatrixAi8051u_ResetGlyphTransfer(GpLedMatrixAi8051uContext xdata *context)
{
    context->glyphExpectedBytes = 0U;
    context->glyphReceivedBytes = 0U;
    context->glyphCount = 0U;
    context->glyphWidth = 0U;
    context->glyphSpacing = 0U;
}

static const char *GpLedMatrixAi8051u_CommandName(uint8_t command)
{
    switch (command)
    {
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

static void GpLedMatrixAi8051u_LogPacketRx(uint8_t command, uint8_t sequence, uint16_t payloadLength)
{
    printf("[GP_RX] cmd=%s seq=%u len=%u\r\n",
           GpLedMatrixAi8051u_CommandName(command),
           (unsigned int)sequence,
           (unsigned int)payloadLength);
}

static void GpLedMatrixAi8051u_LogPacketDrop(uint8_t command, uint8_t sequence, const char *reason)
{
    printf("[GP_DROP] cmd=%s seq=%u reason=%s\r\n",
           GpLedMatrixAi8051u_CommandName(command),
           (unsigned int)sequence,
           reason);
}

static void GpLedMatrixAi8051u_LogReply(uint8_t sequence,
                                        GpMatrixStatusCode status,
                         uint8_t sourceCommand,
                         uint8_t detail)
{
    printf("[GP_TX] cmd=reply seq=%u for=%s st=%u detail=%u\r\n",
           (unsigned int)sequence,
           GpLedMatrixAi8051u_CommandName(sourceCommand),
        (unsigned int)status,
        (unsigned int)detail);
}

static void GpLedMatrixAi8051u_ResetPacketAssembly(void)
{
    g_gpMatrixStreamLength = 0U;
    g_gpMatrixExpectedPacketLength = 0U;
    g_gpMatrixStreamIdleTicks = 0U;
}

static void GpLedMatrixAi8051u_ResetContext(GpLedMatrixAi8051uContext xdata *context, uint8_t transportAddress)
{
    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < GP_MATRIX_AI8051U_FRAME_BUFFER_SIZE; ++g_gpMatrixLoopIndex)
    {
        context->frameBuffer[g_gpMatrixLoopIndex] = 0U;
    }

    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES; ++g_gpMatrixLoopIndex)
    {
        context->glyphBuffer[g_gpMatrixLoopIndex] = 0U;
    }

    context->transportEndpoint = transportAddress;
    context->brightness = 200U;
    context->mode = (uint8_t)kGpMatrixModeSolidFrame;
    context->lastSequence = 0U;
    context->lastCommand = 0U;
    context->lastFlags = 0U;
    context->lastStatus = (uint8_t)kGpMatrixStatusOk;
    context->packetLength = 0U;
    context->packetPending = 0U;
    context->packetReplyPrepared = 0U;
    context->txLength = 0U;
    context->txPending = 0U;
    GpLedMatrixAi8051u_ResetFrameTransfer(context);
    GpLedMatrixAi8051u_ResetGlyphTransfer(context);
}

static uint8_t GpLedMatrixAi8051u_ShouldReply(void)
{
    if ((g_gpMatrixFlags & GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED) != 0U)
    {
        return 1U;
    }

    return 0U;
}

static void GpLedMatrixAi8051u_ShutdownLegacyI2cHardware(void)
{
    DMA_I2C_DisableDMA();
    DMA_I2C_DisableTx();
    DMA_I2C_DisableRx();
    DMA_I2C_DisableTxInt();
    DMA_I2C_DisableRxInt();
    DMA_I2C_DisableACKErrorInt();
    DMA_I2C_ClearTxFlag();
    DMA_I2C_ClearRxFlag();
    DMA_I2C_ClearOverWriteFlag();
    DMA_I2C_ClearRxLossFlag();
    DMA_I2C_ClearFIFO();

    I2C_DisableSlaveAllInt();
    I2C_DisableTimeoutInt();
    I2C_DisableTimeout();
    I2C_ClearSlaveAllFlag();
    I2C_ClearTimeoutFlag();
    I2C_Disable();
}

static void GpLedMatrixAi8051u_QueueCompletePacket(GpLedMatrixAi8051uContext xdata *context)
{
    if (context->packetPending != 0U)
    {
        context->lastStatus = (uint8_t)kGpMatrixStatusBusy;
        GpLedMatrixAi8051u_ResetPacketAssembly();
        return;
    }

    context->packetLength = g_gpMatrixExpectedPacketLength;
    context->packetPending = 1U;
    context->packetReplyPrepared = 0U;
    GpLedMatrixAi8051u_ResetPacketAssembly();
}

static void GpLedMatrixAi8051u_ResyncFromByte(GpLedMatrixAi8051uContext xdata *context, uint8_t rxByte)
{
    if (rxByte == GP_MATRIX_PROTOCOL_MAGIC0)
    {
        context->rxBuffer[0] = rxByte;
        g_gpMatrixStreamLength = 1U;
        g_gpMatrixExpectedPacketLength = 0U;
        return;
    }

    GpLedMatrixAi8051u_ResetPacketAssembly();
}

static void GpLedMatrixAi8051u_PushStreamByte(GpLedMatrixAi8051uContext xdata *context, uint8_t rxByte)
{
    const GpMatrixPacketHeader xdata *headerV2;
    const GpMatrixPacketHeaderV3 xdata *headerV3;
    uint16_t payloadLength;
    uint16_t expectedLength;
    uint8_t detectedV3;

    if (g_gpMatrixStreamLength == 0U)
    {
        if (rxByte == GP_MATRIX_PROTOCOL_MAGIC)
        {
            context->rxBuffer[0] = rxByte;
            g_gpMatrixStreamLength = 1U;
        }
        return;
    }

    if (g_gpMatrixStreamLength >= GP_MATRIX_AI8051U_RX_BUFFER_SIZE)
    {
        GpLedMatrixAi8051u_ResyncFromByte(context, rxByte);
        return;
    }

    /* Byte 1 determines format: 0x50 = V2 (12-byte header), otherwise = V3 compact (6-byte header). */
    if (g_gpMatrixStreamLength == 1U)
    {
        if ((rxByte != GP_MATRIX_PROTOCOL_MAGIC1) && ((rxByte & 0x80U) != 0U))
        {
            /* Not V2 and not valid V3 flags (bit7 reserved=0): resync. */
            GpLedMatrixAi8051u_ResyncFromByte(context, rxByte);
            return;
        }
    }

    context->rxBuffer[g_gpMatrixStreamLength] = rxByte;
    g_gpMatrixStreamLength++;

    detectedV3 = 0U;
    if ((g_gpMatrixStreamLength == GP_MATRIX_PACKET_HEADER_SIZE_V3) && (g_gpMatrixExpectedPacketLength == 0U))
    {
        if (context->rxBuffer[1U] != GP_MATRIX_PROTOCOL_MAGIC1)
        {
            /* Potential V3 packet: validate CRC8 immediately. Reject on mismatch. */
            headerV3 = (const GpMatrixPacketHeaderV3 xdata *)context->rxBuffer;
            if ((GpMatrixComputeHeaderCrc8(context->rxBuffer, GP_MATRIX_PACKET_HEADER_CRC_BYTES_V3)
                 == headerV3->header_crc8)
                && (headerV3->payload_length <= GP_MATRIX_PACKET_MAX_PAYLOAD_BYTES))
            {
                expectedLength = (uint16_t)(GP_MATRIX_PACKET_HEADER_SIZE_V3
                                            + headerV3->payload_length
                                            + GP_MATRIX_PACKET_TRAILER_SIZE);
                if ((expectedLength >= GP_MATRIX_PROTOCOL_MIN_PACKET_LENGTH)
                    && (expectedLength <= GP_MATRIX_AI8051U_RX_BUFFER_SIZE))
                {
                    g_gpMatrixExpectedPacketLength = expectedLength;
                    detectedV3 = 1U;
                }
            }
            if (detectedV3 == 0U)
            {
                /* V3 header CRC failed: resync immediately. */
                GpLedMatrixAi8051u_ResyncFromByte(context, rxByte);
                return;
            }
        }
    }

    if ((detectedV3 == 0U) && (g_gpMatrixStreamLength == GP_MATRIX_PACKET_HEADER_SIZE)
        && (g_gpMatrixExpectedPacketLength == 0U))
    {
        if (context->rxBuffer[1U] == GP_MATRIX_PROTOCOL_MAGIC1)
        {
            headerV2 = (const GpMatrixPacketHeader xdata *)context->rxBuffer;
            if ((headerV2->version == GP_MATRIX_PROTOCOL_VERSION)
                && (headerV2->header_size == GP_MATRIX_PACKET_HEADER_SIZE)
                && (headerV2->packet_type == (uint8_t)kGpMatrixPacketTypeRequest)
                && (GpMatrixComputeHeaderCrc8(context->rxBuffer, GP_MATRIX_PACKET_HEADER_CRC_BYTES)
                    == headerV2->header_crc8))
            {
                payloadLength = GpLedMatrixAi8051u_ReadLe16(&headerV2->payload_length_lo);
                expectedLength = (uint16_t)(headerV2->header_size + payloadLength
                                            + GP_MATRIX_PACKET_TRAILER_SIZE);
                if ((expectedLength >= GP_MATRIX_PROTOCOL_MIN_PACKET_LENGTH)
                    && (expectedLength <= GP_MATRIX_AI8051U_RX_BUFFER_SIZE))
                {
                    g_gpMatrixExpectedPacketLength = expectedLength;
                }
            }
            if (g_gpMatrixExpectedPacketLength == 0U)
            {
                /* V2 header validation failed: resync immediately. */
                GpLedMatrixAi8051u_ResyncFromByte(context, rxByte);
                return;
            }
        }
        else
        {
            /* rxBuffer[1] != 0x50 and V3 was already rejected at length 6: resync. */
            GpLedMatrixAi8051u_ResyncFromByte(context, rxByte);
            return;
        }
    }

    if ((g_gpMatrixExpectedPacketLength != 0U) && (g_gpMatrixStreamLength == g_gpMatrixExpectedPacketLength))
    {
        GpLedMatrixAi8051u_QueueCompletePacket(context);
    }
}

static void GpLedMatrixAi8051u_BuildReply(GpMatrixStatusCode status)
{
    GpMatrixPacketHeader xdata *header;
    uint16_t packetCrc;

    if (g_gpMatrixCtx == 0)
    {
        return;
    }

    g_gpMatrixReplyPayload[0] = (uint8_t)status;
    g_gpMatrixReplyPayload[1] = g_gpMatrixReplyDetail;
    g_gpMatrixReplyPayload[2] = g_gpMatrixCtx->mode;

    g_gpMatrixPacketLength = (uint16_t)(GP_MATRIX_PACKET_HEADER_SIZE + GP_MATRIX_REPLY_PAYLOAD_BYTES
                                        + GP_MATRIX_PACKET_TRAILER_SIZE);
    if (g_gpMatrixPacketLength > GP_MATRIX_AI8051U_TX_BUFFER_SIZE)
    {
        return;
    }

    header = (GpMatrixPacketHeader xdata *)g_gpMatrixCtx->txBuffer;
    header->magic0 = GP_MATRIX_PROTOCOL_MAGIC0;
    header->magic1 = GP_MATRIX_PROTOCOL_MAGIC1;
    header->version = GP_MATRIX_PROTOCOL_VERSION;
    header->header_size = GP_MATRIX_PACKET_HEADER_SIZE;
    header->packet_type = (uint8_t)kGpMatrixPacketTypeReply;
    header->flags = 0U;
    header->sequence = g_gpMatrixSequence;
    header->reply_to_sequence = g_gpMatrixSequence;
    GpLedMatrixAi8051u_WriteLe16(&header->payload_length_lo, GP_MATRIX_REPLY_PAYLOAD_BYTES);
    header->command = g_gpMatrixCommand;
    header->header_crc8 = GpMatrixComputeHeaderCrc8(g_gpMatrixCtx->txBuffer, GP_MATRIX_PACKET_HEADER_CRC_BYTES);
    GpLedMatrixAi8051u_CopyBytes(&g_gpMatrixCtx->txBuffer[GP_MATRIX_PACKET_HEADER_SIZE],
                                 g_gpMatrixReplyPayload,
                                 GP_MATRIX_REPLY_PAYLOAD_BYTES);
    packetCrc = GpMatrixComputePacketCrc16(g_gpMatrixCtx->txBuffer,
                                           (uint16_t)(g_gpMatrixPacketLength - GP_MATRIX_PACKET_TRAILER_SIZE));
    GpLedMatrixAi8051u_WriteLe16(&g_gpMatrixCtx->txBuffer[g_gpMatrixPacketLength - GP_MATRIX_PACKET_TRAILER_SIZE],
                                 packetCrc);
    g_gpMatrixCtx->txLength = g_gpMatrixPacketLength;
    g_gpMatrixCtx->txPending = 1U;
    g_gpMatrixCtx->packetReplyPrepared = 1U;
    g_gpMatrixCtx->lastStatus = (uint8_t)status;
    GpLedMatrixAi8051u_LogReply(g_gpMatrixSequence, status, g_gpMatrixCommand, g_gpMatrixReplyDetail);
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleSetBrightness(void)
{
    if (g_gpMatrixPayloadLength != 1U)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpMatrixCtx->brightness = g_gpMatrixPayload[0];
    GpLedAction_SetBrightness(g_gpMatrixCtx->brightness);
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleSetMode(void)
{
    if (g_gpMatrixPayloadLength != 1U)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpMatrixCtx->mode = g_gpMatrixPayload[0];
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleSetAction(void)
{
    if (g_gpMatrixPayloadLength != GP_MATRIX_ACTION_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpMatrixAction.source = g_gpMatrixPayload[0];
    g_gpMatrixAction.content = g_gpMatrixPayload[1];
    g_gpMatrixAction.effect = g_gpMatrixPayload[2];
    g_gpMatrixAction.direction = g_gpMatrixPayload[3];
    g_gpMatrixAction.color_mode = g_gpMatrixPayload[4];
    g_gpMatrixAction.brightness = g_gpMatrixPayload[5];
    g_gpMatrixAction.primary_r = g_gpMatrixPayload[6];
    g_gpMatrixAction.primary_g = g_gpMatrixPayload[7];
    g_gpMatrixAction.primary_b = g_gpMatrixPayload[8];
    g_gpMatrixAction.secondary_r = g_gpMatrixPayload[9];
    g_gpMatrixAction.secondary_g = g_gpMatrixPayload[10];
    g_gpMatrixAction.secondary_b = g_gpMatrixPayload[11];
    g_gpMatrixAction.pattern_id = g_gpMatrixPayload[12];
    g_gpMatrixAction.glyph_id = g_gpMatrixPayload[13];
    g_gpMatrixAction.scroll_step = g_gpMatrixPayload[14];
    g_gpMatrixAction.anim_step = g_gpMatrixPayload[15];
    g_gpMatrixAction.gradient_span = g_gpMatrixPayload[16];
    g_gpMatrixAction.flags = g_gpMatrixPayload[17];
    return GpLedAction_ApplyAction(&g_gpMatrixAction);
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleSetDebugLed(void)
{
    if (g_gpMatrixPayloadLength != GP_MATRIX_DEBUG_LED_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }

    if (g_gpMatrixPayload[0] == GP_MATRIX_DEBUG_LED_CLEAR)
    {
        (void)GpLedAction_SetDebugLedFlow(0U);
        PORT2_ClearDebugLeds();
        return kGpMatrixStatusOk;
    }

    if (g_gpMatrixPayload[0] > GP_MATRIX_DEBUG_LED_MAX_INDEX)
    {
        return kGpMatrixStatusUnsupported;
    }

    (void)GpLedAction_SetDebugLedFlow(0U);
    PORT2_SetDebugLedDigit(g_gpMatrixPayload[0]);
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleSetDebugLedFlow(void)
{
    if (g_gpMatrixPayloadLength != GP_MATRIX_DEBUG_LED_FLOW_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }

    if (g_gpMatrixPayload[0] == GP_MATRIX_DEBUG_LED_FLOW_START)
    {
        return GpLedAction_SetDebugLedFlow(1U);
    }
    if (g_gpMatrixPayload[0] == GP_MATRIX_DEBUG_LED_FLOW_STOP)
    {
        return GpLedAction_SetDebugLedFlow(0U);
    }

    return kGpMatrixStatusUnsupported;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleFrameStart(void)
{
    uint8_t frameFormat;

    if (g_gpMatrixPayloadLength != GP_MATRIX_FRAME_START_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }
    if ((g_gpMatrixPayload[1] != GP_MATRIX_WIDTH)
        || (g_gpMatrixPayload[2] != GP_MATRIX_HEIGHT))
    {
        return kGpMatrixStatusUnsupported;
    }

    frameFormat = g_gpMatrixPayload[0];
    g_gpMatrixTotalBytes = GpLedMatrixAi8051u_ReadLe16(&g_gpMatrixPayload[3]);
    if (frameFormat == GP_MATRIX_PAYLOAD_FORMAT_RGB332)
    {
        if (g_gpMatrixTotalBytes > GP_MATRIX_RGB332_FRAME_SIZE)
        {
            return kGpMatrixStatusBadLength;
        }
    }
    else if (frameFormat == GP_MATRIX_PAYLOAD_FORMAT_BITMAP_RGB888)
    {
        if (g_gpMatrixTotalBytes != GP_MATRIX_BITMAP_RGB888_FRAME_SIZE)
        {
            return kGpMatrixStatusBadLength;
        }
    }
    else if (frameFormat == GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED)
    {
        if ((g_gpMatrixTotalBytes < GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)
            || (g_gpMatrixTotalBytes > GP_MATRIX_AI8051U_FRAME_BUFFER_SIZE)
            || ((g_gpMatrixTotalBytes % GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES) != 0U))
        {
            return kGpMatrixStatusBadLength;
        }
    }
    else
    {
        return kGpMatrixStatusUnsupported;
    }

    /* Compact bitmap uploads reuse the same staged frame buffer as full RGB332 frames. */
    g_gpMatrixCtx->frameFormat = frameFormat;
    g_gpMatrixCtx->expectedBytes = g_gpMatrixTotalBytes;
    g_gpMatrixCtx->receivedBytes = 0U;
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleFrameChunk(void)
{
    if (g_gpMatrixPayloadLength < GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailPayloadLength;
        return kGpMatrixStatusBadLength;
    }
    if (g_gpMatrixCtx->expectedBytes == 0U)
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailChunkOffset;
        return kGpMatrixStatusBadSequence;
    }

    g_gpMatrixChunkOffset = GpLedMatrixAi8051u_ReadLe16(g_gpMatrixPayload);
    g_gpMatrixChunkSize = g_gpMatrixPayload[2];
    if ((g_gpMatrixPayloadLength != (uint16_t)g_gpMatrixChunkSize + GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
        || (g_gpMatrixChunkOffset != g_gpMatrixCtx->receivedBytes)
        || ((uint16_t)g_gpMatrixChunkOffset + g_gpMatrixChunkSize > g_gpMatrixCtx->expectedBytes))
    {
        g_gpMatrixReplyDetail = (g_gpMatrixPayloadLength != (uint16_t)g_gpMatrixChunkSize + GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
            ? (uint8_t)kGpMatrixReplyDetailChunkSize
            : (uint8_t)kGpMatrixReplyDetailChunkOffset;
        return kGpMatrixStatusBadSequence;
    }

    GpLedMatrixAi8051u_CopyBytes(&g_gpMatrixCtx->frameBuffer[g_gpMatrixChunkOffset],
                                 &g_gpMatrixPayload[GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES],
                                 g_gpMatrixChunkSize);
    g_gpMatrixCtx->receivedBytes = (uint16_t)(g_gpMatrixCtx->receivedBytes + g_gpMatrixChunkSize);
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleFrameCommit(void)
{
    GpMatrixStatusCode status;
    GpMatrixMode mode;

    if ((g_gpMatrixCtx->expectedBytes == 0U) || (g_gpMatrixCtx->receivedBytes != g_gpMatrixCtx->expectedBytes))
    {
        return kGpMatrixStatusBadSequence;
    }

    mode = (GpMatrixMode)g_gpMatrixCtx->mode;
    if (g_gpMatrixPayloadLength == 1U)
    {
        mode = (GpMatrixMode)g_gpMatrixPayload[0];
        g_gpMatrixCtx->mode = g_gpMatrixPayload[0];
    }
    else if (g_gpMatrixPayloadLength != 0U)
    {
        return kGpMatrixStatusBadLength;
    }

    if (g_gpMatrixCtx->frameFormat == GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED)
    {
        status = GpLedAction_ApplyFrameBitmapLayered(g_gpMatrixCtx->frameBuffer,
                                                      g_gpMatrixCtx->receivedBytes,
                                                      mode);
    }
    else if (g_gpMatrixCtx->frameFormat == GP_MATRIX_PAYLOAD_FORMAT_BITMAP_RGB888)
    {
        status = GpLedAction_ApplyFrameBitmapRgb888(g_gpMatrixCtx->frameBuffer,
                                                    g_gpMatrixCtx->receivedBytes,
                                                    mode);
    }
    else
    {
        status = GpLedAction_ApplyFrameRgb332(g_gpMatrixCtx->frameBuffer, g_gpMatrixCtx->receivedBytes, mode);
    }
    GpLedMatrixAi8051u_ResetFrameTransfer(g_gpMatrixCtx);
    return status;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleGlyphStart(void)
{
    if (g_gpMatrixPayloadLength != GP_MATRIX_SCROLL_START_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpMatrixTotalBytes = GpLedMatrixAi8051u_ReadLe16(&g_gpMatrixPayload[3]);
    if (g_gpMatrixTotalBytes > GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpMatrixCtx->glyphCount = g_gpMatrixPayload[0];
    g_gpMatrixCtx->glyphWidth = g_gpMatrixPayload[1];
    g_gpMatrixCtx->glyphSpacing = g_gpMatrixPayload[2];
    g_gpMatrixCtx->glyphExpectedBytes = g_gpMatrixTotalBytes;
    g_gpMatrixCtx->glyphReceivedBytes = 0U;
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleGlyphChunk(void)
{
    if (g_gpMatrixPayloadLength < GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailPayloadLength;
        return kGpMatrixStatusBadLength;
    }
    if (g_gpMatrixCtx->glyphExpectedBytes == 0U)
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailChunkOffset;
        return kGpMatrixStatusBadSequence;
    }

    g_gpMatrixChunkOffset = GpLedMatrixAi8051u_ReadLe16(g_gpMatrixPayload);
    g_gpMatrixChunkSize = g_gpMatrixPayload[2];
    if ((g_gpMatrixPayloadLength != (uint16_t)g_gpMatrixChunkSize + GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
        || (g_gpMatrixChunkOffset != g_gpMatrixCtx->glyphReceivedBytes)
        || ((uint16_t)g_gpMatrixChunkOffset + g_gpMatrixChunkSize > g_gpMatrixCtx->glyphExpectedBytes))
    {
        g_gpMatrixReplyDetail = (g_gpMatrixPayloadLength != (uint16_t)g_gpMatrixChunkSize + GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
            ? (uint8_t)kGpMatrixReplyDetailChunkSize
            : (uint8_t)kGpMatrixReplyDetailChunkOffset;
        return kGpMatrixStatusBadSequence;
    }

    GpLedMatrixAi8051u_CopyBytes(&g_gpMatrixCtx->glyphBuffer[g_gpMatrixChunkOffset],
                                 &g_gpMatrixPayload[GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES],
                                 g_gpMatrixChunkSize);
    g_gpMatrixCtx->glyphReceivedBytes = (uint16_t)(g_gpMatrixCtx->glyphReceivedBytes + g_gpMatrixChunkSize);
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleGlyphCommit(void)
{
    GpMatrixStatusCode status;

    if (g_gpMatrixPayloadLength != 0U)
    {
        return kGpMatrixStatusBadLength;
    }
    if ((g_gpMatrixCtx->glyphExpectedBytes == 0U)
        || (g_gpMatrixCtx->glyphReceivedBytes != g_gpMatrixCtx->glyphExpectedBytes))
    {
        return kGpMatrixStatusBadSequence;
    }

    status = GpLedAction_ApplyGlyphRows(g_gpMatrixCtx->glyphBuffer,
                                        g_gpMatrixCtx->glyphReceivedBytes,
                                        g_gpMatrixCtx->glyphCount,
                                        g_gpMatrixCtx->glyphWidth,
                                        g_gpMatrixCtx->glyphSpacing);
    GpLedMatrixAi8051u_ResetGlyphTransfer(g_gpMatrixCtx);
    return status;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleAnimationStart(void)
{
    uint8_t animFormat;

    if (g_gpMatrixPayloadLength != GP_MATRIX_ANIMATION_START_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }

    animFormat = g_gpMatrixPayload[0];
    if ((animFormat != GP_MATRIX_PAYLOAD_FORMAT_BITMAP_RGB888)
        && (animFormat != GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED))
    {
        return kGpMatrixStatusUnsupported;
    }

    return GpLedAction_BeginAnimationUpload(animFormat,
                                            g_gpMatrixPayload[1],
                                            GpLedMatrixAi8051u_ReadLe16(&g_gpMatrixPayload[2]),
                                            g_gpMatrixPayload[4]);
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleAnimationFrame(void)
{
    uint16_t frameDataLen;

    if (g_gpMatrixPayloadLength < (GP_MATRIX_ANIMATION_FRAME_PREFIX_BYTES + GP_MATRIX_BITMAP_RGB888_FRAME_SIZE))
    {
        return kGpMatrixStatusBadLength;
    }

    frameDataLen = (uint16_t)(g_gpMatrixPayloadLength - GP_MATRIX_ANIMATION_FRAME_PREFIX_BYTES);
    return GpLedAction_StoreAnimationFrame(g_gpMatrixPayload[0],
                                           &g_gpMatrixPayload[GP_MATRIX_ANIMATION_FRAME_PREFIX_BYTES],
                                           frameDataLen);
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleAnimationEnd(void)
{
    if (g_gpMatrixPayloadLength != GP_MATRIX_ANIMATION_END_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }

    return GpLedAction_CommitAnimation(g_gpMatrixPayload[0]);
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleLayeredFrame(void)
{
    GpMatrixStatusCode status;

    if ((g_gpMatrixPayloadLength < GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)
        || (g_gpMatrixPayloadLength > GP_MATRIX_LAYERED_FRAME_MAX_PAYLOAD)
        || ((g_gpMatrixPayloadLength % GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES) != 0U))
    {
        return kGpMatrixStatusBadLength;
    }

    status = GpLedAction_ApplyFrameBitmapLayered(g_gpMatrixPayload,
                                                  g_gpMatrixPayloadLength,
                                                  kGpMatrixModeSolidFrame);
    GpLedAction_NotifyCommunicationActive();
    return status;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleLayeredAnimFrame(void)
{
    uint8_t frameIndex;

    if (g_gpMatrixPayloadLength < (1U + GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES))
    {
        return kGpMatrixStatusBadLength;
    }

    frameIndex = g_gpMatrixPayload[0];
    GpLedAction_NotifyCommunicationActive();
    return GpLedAction_StoreAnimationFrame(frameIndex,
                                            &g_gpMatrixPayload[1U],
                                            (uint16_t)(g_gpMatrixPayloadLength - 1U));
}

static GpMatrixStatusCode GpLedMatrixAi8051u_ProcessPacket(GpLedMatrixAi8051uContext xdata *context,
                                                           const uint8_t xdata *packet,
                                                           uint16_t packetLength)
{
    const GpMatrixPacketHeader xdata *headerV2;
    const GpMatrixPacketHeaderV3 xdata *headerV3;
    GpMatrixStatusCode status;
    uint8_t command;
    uint8_t sequence;
    uint8_t headerSize;
    uint16_t payloadLength;
    uint16_t expectedLength;
    uint16_t packetCrc;
    uint8_t headerCrcBytes;
    const uint8_t xdata *payload;

    if (packetLength < GP_MATRIX_PROTOCOL_MIN_PACKET_LENGTH)
    {
        return kGpMatrixStatusBadLength;
    }

    if (packet[0] != GP_MATRIX_PROTOCOL_MAGIC)
    {
        return kGpMatrixStatusUnsupported;
    }

    /* Detect format by byte 1: 0x50 = V2 (12-byte), else = V3 compact (6-byte). */
    if (packet[1] == GP_MATRIX_PROTOCOL_MAGIC1)
    {
        headerV2 = (const GpMatrixPacketHeader xdata *)packet;
        if ((headerV2->version != GP_MATRIX_PROTOCOL_VERSION)
            || (headerV2->header_size != GP_MATRIX_PACKET_HEADER_SIZE)
            || (headerV2->packet_type != (uint8_t)kGpMatrixPacketTypeRequest))
        {
            return kGpMatrixStatusUnsupported;
        }
        headerCrcBytes = GP_MATRIX_PACKET_HEADER_CRC_BYTES;
        if (GpMatrixComputeHeaderCrc8(packet, headerCrcBytes) != headerV2->header_crc8)
        {
            return kGpMatrixStatusBadChecksum;
        }
        command = headerV2->command;
        sequence = headerV2->sequence;
        g_gpMatrixFlags = headerV2->flags;
        payloadLength = GpLedMatrixAi8051u_ReadLe16(&headerV2->payload_length_lo);
        headerSize = headerV2->header_size;
    }
    else
    {
        headerV3 = (const GpMatrixPacketHeaderV3 xdata *)packet;
        headerCrcBytes = GP_MATRIX_PACKET_HEADER_CRC_BYTES_V3;
        if (GpMatrixComputeHeaderCrc8(packet, headerCrcBytes) != headerV3->header_crc8)
        {
            return kGpMatrixStatusBadChecksum;
        }
        if ((headerV3->flags & GP_MATRIX_PROTOCOL_FLAG_V3_IS_REPLY) != 0U)
        {
            return kGpMatrixStatusUnsupported;
        }
        command = headerV3->command;
        sequence = headerV3->sequence;
        g_gpMatrixFlags = (headerV3->flags & (GP_MATRIX_PROTOCOL_FLAG_ACK_REQUIRED
                                               | GP_MATRIX_PROTOCOL_FLAG_LOCAL_ONLY));
        payloadLength = headerV3->payload_length;
        headerSize = GP_MATRIX_PACKET_HEADER_SIZE_V3;
    }

    g_gpMatrixCtx = context;
    g_gpMatrixCommand = command;
    g_gpMatrixSequence = sequence;
    g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailNone;

    expectedLength = (uint16_t)(headerSize + payloadLength + GP_MATRIX_PACKET_TRAILER_SIZE);
    if (expectedLength != packetLength)
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailPayloadLength;
        GpLedMatrixAi8051u_LogPacketDrop(command, sequence, "length");
        return kGpMatrixStatusBadLength;
    }

    packetCrc = GpMatrixComputePacketCrc16(packet, (uint16_t)(packetLength - GP_MATRIX_PACKET_TRAILER_SIZE));
    if (packetCrc != GpLedMatrixAi8051u_ReadLe16(&packet[packetLength - GP_MATRIX_PACKET_TRAILER_SIZE]))
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailPacketCrc;
        GpLedMatrixAi8051u_LogPacketDrop(command, sequence, "packet_crc");
        return kGpMatrixStatusBadChecksum;
    }

    context->lastCommand = command;
    context->lastSequence = sequence;
    context->lastFlags = g_gpMatrixFlags;
    payload = &packet[headerSize];
    g_gpMatrixPayload = payload;
    g_gpMatrixPayloadLength = payloadLength;
    GpLedMatrixAi8051u_LogPacketRx(command, sequence, payloadLength);
    GpLedAction_NotifyCommunicationActive();
    status = kGpMatrixStatusUnsupported;

    switch (command)
    {
        case kGpMatrixCommandPing:
        case kGpMatrixCommandHeartbeat:
            status = kGpMatrixStatusOk;
            break;

        case kGpMatrixCommandSetBrightness:
            status = GpLedMatrixAi8051u_HandleSetBrightness();
            break;

        case kGpMatrixCommandSetMode:
            status = GpLedMatrixAi8051u_HandleSetMode();
            break;

        case kGpMatrixCommandSetAction:
            status = GpLedMatrixAi8051u_HandleSetAction();
            break;

        case kGpMatrixCommandSetDebugLed:
            status = GpLedMatrixAi8051u_HandleSetDebugLed();
            break;

        case kGpMatrixCommandSetDebugLedFlow:
            status = GpLedMatrixAi8051u_HandleSetDebugLedFlow();
            break;

        case kGpMatrixCommandFrameStart:
            status = GpLedMatrixAi8051u_HandleFrameStart();
            break;

        case kGpMatrixCommandFrameChunk:
            status = GpLedMatrixAi8051u_HandleFrameChunk();
            break;

        case kGpMatrixCommandFrameCommit:
            status = GpLedMatrixAi8051u_HandleFrameCommit();
            break;

        case kGpMatrixCommandAnimationStart:
            status = GpLedMatrixAi8051u_HandleAnimationStart();
            break;

        case kGpMatrixCommandAnimationFrame:
            status = GpLedMatrixAi8051u_HandleAnimationFrame();
            break;

        case kGpMatrixCommandAnimationEnd:
            status = GpLedMatrixAi8051u_HandleAnimationEnd();
            break;

        case kGpMatrixCommandLayeredFrame:
            status = GpLedMatrixAi8051u_HandleLayeredFrame();
            break;

        case kGpMatrixCommandLayeredAnimFrame:
            status = GpLedMatrixAi8051u_HandleLayeredAnimFrame();
            break;

        case kGpMatrixCommandScrollGlyphStart:
            status = GpLedMatrixAi8051u_HandleGlyphStart();
            break;

        case kGpMatrixCommandScrollGlyphChunk:
            status = GpLedMatrixAi8051u_HandleGlyphChunk();
            break;

        case kGpMatrixCommandScrollGlyphCommit:
            status = GpLedMatrixAi8051u_HandleGlyphCommit();
            break;

        default:
            status = kGpMatrixStatusUnsupported;
            break;
    }

    if ((status == kGpMatrixStatusBadLength) && (g_gpMatrixReplyDetail == (uint8_t)kGpMatrixReplyDetailNone))
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailPayloadLength;
    }
    else if ((status == kGpMatrixStatusBadSequence) && (g_gpMatrixReplyDetail == (uint8_t)kGpMatrixReplyDetailNone))
    {
        g_gpMatrixReplyDetail = (uint8_t)kGpMatrixReplyDetailChunkOffset;
    }

    if (GpLedMatrixAi8051u_ShouldReply() != 0U)
    {
        GpLedMatrixAi8051u_BuildReply(status);
    }
    return status;
}

static void GpLedMatrixAi8051u_FlushReply(GpLedMatrixAi8051uContext xdata *context)
{
    if ((context == 0) || (context->txPending == 0U))
    {
        return;
    }

    if (context->txLength > 0xFFU)
    {
        context->lastStatus = (uint8_t)kGpMatrixStatusInternalError;
        context->txPending = 0U;
        return;
    }

    if (UART2_SendBuffer(context->txBuffer, (uint8_t)context->txLength) == 0U)
    {
        context->lastStatus = (uint8_t)kGpMatrixStatusBusy;
        return;
    }

    context->txPending = 0U;
}

static void GpLedMatrixAi8051u_ProcessPendingPacket(GpLedMatrixAi8051uContext xdata *context)
{
    GpMatrixStatusCode status;

    if ((context == 0) || (context->packetPending == 0U))
    {
        return;
    }

    g_gpMatrixPacketLength = context->packetLength;
    context->packetPending = 0U;
    context->packetLength = 0U;

    status = GpLedMatrixAi8051u_ProcessPacket(context, context->rxBuffer, g_gpMatrixPacketLength);
    if ((context->packetReplyPrepared == 0U) && (GpLedMatrixAi8051u_ShouldReply() != 0U))
    {
        GpLedMatrixAi8051u_BuildReply(status);
    }
    context->packetReplyPrepared = 0U;
    GpLedMatrixAi8051u_FlushReply(context);
}

void GpLedMatrixAi8051u_Init(GpLedMatrixAi8051uContext xdata *context, uint8_t transportAddress)
{
    if (context == 0)
    {
        return;
    }

    GpLedMatrixAi8051u_ResetContext(context, transportAddress);
    g_gpMatrixCtx = context;
    GpLedAction_Init();
    GpLedAction_SetBrightness(context->brightness);

    GpLedMatrixAi8051u_ShutdownLegacyI2cHardware();
    GpLedMatrixAi8051u_ResetPacketAssembly();
    UART2_Init();
    GpLedMatrixBtDebug_SetReady(1U);
    GpLedMatrixBtDebug_PrintInit();
}

uint8_t GpLedMatrixAi8051u_PrepareTx(GpLedMatrixAi8051uContext xdata *context, uint8_t *outData, uint8_t maxLength)
{
    if ((context == 0) || (outData == 0) || (context->txPending == 0U))
    {
        return 0U;
    }

    g_gpMatrixCopyLength = (uint8_t)context->txLength;
    if (g_gpMatrixCopyLength > maxLength)
    {
        g_gpMatrixCopyLength = maxLength;
    }

    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < g_gpMatrixCopyLength; ++g_gpMatrixLoopIndex)
    {
        outData[g_gpMatrixLoopIndex] = context->txBuffer[g_gpMatrixLoopIndex];
    }

    context->txPending = 0U;
    return g_gpMatrixCopyLength;
}

void GpLedMatrixAi8051u_Poll(GpLedMatrixAi8051uContext xdata *context)
{
    uint8_t rxByte;
    uint8_t hasRxByte;

    if (context == 0)
    {
        return;
    }

    UART2_ServiceRx();

    if (UART2_TakeRxOverflow() != 0U)
    {
        printf("[BT_WARN] uart2_rx_overflow\r\n");
    }

    hasRxByte = 0U;
    while (UART2_TryPopByte(&rxByte) != 0U)
    {
        hasRxByte = 1U;
        g_gpMatrixStreamIdleTicks = 0U;
        GpLedMatrixAi8051u_PushStreamByte(context, rxByte);
        GpLedMatrixAi8051u_ProcessPendingPacket(context);
    }

    if ((hasRxByte == 0U) && (g_gpMatrixStreamLength != 0U))
    {
        g_gpMatrixStreamIdleTicks++;
        if (g_gpMatrixStreamIdleTicks >= GP_MATRIX_UART2_STREAM_IDLE_RESET_TICKS)
        {
            printf("[GP_SYNC] idle_reset len=%u\r\n", (unsigned int)g_gpMatrixStreamLength);
            GpLedMatrixAi8051u_ResetPacketAssembly();
        }
    }

    GpLedMatrixAi8051u_ProcessPendingPacket(context);
}

void GpLedMatrixAi8051u_RenderFrame(GpLedMatrixAi8051uContext xdata *context)
{
    if (context == 0)
    {
        return;
    }

    (void)GpLedAction_ApplyFrameRgb332(context->frameBuffer,
                                       GP_MATRIX_RGB332_FRAME_SIZE,
                                       (GpMatrixMode)context->mode);
}

void GpLedMatrixAi8051u_LoadGlyphRows(GpLedMatrixAi8051uContext xdata *context,
                                      const uint16_t *rows,
                                      uint8_t glyphCount,
                                      uint8_t glyphWidth,
                                      uint8_t glyphSpacing)
{
    static uint16_t rowData;

    if ((context == 0) || (rows == 0))
    {
        return;
    }
    if (GP_MATRIX_GLYPH_ROWS_SIZE > GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES)
    {
        return;
    }

    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < GP_MATRIX_HEIGHT; ++g_gpMatrixLoopIndex)
    {
        rowData = rows[g_gpMatrixLoopIndex];
        context->glyphBuffer[(uint16_t)g_gpMatrixLoopIndex * 2U] = (uint8_t)(rowData & 0xFFU);
        context->glyphBuffer[(uint16_t)g_gpMatrixLoopIndex * 2U + 1U] = (uint8_t)((rowData >> 8) & 0xFFU);
    }

    (void)GpLedAction_ApplyGlyphRows(context->glyphBuffer,
                                     GP_MATRIX_GLYPH_ROWS_SIZE,
                                     glyphCount,
                                     glyphWidth,
                                     glyphSpacing);
}

