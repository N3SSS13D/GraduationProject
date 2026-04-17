#include "config.h"
#include "gp_led_action.h"
#include "gp_led_matrix_ai8051u.h"

#define GP_MATRIX_I2C_PIN_MASK ((uint8_t)((1U << 3) | (1U << 4)))
#define GP_MATRIX_PROTOCOL_MIN_PACKET_LENGTH GP_MATRIX_PACKET_OVERHEAD_SIZE
#define GP_MATRIX_STATUS_PAYLOAD_BYTES 4U
#define GP_MATRIX_I2C_DEBUG_ENABLE 1U

/* Parser helpers use shared scratch state so command handlers do not consume extra stack. */
static GpLedMatrixAi8051uContext xdata *g_gpMatrixCtx = 0;
static GpLedMatrixAi8051uContext xdata *g_gpMatrixI2cContext = 0;
static const uint8_t xdata *g_gpMatrixPayload = 0;
static uint16_t xdata g_gpMatrixPayloadLength = 0U;
static uint8_t xdata g_gpMatrixSequence = 0U;
static uint8_t xdata g_gpMatrixCommand = 0U;
static uint8_t xdata g_gpMatrixPacketLength = 0U;
static uint8_t xdata g_gpMatrixExpectedLength = 0U;
static uint8_t xdata g_gpMatrixChunkSize = 0U;
static uint16_t xdata g_gpMatrixChunkOffset = 0U;
static uint16_t xdata g_gpMatrixTotalBytes = 0U;
static uint16_t xdata g_gpMatrixLoopIndex = 0U;
static uint16_t xdata g_gpMatrixI2cDmaAddress = 0U;
static uint16_t xdata g_gpMatrixI2cDmaDone = 0U;
static uint8_t xdata g_gpMatrixCopyLength = 0U;
static uint8_t xdata g_gpMatrixI2cRxCount = 0U;
static uint8_t xdata g_gpMatrixI2cTxIndex = 0U;
static uint8_t xdata g_gpMatrixI2cExpectAddrByte = 0U;
static uint8_t xdata g_gpMatrixI2cLastAddrByte = 0U;
static uint8_t xdata g_gpMatrixReplyPayload[GP_MATRIX_STATUS_PAYLOAD_BYTES];
static GpMatrixActionPayload xdata g_gpMatrixAction;
#if GP_MATRIX_I2C_DEBUG_ENABLE
static volatile uint16_t g_gpMatrixI2cStartCount = 0U;
static volatile uint16_t g_gpMatrixI2cStopCount = 0U;
static volatile uint16_t g_gpMatrixI2cTimeoutCount = 0U;
static volatile uint16_t g_gpMatrixI2cOverflowCount = 0U;
static volatile uint16_t g_gpMatrixI2cRxByteCount = 0U;
static volatile uint16_t g_gpMatrixI2cTxByteCount = 0U;
static volatile uint16_t g_gpMatrixI2cEmptyStopCount = 0U;
static volatile uint16_t g_gpMatrixI2cPayloadStopCount = 0U;
static volatile uint16_t g_gpMatrixI2cRestartFlushCount = 0U;
static volatile uint16_t g_gpMatrixI2cPacketDropCount = 0U;
static volatile uint16_t g_gpMatrixI2cProtocolDropCount = 0U;
static volatile uint16_t g_gpMatrixI2cAddrByteCount = 0U;
static volatile uint8_t g_gpMatrixI2cLastStopLength = 0U;
static volatile uint8_t g_gpMatrixI2cDebugFlags = 0U;
static uint16_t xdata g_gpMatrixI2cStartCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cStopCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cTimeoutCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cOverflowCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cRxByteCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cTxByteCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cEmptyStopCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cPayloadStopCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cRestartFlushCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cPacketDropCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cProtocolDropCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cAddrByteCountShadow = 0U;
static uint16_t xdata g_gpMatrixI2cEmptyStopPrinted = 0U;
static uint8_t xdata g_gpMatrixI2cLastStopLengthShadow = 0U;
static uint8_t xdata g_gpMatrixI2cDebugFlagsShadow = 0U;
#define GP_MATRIX_I2C_DEBUG_FLAG_START 0x01U
#define GP_MATRIX_I2C_DEBUG_FLAG_PAYLOAD_STOP 0x02U
#define GP_MATRIX_I2C_DEBUG_FLAG_TIMEOUT 0x04U
#define GP_MATRIX_I2C_DEBUG_FLAG_OVERFLOW 0x08U
#define GP_MATRIX_I2C_DEBUG_FLAG_EMPTY_STOP 0x10U
#define GP_MATRIX_I2C_DEBUG_FLAG_RESTART_FLUSH 0x20U
#define GP_MATRIX_I2C_DEBUG_FLAG_PACKET_DROP 0x40U
#define GP_MATRIX_I2C_DEBUG_FLAG_PROTOCOL_DROP 0x80U
#endif

uint8_t GpMatrixComputeChecksum(const uint8_t *packetBytes, uint16_t byteCount)
{
    uint8_t checksum;
    uint16_t index;

    checksum = 0U;
    for (index = 0U; index < byteCount; ++index)
    {
        checksum ^= packetBytes[index];
    }

    return checksum;
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
    uint16_t value;

    value = srcBytes[0];
    value |= (uint16_t)srcBytes[1] << 8;
    return value;
}

static void GpLedMatrixAi8051u_ResetFrameTransfer(GpLedMatrixAi8051uContext xdata *context)
{
    context->expectedBytes = 0U;
    context->receivedBytes = 0U;
}

static void GpLedMatrixAi8051u_ResetGlyphTransfer(GpLedMatrixAi8051uContext xdata *context)
{
    context->glyphExpectedBytes = 0U;
    context->glyphReceivedBytes = 0U;
    context->glyphCount = 0U;
    context->glyphWidth = 0U;
    context->glyphSpacing = 0U;
}

static void GpLedMatrixAi8051u_ResetI2cTransfer(void)
{
    g_gpMatrixI2cRxCount = 0U;
    g_gpMatrixI2cTxIndex = 0U;
    g_gpMatrixI2cExpectAddrByte = 1U;
}

static void GpLedMatrixAi8051u_UpdateDmaGate(void)
{
    if ((g_gpMatrixI2cContext != 0)
        && ((g_gpMatrixI2cContext->dmaRxActive != 0U) || (g_gpMatrixI2cContext->dmaTxActive != 0U)))
    {
        DMA_I2C_EnableDMA();
        return;
    }

    DMA_I2C_DisableDMA();
}

static void GpLedMatrixAi8051u_StopDmaTx(void)
{
    DMA_I2C_DisableTxInt();
    DMA_I2C_DisableTx();
    DMA_I2C_ClearTxFlag();
    DMA_I2C_ClearOverWriteFlag();
    if (g_gpMatrixI2cContext != 0)
    {
        g_gpMatrixI2cContext->dmaTxActive = 0U;
    }
    I2C_EnableSlaveTXInt();
    GpLedMatrixAi8051u_UpdateDmaGate();
}

static void GpLedMatrixAi8051u_StopDmaRx(void)
{
    DMA_I2C_DisableRxInt();
    DMA_I2C_DisableRx();
    DMA_I2C_ClearRxFlag();
    DMA_I2C_ClearRxLossFlag();
    if (g_gpMatrixI2cContext != 0)
    {
        g_gpMatrixI2cContext->dmaRxActive = 0U;
    }
    I2C_EnableSlaveRXInt();
    GpLedMatrixAi8051u_UpdateDmaGate();
}

static void GpLedMatrixAi8051u_ConfigureDmaBackend(void)
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
    DMA_I2C_SetInterval(0U);
    DMA_I2C_SetTxBusPriority(1U);
    DMA_I2C_SetRxBusPriority(1U);
    DMA_I2C_SetTxIntPriority(1U);
    DMA_I2C_SetRxIntPriority(1U);
}

static uint8_t GpLedMatrixAi8051u_StartDmaTx(void)
{
    if ((g_gpMatrixI2cContext == 0) || (g_gpMatrixI2cContext->dmaTxEnabled == 0U) || (g_gpMatrixI2cContext->txPending == 0U))
    {
        return 0U;
    }

    DMA_I2C_ClearTxFlag();
    DMA_I2C_ClearOverWriteFlag();
    g_gpMatrixI2cDmaAddress = (uint16_t)g_gpMatrixI2cContext->txBuffer;
    DMA_I2C_SetTxAddress(g_gpMatrixI2cDmaAddress);
    DMA_I2C_SetTxAmount((uint16_t)(g_gpMatrixI2cContext->txLength - 1U));
    DMA_I2C_EnableTxInt();
    DMA_I2C_EnableTx();
    g_gpMatrixI2cContext->dmaTxActive = 1U;
    I2C_DisableSlaveTXInt();
    GpLedMatrixAi8051u_UpdateDmaGate();
    DMA_I2C_TriggerTx();
    return 1U;
}

static void GpLedMatrixAi8051u_StartDmaRx(void)
{
    if ((g_gpMatrixI2cContext == 0) || (g_gpMatrixI2cContext->dmaRxEnabled == 0U))
    {
        return;
    }

    DMA_I2C_ClearFIFO();
    DMA_I2C_ClearRxFlag();
    DMA_I2C_ClearRxLossFlag();
    g_gpMatrixI2cDmaAddress = (uint16_t)g_gpMatrixI2cContext->rxBuffer;
    DMA_I2C_SetRxAddress(g_gpMatrixI2cDmaAddress);
    DMA_I2C_SetRxAmount((uint16_t)(GP_MATRIX_AI8051U_RX_BUFFER_SIZE - 1U));
    DMA_I2C_EnableRxInt();
    DMA_I2C_EnableRx();
    g_gpMatrixI2cContext->dmaRxActive = 1U;
    g_gpMatrixI2cExpectAddrByte = 0U;
    I2C_DisableSlaveRXInt();
    GpLedMatrixAi8051u_UpdateDmaGate();
    DMA_I2C_TriggerRx();
}

static uint8_t GpLedMatrixAi8051u_FinalizeDmaRxPacket(uint8_t queuePacket)
{
    if ((g_gpMatrixI2cContext == 0) || (g_gpMatrixI2cContext->dmaRxActive == 0U))
    {
        return 0U;
    }

    g_gpMatrixI2cDmaDone = DMA_I2C_ReadRxDone();
    if (g_gpMatrixI2cDmaDone > GP_MATRIX_AI8051U_RX_BUFFER_SIZE)
    {
        g_gpMatrixI2cDmaDone = GP_MATRIX_AI8051U_RX_BUFFER_SIZE;
    }

    g_gpMatrixI2cContext->dmaLastRxDone = g_gpMatrixI2cDmaDone;
    g_gpMatrixI2cRxCount = (uint8_t)g_gpMatrixI2cDmaDone;
    if ((g_gpMatrixI2cRxCount > 1U)
        && (g_gpMatrixI2cContext->rxBuffer[0] != GP_MATRIX_PROTOCOL_MAGIC0)
        && (g_gpMatrixI2cContext->rxBuffer[1] == GP_MATRIX_PROTOCOL_MAGIC0))
    {
        GpLedMatrixAi8051u_CopyBytes(g_gpMatrixI2cContext->rxBuffer,
                                     &g_gpMatrixI2cContext->rxBuffer[1],
                                     (uint16_t)(g_gpMatrixI2cRxCount - 1U));
        g_gpMatrixI2cRxCount--;
    }

    if ((queuePacket != 0U) && (g_gpMatrixI2cRxCount != 0U))
    {
        g_gpMatrixI2cLastStopLength = g_gpMatrixI2cRxCount;
        GpLedMatrixAi8051u_QueueRxPacketFromIsr();
    }

    GpLedMatrixAi8051u_StopDmaRx();
    return g_gpMatrixI2cRxCount;
}

static void GpLedMatrixAi8051u_QueueRxPacketFromIsr(void)
{
    if ((g_gpMatrixI2cContext == 0) || (g_gpMatrixI2cRxCount == 0U))
    {
        return;
    }

    if (g_gpMatrixI2cContext->packetPending == 0U)
    {
        g_gpMatrixI2cContext->packetLength = g_gpMatrixI2cRxCount;
        g_gpMatrixI2cContext->packetPending = 1U;
        return;
    }

#if GP_MATRIX_I2C_DEBUG_ENABLE
    g_gpMatrixI2cPacketDropCount++;
    g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_PACKET_DROP;
#endif
}

static uint8_t GpLedMatrixAi8051u_HasProtocolPrefix(const uint8_t xdata *packet, uint8_t packetLength)
{
    if ((packet == 0) || (packetLength < 3U))
    {
        return 0U;
    }

    if ((packet[0] != GP_MATRIX_PROTOCOL_MAGIC0)
        || (packet[1] != GP_MATRIX_PROTOCOL_MAGIC1)
        || (packet[2] != GP_MATRIX_PROTOCOL_VERSION))
    {
        return 0U;
    }

    return 1U;
}

#if GP_MATRIX_I2C_DEBUG_ENABLE
static void GpLedMatrixAi8051u_ReportI2cDebugSnapshot(void)
{
    DisableGlobalInt();
    g_gpMatrixI2cStartCountShadow = g_gpMatrixI2cStartCount;
    g_gpMatrixI2cStopCountShadow = g_gpMatrixI2cStopCount;
    g_gpMatrixI2cTimeoutCountShadow = g_gpMatrixI2cTimeoutCount;
    g_gpMatrixI2cOverflowCountShadow = g_gpMatrixI2cOverflowCount;
    g_gpMatrixI2cRxByteCountShadow = g_gpMatrixI2cRxByteCount;
    g_gpMatrixI2cTxByteCountShadow = g_gpMatrixI2cTxByteCount;
    g_gpMatrixI2cEmptyStopCountShadow = g_gpMatrixI2cEmptyStopCount;
    g_gpMatrixI2cPayloadStopCountShadow = g_gpMatrixI2cPayloadStopCount;
    g_gpMatrixI2cRestartFlushCountShadow = g_gpMatrixI2cRestartFlushCount;
    g_gpMatrixI2cPacketDropCountShadow = g_gpMatrixI2cPacketDropCount;
    g_gpMatrixI2cProtocolDropCountShadow = g_gpMatrixI2cProtocolDropCount;
    g_gpMatrixI2cAddrByteCountShadow = g_gpMatrixI2cAddrByteCount;
    g_gpMatrixI2cLastStopLengthShadow = g_gpMatrixI2cLastStopLength;
    g_gpMatrixI2cDebugFlagsShadow = g_gpMatrixI2cDebugFlags;
    g_gpMatrixI2cDebugFlags = 0U;
    EnableGlobalInt();

    if ((g_gpMatrixI2cDebugFlagsShadow & GP_MATRIX_I2C_DEBUG_FLAG_TIMEOUT) != 0U)
    {
        printf("[I2C_WARN] timeout=%u start=%u stop=%u rx=%u tx=%u\r\n",
               (unsigned int)g_gpMatrixI2cTimeoutCountShadow,
               (unsigned int)g_gpMatrixI2cStartCountShadow,
               (unsigned int)g_gpMatrixI2cStopCountShadow,
               (unsigned int)g_gpMatrixI2cRxByteCountShadow,
               (unsigned int)g_gpMatrixI2cTxByteCountShadow);
    }

    if ((g_gpMatrixI2cDebugFlagsShadow & GP_MATRIX_I2C_DEBUG_FLAG_OVERFLOW) != 0U)
    {
        printf("[I2C_WARN] overflow=%u last_stop_len=%u\r\n",
               (unsigned int)g_gpMatrixI2cOverflowCountShadow,
               (unsigned int)g_gpMatrixI2cLastStopLengthShadow);
    }

    if ((g_gpMatrixI2cDebugFlagsShadow & GP_MATRIX_I2C_DEBUG_FLAG_PACKET_DROP) != 0U)
    {
        printf("[I2C_WARN] pending_drop=%u last_len=%u\r\n",
               (unsigned int)g_gpMatrixI2cPacketDropCountShadow,
               (unsigned int)g_gpMatrixI2cLastStopLengthShadow);
    }

    if ((g_gpMatrixI2cDebugFlagsShadow & GP_MATRIX_I2C_DEBUG_FLAG_PROTOCOL_DROP) != 0U)
    {
        printf("[I2C_DROP] protocol_drop=%u last_len=%u\r\n",
               (unsigned int)g_gpMatrixI2cProtocolDropCountShadow,
               (unsigned int)g_gpMatrixI2cLastStopLengthShadow);
    }

    if ((g_gpMatrixI2cDebugFlagsShadow & GP_MATRIX_I2C_DEBUG_FLAG_RESTART_FLUSH) != 0U)
    {
        printf("[I2C_EVT] restart_flush len=%u payload_stop=%u empty_stop=%u\r\n",
               (unsigned int)g_gpMatrixI2cLastStopLengthShadow,
               (unsigned int)g_gpMatrixI2cPayloadStopCountShadow,
               (unsigned int)g_gpMatrixI2cEmptyStopCountShadow);
    }

    if ((g_gpMatrixI2cDebugFlagsShadow & GP_MATRIX_I2C_DEBUG_FLAG_PAYLOAD_STOP) != 0U)
    {
        printf("[I2C_EVT] payload_stop len=%u start=%u stop=%u payload=%u empty=%u rx=%u tx=%u pending=%u\r\n",
               (unsigned int)g_gpMatrixI2cLastStopLengthShadow,
               (unsigned int)g_gpMatrixI2cStartCountShadow,
               (unsigned int)g_gpMatrixI2cStopCountShadow,
               (unsigned int)g_gpMatrixI2cPayloadStopCountShadow,
               (unsigned int)g_gpMatrixI2cEmptyStopCountShadow,
               (unsigned int)g_gpMatrixI2cRxByteCountShadow,
               (unsigned int)g_gpMatrixI2cTxByteCountShadow,
               (unsigned int)((g_gpMatrixCtx != 0) ? g_gpMatrixCtx->packetPending : 0U));
    }

    if (((g_gpMatrixI2cDebugFlagsShadow & GP_MATRIX_I2C_DEBUG_FLAG_EMPTY_STOP) != 0U)
        && ((uint16_t)(g_gpMatrixI2cEmptyStopCountShadow - g_gpMatrixI2cEmptyStopPrinted) >= 16U))
    {
        g_gpMatrixI2cEmptyStopPrinted = g_gpMatrixI2cEmptyStopCountShadow;
        printf("[I2C_IDLE] empty_stop=%u start=%u stop=%u rx=%u busy=%u ack=%u\r\n",
               (unsigned int)g_gpMatrixI2cEmptyStopCountShadow,
               (unsigned int)g_gpMatrixI2cStartCountShadow,
               (unsigned int)g_gpMatrixI2cStopCountShadow,
               (unsigned int)g_gpMatrixI2cRxByteCountShadow,
               (unsigned int)I2C_CheckSlaveBusy(),
               (unsigned int)I2C_SlaveReadACK());
    }
}
#endif

static void GpLedMatrixAi8051u_BuildReply(GpMatrixStatusCode status)
{
    if (g_gpMatrixCtx == 0)
    {
        return;
    }

    g_gpMatrixReplyPayload[0] = (uint8_t)status;
    g_gpMatrixReplyPayload[1] = g_gpMatrixCommand;
    g_gpMatrixReplyPayload[2] = g_gpMatrixSequence;
    g_gpMatrixReplyPayload[3] = g_gpMatrixCtx->mode;

    g_gpMatrixPacketLength = (uint8_t)(GP_MATRIX_PACKET_HEADER_SIZE + GP_MATRIX_STATUS_PAYLOAD_BYTES + 1U);
    if (g_gpMatrixPacketLength > GP_MATRIX_AI8051U_TX_BUFFER_SIZE)
    {
        return;
    }

    g_gpMatrixCtx->txBuffer[0] = GP_MATRIX_PROTOCOL_MAGIC0;
    g_gpMatrixCtx->txBuffer[1] = GP_MATRIX_PROTOCOL_MAGIC1;
    g_gpMatrixCtx->txBuffer[2] = GP_MATRIX_PROTOCOL_VERSION;
    g_gpMatrixCtx->txBuffer[3] = (status == kGpMatrixStatusOk) ? (uint8_t)kGpMatrixCommandStatus : (uint8_t)kGpMatrixCommandError;
    g_gpMatrixCtx->txBuffer[4] = g_gpMatrixSequence;
    g_gpMatrixCtx->txBuffer[5] = 0U;
    g_gpMatrixCtx->txBuffer[6] = GP_MATRIX_STATUS_PAYLOAD_BYTES;
    g_gpMatrixCtx->txBuffer[7] = 0U;
    GpLedMatrixAi8051u_CopyBytes(&g_gpMatrixCtx->txBuffer[GP_MATRIX_PACKET_HEADER_SIZE], g_gpMatrixReplyPayload, GP_MATRIX_STATUS_PAYLOAD_BYTES);
    g_gpMatrixCtx->txBuffer[g_gpMatrixPacketLength - 1U] = GpMatrixComputeChecksum(g_gpMatrixCtx->txBuffer, g_gpMatrixPacketLength - 1U);
    g_gpMatrixCtx->txLength = g_gpMatrixPacketLength;
    g_gpMatrixCtx->txPending = 1U;
    g_gpMatrixCtx->lastStatus = (uint8_t)status;
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

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleFrameStart(void)
{
    if (g_gpMatrixPayloadLength != GP_MATRIX_FRAME_START_PAYLOAD_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }
    if ((g_gpMatrixPayload[0] != GP_MATRIX_PAYLOAD_FORMAT_RGB332)
        || (g_gpMatrixPayload[1] != GP_MATRIX_WIDTH)
        || (g_gpMatrixPayload[2] != GP_MATRIX_HEIGHT))
    {
        return kGpMatrixStatusUnsupported;
    }

    g_gpMatrixTotalBytes = GpLedMatrixAi8051u_ReadLe16(&g_gpMatrixPayload[3]);
    if (g_gpMatrixTotalBytes > GP_MATRIX_RGB332_FRAME_SIZE)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpMatrixCtx->expectedBytes = g_gpMatrixTotalBytes;
    g_gpMatrixCtx->receivedBytes = 0U;
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedMatrixAi8051u_HandleFrameChunk(void)
{
    if (g_gpMatrixPayloadLength < GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
    {
        return kGpMatrixStatusBadLength;
    }
    if (g_gpMatrixCtx->expectedBytes == 0U)
    {
        return kGpMatrixStatusBadSequence;
    }

    g_gpMatrixChunkOffset = (uint16_t)g_gpMatrixPayload[0] * GP_MATRIX_MAX_CHUNK_DATA;
    g_gpMatrixChunkSize = g_gpMatrixPayload[1];
    if ((g_gpMatrixPayloadLength != (uint16_t)g_gpMatrixChunkSize + GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
        || (g_gpMatrixChunkOffset != g_gpMatrixCtx->receivedBytes)
        || ((uint16_t)g_gpMatrixChunkOffset + g_gpMatrixChunkSize > g_gpMatrixCtx->expectedBytes))
    {
        return kGpMatrixStatusBadSequence;
    }

    GpLedMatrixAi8051u_CopyBytes(&g_gpMatrixCtx->frameBuffer[g_gpMatrixChunkOffset], &g_gpMatrixPayload[2], g_gpMatrixChunkSize);
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

    status = GpLedAction_ApplyFrameRgb332(g_gpMatrixCtx->frameBuffer, g_gpMatrixCtx->receivedBytes, mode);
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
        return kGpMatrixStatusBadLength;
    }
    if (g_gpMatrixCtx->glyphExpectedBytes == 0U)
    {
        return kGpMatrixStatusBadSequence;
    }

    g_gpMatrixChunkOffset = (uint16_t)g_gpMatrixPayload[0] * GP_MATRIX_MAX_CHUNK_DATA;
    g_gpMatrixChunkSize = g_gpMatrixPayload[1];
    if ((g_gpMatrixPayloadLength != (uint16_t)g_gpMatrixChunkSize + GP_MATRIX_FRAME_CHUNK_PREFIX_BYTES)
        || (g_gpMatrixChunkOffset != g_gpMatrixCtx->glyphReceivedBytes)
        || ((uint16_t)g_gpMatrixChunkOffset + g_gpMatrixChunkSize > g_gpMatrixCtx->glyphExpectedBytes))
    {
        return kGpMatrixStatusBadSequence;
    }

    GpLedMatrixAi8051u_CopyBytes(&g_gpMatrixCtx->glyphBuffer[g_gpMatrixChunkOffset], &g_gpMatrixPayload[2], g_gpMatrixChunkSize);
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
    if ((g_gpMatrixCtx->glyphExpectedBytes == 0U) || (g_gpMatrixCtx->glyphReceivedBytes != g_gpMatrixCtx->glyphExpectedBytes))
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

static GpMatrixStatusCode GpLedMatrixAi8051u_ProcessPacket(GpLedMatrixAi8051uContext xdata *context,
                                                           const uint8_t xdata *packet,
                                                           uint8_t packetLength)
{
    GpMatrixStatusCode status;
    uint8_t command;
    uint8_t sequence;
    uint16_t payloadLength;
    uint8_t expectedLength;
    const uint8_t xdata *payload;

    if (packetLength < GP_MATRIX_PACKET_OVERHEAD_SIZE)
    {
        return kGpMatrixStatusBadLength;
    }
    if ((packet[0] != GP_MATRIX_PROTOCOL_MAGIC0)
        || (packet[1] != GP_MATRIX_PROTOCOL_MAGIC1)
        || (packet[2] != GP_MATRIX_PROTOCOL_VERSION))
    {
        return kGpMatrixStatusUnsupported;
    }
    if (GpMatrixComputeChecksum(packet, packetLength - 1U) != packet[packetLength - 1U])
    {
        return kGpMatrixStatusBadChecksum;
    }

    command = packet[3];
    sequence = packet[4];
    payloadLength = GpLedMatrixAi8051u_ReadLe16(&packet[6]);
    expectedLength = (uint8_t)(GP_MATRIX_PACKET_HEADER_SIZE + payloadLength + 1U);
    if (expectedLength != packetLength)
    {
        return kGpMatrixStatusBadLength;
    }

    context->lastCommand = command;
    context->lastSequence = sequence;
    payload = &packet[GP_MATRIX_PACKET_HEADER_SIZE];
    g_gpMatrixCtx = context;
    g_gpMatrixCommand = command;
    g_gpMatrixSequence = sequence;
    g_gpMatrixPayload = payload;
    g_gpMatrixPayloadLength = payloadLength;
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

        case kGpMatrixCommandFrameStart:
            status = GpLedMatrixAi8051u_HandleFrameStart();
            break;

        case kGpMatrixCommandFrameChunk:
            status = GpLedMatrixAi8051u_HandleFrameChunk();
            break;

        case kGpMatrixCommandFrameCommit:
            status = GpLedMatrixAi8051u_HandleFrameCommit();
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

    GpLedMatrixAi8051u_BuildReply(status);
    return status;
}

void GpLedMatrixAi8051u_Init(GpLedMatrixAi8051uContext xdata *context, uint8_t i2cAddress)
{
    if (context == 0)
    {
        return;
    }

    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < GP_MATRIX_RGB332_FRAME_SIZE; ++g_gpMatrixLoopIndex)
    {
        context->frameBuffer[g_gpMatrixLoopIndex] = 0U;
    }
    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES; ++g_gpMatrixLoopIndex)
    {
        context->glyphBuffer[g_gpMatrixLoopIndex] = 0U;
    }

    context->i2cAddress = i2cAddress;
    context->brightness = 200U;
    context->mode = (uint8_t)kGpMatrixModeSolidFrame;
    context->lastSequence = 0U;
    context->lastCommand = 0U;
    context->lastStatus = (uint8_t)kGpMatrixStatusOk;
    context->packetLength = 0U;
    context->packetPending = 0U;
    context->txLength = 0U;
    context->txPending = 0U;
    context->dmaRxEnabled = 0U;
    context->dmaTxEnabled = 0U;
    context->dmaRxActive = 0U;
    context->dmaTxActive = 0U;
    GpLedMatrixAi8051u_ResetFrameTransfer(context);
    GpLedMatrixAi8051u_ResetGlyphTransfer(context);
    context->dmaLastRxDone = 0U;
    context->dmaLastTxDone = 0U;
    g_gpMatrixCtx = context;
    g_gpMatrixI2cContext = context;
    GpLedMatrixAi8051u_ResetI2cTransfer();

    GpLedAction_Init();
    GpLedAction_SetBrightness(context->brightness);

    /* Keep P2.4/P2.3 released high as open-drain I2C pins and rely only on the external 3.3 V pull-up network. */
    SetP2nInitLevelHigh(GP_MATRIX_I2C_PIN_MASK);
    SetP2nOpenDrainMode(GP_MATRIX_I2C_PIN_MASK);
    SetP2nDigitalInput(GP_MATRIX_I2C_PIN_MASK);
    DisableP2nPullUp(GP_MATRIX_I2C_PIN_MASK);
    DisableP2nPullDown(GP_MATRIX_I2C_PIN_MASK);

    /* Keep the I2C DMA block initialized but idle until the higher-level mode switch explicitly enables a direction. */
    GpLedMatrixAi8051u_ConfigureDmaBackend();

    /* Route the hardware I2C block to P2.3/P2.4, where this board uses P2.4 as SCL and P2.3 as SDA. */
    I2C_SwitchP2324();
    I2C_DisableSlaveAllInt();
    I2C_DisableTimeoutInt();
    I2C_DisableTimeout();
    I2C_Disable();
    I2C_SlaveMode();
    I2C_SetSlaveAddress(i2cAddress);
    CLR_REG_BIT(I2CSLADR, I2CSLADR_MA_MSK);
    I2C_TimeoutScale_1us();
    I2C_SetTimeoutInterval(5000U);
    I2C_SlaveReset();
    I2C_ClearSlaveAllFlag();
    I2C_ClearTimeoutFlag();
    I2C_SlaveSetACK();
    I2C_EnableTimeout();
    I2C_EnableTimeoutInt();
    I2C_SetIntPriority(1);
    I2C_Enable();
    I2C_EnableSlaveAllInt();

#if GP_MATRIX_I2C_DEBUG_ENABLE
    printf("[I2C_INIT] addr=0x%02X pins=P24(SCL),P23(SDA) mode=open-drain pullup=disabled ext=3V3 speed<=100k\r\n",
           (unsigned int)i2cAddress);
#endif
}

void GpLedMatrixAi8051u_SetDmaMode(GpLedMatrixAi8051uContext xdata *context, uint8_t enableRx, uint8_t enableTx)
{
    if (context == 0)
    {
        return;
    }

    context->dmaLastRxDone = 0U;
    context->dmaLastTxDone = 0U;
    context->dmaRxEnabled = (enableRx != 0U) ? 1U : 0U;
    context->dmaTxEnabled = (enableTx != 0U) ? 1U : 0U;

    if (context->dmaRxEnabled == 0U)
    {
        GpLedMatrixAi8051u_StopDmaRx();
    }

    if (context->dmaTxEnabled == 0U)
    {
        GpLedMatrixAi8051u_StopDmaTx();
    }

#if GP_MATRIX_I2C_DEBUG_ENABLE
    printf("[I2C_DMA] rx=%u tx=%u\r\n",
           (unsigned int)context->dmaRxEnabled,
           (unsigned int)context->dmaTxEnabled);
#endif
}

void GpLedMatrixAi8051u_OnI2cReceive(GpLedMatrixAi8051uContext xdata *context, const uint8_t *rxBytes, uint8_t length)
{
    if ((context == 0) || (rxBytes == 0) || (length == 0U))
    {
        return;
    }
    if (length > GP_MATRIX_AI8051U_RX_BUFFER_SIZE)
    {
        g_gpMatrixCtx = context;
        g_gpMatrixSequence = context->lastSequence;
        g_gpMatrixCommand = context->lastCommand;
        GpLedMatrixAi8051u_BuildReply(kGpMatrixStatusBadLength);
        return;
    }
    if (context->packetPending != 0U)
    {
        g_gpMatrixCtx = context;
        g_gpMatrixSequence = context->lastSequence;
        g_gpMatrixCommand = context->lastCommand;
        GpLedMatrixAi8051u_BuildReply(kGpMatrixStatusBusy);
        return;
    }

    for (g_gpMatrixLoopIndex = 0U; g_gpMatrixLoopIndex < length; ++g_gpMatrixLoopIndex)
    {
        context->rxBuffer[g_gpMatrixLoopIndex] = rxBytes[g_gpMatrixLoopIndex];
    }
    context->packetLength = length;
    context->packetPending = 1U;
}

uint8_t GpLedMatrixAi8051u_PrepareTx(GpLedMatrixAi8051uContext xdata *context, uint8_t *outData, uint8_t maxLength)
{
    if ((context == 0) || (outData == 0) || (context->txPending == 0U))
    {
        return 0U;
    }

    g_gpMatrixCopyLength = context->txLength;
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
    GpMatrixStatusCode status;

    if (context == 0)
    {
        return;
    }

#if GP_MATRIX_I2C_DEBUG_ENABLE
    GpLedMatrixAi8051u_ReportI2cDebugSnapshot();
#endif

    if (I2C_CheckTimeoutFlag() != 0)
    {
        I2C_ClearTimeoutFlag();
        g_gpMatrixCtx = context;
        g_gpMatrixSequence = context->lastSequence;
        g_gpMatrixCommand = context->lastCommand;
        GpLedMatrixAi8051u_BuildReply(kGpMatrixStatusBusy);
    }
    if (context->packetPending == 0U)
    {
        return;
    }

    g_gpMatrixPacketLength = context->packetLength;
    context->packetPending = 0U;
    context->packetLength = 0U;

    if ((g_gpMatrixPacketLength < GP_MATRIX_PROTOCOL_MIN_PACKET_LENGTH)
        || (GpLedMatrixAi8051u_HasProtocolPrefix(context->rxBuffer, g_gpMatrixPacketLength) == 0U))
    {
#if GP_MATRIX_I2C_DEBUG_ENABLE
        g_gpMatrixI2cProtocolDropCount++;
        g_gpMatrixI2cLastStopLength = g_gpMatrixPacketLength;
        g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_PROTOCOL_DROP;
#endif
        return;
    }

    status = GpLedMatrixAi8051u_ProcessPacket(context, context->rxBuffer, g_gpMatrixPacketLength);

#if GP_MATRIX_I2C_DEBUG_ENABLE
    printf("[I2C_PKT] len=%u cmd=0x%02X seq=%u status=0x%02X reply=%u\r\n",
           (unsigned int)g_gpMatrixPacketLength,
           (unsigned int)context->lastCommand,
           (unsigned int)context->lastSequence,
           (unsigned int)status,
           (unsigned int)context->txLength);
#endif
}

void GpLedMatrixAi8051u_RenderFrame(GpLedMatrixAi8051uContext xdata *context)
{
    if (context == 0)
    {
        return;
    }

    (void)GpLedAction_ApplyFrameRgb332(context->frameBuffer, GP_MATRIX_RGB332_FRAME_SIZE, (GpMatrixMode)context->mode);
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

void GpLedMatrixAi8051u_ISR(void) interrupt 24
{
    uint8_t rxByte;
    uint8_t txByte;

    if (I2C_CheckSlaveSTAFlag() != 0)
    {
        I2C_ClearSlaveSTAFlag();
        I2C_SlaveSetACK();
#if GP_MATRIX_I2C_DEBUG_ENABLE
        g_gpMatrixI2cStartCount++;
        g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_START;
#endif

        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaTxActive != 0U))
        {
            g_gpMatrixI2cContext->dmaLastTxDone = DMA_I2C_ReadTxDone();
            g_gpMatrixI2cContext->txPending = 0U;
            GpLedMatrixAi8051u_StopDmaTx();
        }

        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaRxActive != 0U))
        {
            g_gpMatrixI2cLastStopLength = GpLedMatrixAi8051u_FinalizeDmaRxPacket(1U);
        }

        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaRxEnabled != 0U))
        {
            GpLedMatrixAi8051u_ResetI2cTransfer();
            GpLedMatrixAi8051u_StartDmaRx();
            return;
        }

        if (g_gpMatrixI2cRxCount != 0U)
        {
            g_gpMatrixI2cLastStopLength = g_gpMatrixI2cRxCount;
            GpLedMatrixAi8051u_QueueRxPacketFromIsr();
    #if GP_MATRIX_I2C_DEBUG_ENABLE
            g_gpMatrixI2cRestartFlushCount++;
            g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_RESTART_FLUSH;
    #endif
        }

        GpLedMatrixAi8051u_ResetI2cTransfer();
    }

    if (I2C_CheckSlaveRXFlag() != 0)
    {
        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaRxActive != 0U))
        {
            I2C_ClearSlaveRXFlag();
            I2C_SlaveSetACK();
            return;
        }

        I2C_ClearSlaveRXFlag();
        I2C_SlaveSetACK();
        rxByte = I2C_ReadData();
        if (g_gpMatrixI2cExpectAddrByte != 0U)
        {
            g_gpMatrixI2cExpectAddrByte = 0U;
            g_gpMatrixI2cLastAddrByte = rxByte;
#if GP_MATRIX_I2C_DEBUG_ENABLE
            g_gpMatrixI2cAddrByteCount++;
#endif
            return;
        }

        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cRxCount < GP_MATRIX_AI8051U_RX_BUFFER_SIZE))
        {
            g_gpMatrixI2cContext->rxBuffer[g_gpMatrixI2cRxCount] = rxByte;
            g_gpMatrixI2cRxCount++;
#if GP_MATRIX_I2C_DEBUG_ENABLE
            g_gpMatrixI2cRxByteCount++;
#endif
        }
#if GP_MATRIX_I2C_DEBUG_ENABLE
        else
        {
            g_gpMatrixI2cOverflowCount++;
            g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_OVERFLOW;
        }
#endif
    }

    if (I2C_CheckSlaveTXFlag() != 0)
    {
        I2C_ClearSlaveTXFlag();
        I2C_SlaveSetACK();

        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaTxActive != 0U))
        {
            return;
        }

        if ((g_gpMatrixI2cContext != 0)
            && (g_gpMatrixI2cContext->dmaTxEnabled != 0U)
            && (g_gpMatrixI2cContext->txPending != 0U)
            && (GpLedMatrixAi8051u_StartDmaTx() != 0U))
        {
            return;
        }

        txByte = 0U;
        if ((g_gpMatrixI2cContext != 0)
            && (g_gpMatrixI2cContext->txPending != 0U)
            && (g_gpMatrixI2cTxIndex < g_gpMatrixI2cContext->txLength))
        {
            txByte = g_gpMatrixI2cContext->txBuffer[g_gpMatrixI2cTxIndex];
            g_gpMatrixI2cTxIndex++;
            if (g_gpMatrixI2cTxIndex >= g_gpMatrixI2cContext->txLength)
            {
                g_gpMatrixI2cContext->txPending = 0U;
            }
        }
#if GP_MATRIX_I2C_DEBUG_ENABLE
        g_gpMatrixI2cTxByteCount++;
#endif
        I2C_WriteData(txByte);
    }

    if (I2C_CheckSlaveSTOFlag() != 0)
    {
        I2C_ClearSlaveSTOFlag();
        I2C_SlaveSetACK();
#if GP_MATRIX_I2C_DEBUG_ENABLE
        g_gpMatrixI2cStopCount++;
        g_gpMatrixI2cLastStopLength = g_gpMatrixI2cRxCount;
#endif

    if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaTxActive != 0U))
    {
        g_gpMatrixI2cContext->dmaLastTxDone = DMA_I2C_ReadTxDone();
        g_gpMatrixI2cContext->txPending = 0U;
        GpLedMatrixAi8051u_StopDmaTx();
    }

    if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaRxActive != 0U))
    {
        g_gpMatrixI2cLastStopLength = GpLedMatrixAi8051u_FinalizeDmaRxPacket(1U);
    }
    else if (g_gpMatrixI2cRxCount != 0U)
        {
#if GP_MATRIX_I2C_DEBUG_ENABLE
            g_gpMatrixI2cPayloadStopCount++;
            g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_PAYLOAD_STOP;
#endif
            GpLedMatrixAi8051u_QueueRxPacketFromIsr();
        }
        else
        {
#if GP_MATRIX_I2C_DEBUG_ENABLE
            g_gpMatrixI2cEmptyStopCount++;
            g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_EMPTY_STOP;
#endif
        }
        GpLedMatrixAi8051u_ResetI2cTransfer();
    }

    if (I2C_CheckTimeoutFlag() != 0)
    {
        I2C_ClearTimeoutFlag();
        I2C_SlaveSetACK();
#if GP_MATRIX_I2C_DEBUG_ENABLE
        g_gpMatrixI2cTimeoutCount++;
        g_gpMatrixI2cDebugFlags |= GP_MATRIX_I2C_DEBUG_FLAG_TIMEOUT;
#endif
        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaTxActive != 0U))
        {
            GpLedMatrixAi8051u_StopDmaTx();
        }
        if ((g_gpMatrixI2cContext != 0) && (g_gpMatrixI2cContext->dmaRxActive != 0U))
        {
            GpLedMatrixAi8051u_FinalizeDmaRxPacket(0U);
        }
        GpLedMatrixAi8051u_ResetI2cTransfer();
    }
}

void GpLedMatrixAi8051u_DmaTxISR(void) interrupt DMA_I2CT_VECTOR
{
    if (DMA_I2C_CheckTxFlag() != 0U)
    {
        if (g_gpMatrixI2cContext != 0)
        {
            g_gpMatrixI2cContext->dmaLastTxDone = DMA_I2C_ReadTxDone();
            g_gpMatrixI2cContext->txPending = 0U;
        }
        DMA_I2C_ClearTxFlag();
        GpLedMatrixAi8051u_StopDmaTx();
    }

    if (DMA_I2C_CheckOverWriteFlag() != 0U)
    {
        DMA_I2C_ClearOverWriteFlag();
        if (g_gpMatrixI2cContext != 0)
        {
            g_gpMatrixI2cContext->txPending = 0U;
        }
        GpLedMatrixAi8051u_StopDmaTx();
    }
}

void GpLedMatrixAi8051u_DmaRxISR(void) interrupt DMA_I2CR_VECTOR
{
    if (DMA_I2C_CheckRxFlag() != 0U)
    {
        DMA_I2C_ClearRxFlag();
        (void)GpLedMatrixAi8051u_FinalizeDmaRxPacket(1U);
    }

    if (DMA_I2C_CheckRxLossFlag() != 0U)
    {
        DMA_I2C_ClearRxLossFlag();
        (void)GpLedMatrixAi8051u_FinalizeDmaRxPacket(0U);
    }
}