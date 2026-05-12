#include "config.h"
#include "draw_drv.h"
#include "offline_pattern.h"
#include "gp_led_action.h"
#include "rtc_clock.h"
#include "local_display_assets.h"
#include "ws2812_drv.h"

#define GP_LED_COMM_ACTIVE_TIMEOUT_TICKS 300U
#define GP_LED_DEBUG_FLOW_INTERVAL_TICKS 100U
static uint8_t g_gpLedRemoteActive = 0U;
static uint8_t g_gpLedDirectFrameActive = 0U;
static uint8_t g_gpLedDefaultBrightness = 200U;
static uint8_t g_gpLedCommOnline = 0U;
static uint8_t g_gpLedManualOverrideEnabled = 0U;
static uint8_t g_gpLedManualOverrideOnline = 0U;
static uint8_t g_gpLedEffectiveOnline = 0U;
static uint8_t g_gpLedDebugFlowEnabled = 0U;
static uint8_t g_gpLedDebugFlowIndex = 0U;
static uint8_t g_gpLedAnimationUploadActive = 0U;
static uint8_t g_gpLedAnimationLoopEnabled = 0U;
static uint8_t g_gpLedAnimationActive = 0U;
static uint8_t g_gpLedAnimationFrameCount = 0U;
static uint8_t g_gpLedAnimationFrameIndex = 0U;
static uint8_t g_gpLedAnimationFrameFormat = 0U;
static uint8_t g_gpLedAnimationFramePending = 0U;
static uint16_t g_gpLedAnimationFrameIntervalMs = GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS;
static uint16_t g_gpLedAnimationElapsedMs = 0U;
static uint16_t g_gpLedCommActiveTicks = 0U;
static uint16_t g_gpLedDebugFlowTicks = 0U;
/* Keep buffered playback in one canonical 36-byte single-layer format to cap storage at 32 x 36 bytes. */
static uint8_t xdata g_gpLedAnimationFrames[GP_MATRIX_ANIMATION_MAX_FRAMES][GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES];
static uint8_t xdata g_gpLedAnimationFrameValid[GP_MATRIX_ANIMATION_MAX_FRAMES];
static uint8_t xdata g_gpLedRemotePatternFrame[GP_MATRIX_RGB332_FRAME_SIZE];
static uint8_t xdata g_gpLedTempRow = 0U;
static uint8_t xdata g_gpLedTempCol = 0U;
static uint8_t xdata g_gpLedTempRowMapped = 0U;
static uint8_t xdata g_gpLedTempR = 0U;
static uint8_t xdata g_gpLedTempG = 0U;
static uint8_t xdata g_gpLedTempB = 0U;
static uint8_t xdata g_gpLedPrimaryR = 0U;
static uint8_t xdata g_gpLedPrimaryG = 0U;
static uint8_t xdata g_gpLedPrimaryB = 0U;
static uint8_t xdata g_gpLedBackgroundR = 0U;
static uint8_t xdata g_gpLedBackgroundG = 0U;
static uint8_t xdata g_gpLedBackgroundB = 0U;
static uint16_t xdata g_gpLedTempPixelIndex = 0U;
static uint16_t xdata g_gpLedTempRowBits = 0U;
/* Reuse one profile buffer to keep stack usage stable on AI8051U. */
static GpLedDisplayProfile xdata g_gpLedProfile;

static void GpLedAction_RenderBitmapFrameRgb888(const uint8_t xdata *frameData, uint8_t brightness);
static void GpLedAction_RenderBitmapLayeredFrame(const uint8_t xdata *frameData, uint16_t length, uint8_t brightness);
static uint8_t GpLedAction_ValidateLayeredFrame(const uint8_t xdata *frameData, uint16_t length);
static GpMatrixStatusCode GpLedAction_ApplyDisplayProfileCore(const GpLedDisplayProfile xdata *profile,
                                                              uint8_t requireHostControl,
                                                              uint8_t markRemoteActive);
static uint8_t GpLedAction_StoreCanonicalAnimationFrame(uint8_t frameIndex,
                                                        const uint8_t xdata *frameData,
                                                        uint16_t length);
static uint8_t GpLedAction_PackRgb332(uint8_t red, uint8_t green, uint8_t blue);
static void GpLedAction_CacheRemoteRgb332Frame(const uint8_t xdata *frameData);
static void GpLedAction_CacheRemoteBitmapFrameRgb888(const uint8_t xdata *frameData);
static void GpLedAction_CacheRemoteBitmapLayeredFrame(const uint8_t xdata *frameData, uint16_t length);

static void GpLedAction_ResetAnimationState(void)
{
    uint8_t frameIndex;

    g_gpLedAnimationUploadActive = 0U;
    g_gpLedAnimationLoopEnabled = 0U;
    g_gpLedAnimationActive = 0U;
    g_gpLedAnimationFrameCount = 0U;
    g_gpLedAnimationFrameIndex = 0U;
    g_gpLedAnimationFrameFormat = 0U;
    g_gpLedAnimationFramePending = 0U;
    g_gpLedAnimationFrameIntervalMs = GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS;
    g_gpLedAnimationElapsedMs = 0U;
    for (frameIndex = 0U; frameIndex < GP_MATRIX_ANIMATION_MAX_FRAMES; ++frameIndex)
    {
        g_gpLedAnimationFrameValid[frameIndex] = 0U;
    }
}

static void GpLedAction_RenderAnimationFrame(uint8_t frameIndex)
{
    if ((frameIndex >= g_gpLedAnimationFrameCount) || (g_gpLedAnimationFrameValid[frameIndex] == 0U))
    {
        return;
    }

    GpLedAction_RenderBitmapLayeredFrame(g_gpLedAnimationFrames[frameIndex],
                                         GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES,
                                         g_gpLedDefaultBrightness);
}

static uint8_t GpLedAction_AreAnimationFramesReady(uint8_t frameCount)
{
    uint8_t frameIndex;

    for (frameIndex = 0U; frameIndex < frameCount; ++frameIndex)
    {
        if (g_gpLedAnimationFrameValid[frameIndex] == 0U)
        {
            return 0U;
        }
    }

    return 1U;
}

static uint8_t GpLedAction_GetRequestedOnline(void)
{
    if (g_gpLedManualOverrideEnabled != 0U)
    {
        return g_gpLedManualOverrideOnline;
    }

    return g_gpLedCommOnline;
}

static void GpLedAction_UpdateControlMode(void)
{
    uint8_t nextOnline;

    nextOnline = GpLedAction_GetRequestedOnline();
    if (nextOnline == g_gpLedEffectiveOnline)
    {
        return;
    }

    g_gpLedEffectiveOnline = nextOnline;
    if ((g_gpLedEffectiveOnline == 0U) && (g_gpLedManualOverrideEnabled != 0U))
    {
        GpLedAction_ReleaseRemoteMode();
    }
}

static uint8_t GpLedAction_ScaleColor(uint8_t value, uint8_t brightness)
{
    uint16_t scaled;

    if (brightness >= 255U)
    {
        return value;
    }

    scaled = (uint16_t)value * (uint16_t)brightness;
    return (uint8_t)(scaled / 255U);
}

static uint8_t GpLedAction_PackRgb332(uint8_t red, uint8_t green, uint8_t blue)
{
    return (uint8_t)((red & 0xE0U) | ((green >> 3) & 0x1CU) | ((blue >> 6) & 0x03U));
}

static void GpLedAction_CacheRemoteRgb332Frame(const uint8_t xdata *frameData)
{
    if (frameData == 0)
    {
        return;
    }

    for (g_gpLedTempPixelIndex = 0U; g_gpLedTempPixelIndex < GP_MATRIX_RGB332_FRAME_SIZE; ++g_gpLedTempPixelIndex)
    {
        g_gpLedRemotePatternFrame[g_gpLedTempPixelIndex] = frameData[g_gpLedTempPixelIndex];
    }

    (void)DrawDrv_LoadRemotePatternFrame(g_gpLedRemotePatternFrame, GP_MATRIX_RGB332_FRAME_SIZE);
}

static void GpLedAction_CacheRemoteBitmapFrameRgb888(const uint8_t xdata *frameData)
{
    const uint8_t xdata *bitmapData;
    const uint8_t xdata *colorData;
    uint8_t primaryPacked;
    uint8_t backgroundPacked;

    if (frameData == 0)
    {
        return;
    }

    bitmapData = frameData;
    colorData = &frameData[GP_MATRIX_BITMAP_ROWS_BYTES];
    primaryPacked = GpLedAction_PackRgb332(colorData[0], colorData[1], colorData[2]);
    backgroundPacked = GpLedAction_PackRgb332(colorData[3], colorData[4], colorData[5]);

    for (g_gpLedTempRow = 0U; g_gpLedTempRow < GP_MATRIX_HEIGHT; ++g_gpLedTempRow)
    {
        g_gpLedTempRowBits = (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U];
        g_gpLedTempRowBits |= (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U + 1U] << 8;
        for (g_gpLedTempCol = 0U; g_gpLedTempCol < GP_MATRIX_WIDTH; ++g_gpLedTempCol)
        {
            g_gpLedTempPixelIndex = (uint16_t)g_gpLedTempRow * GP_MATRIX_WIDTH + g_gpLedTempCol;
            if ((g_gpLedTempRowBits & (uint16_t)(0x8000U >> g_gpLedTempCol)) != 0U)
            {
                g_gpLedRemotePatternFrame[g_gpLedTempPixelIndex] = primaryPacked;
            }
            else
            {
                g_gpLedRemotePatternFrame[g_gpLedTempPixelIndex] = backgroundPacked;
            }
        }
    }

    (void)DrawDrv_LoadRemotePatternFrame(g_gpLedRemotePatternFrame, GP_MATRIX_RGB332_FRAME_SIZE);
}

static void GpLedAction_CacheRemoteBitmapLayeredFrame(const uint8_t xdata *frameData, uint16_t length)
{
    const uint8_t xdata *layerData;
    const uint8_t xdata *bitmapData;
    uint8_t totalLayers;
    uint8_t layerPacked;
    uint16_t layerLimit;
    uint16_t layerOffset;

    if ((frameData == 0) || (length < GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES))
    {
        return;
    }

    totalLayers = (uint8_t)(frameData[0] >> 4);
    if ((totalLayers == 0U) || ((uint16_t)totalLayers * GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES > length))
    {
        return;
    }

    for (g_gpLedTempPixelIndex = 0U; g_gpLedTempPixelIndex < GP_MATRIX_RGB332_FRAME_SIZE; ++g_gpLedTempPixelIndex)
    {
        g_gpLedRemotePatternFrame[g_gpLedTempPixelIndex] = (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
    }

    layerLimit = (uint16_t)totalLayers * GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES;
    for (layerOffset = 0U; layerOffset < layerLimit; layerOffset += GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)
    {
        layerData = &frameData[layerOffset];
        bitmapData = &layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES];
        layerPacked = GpLedAction_PackRgb332(layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES
                                                        + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES],
                                             layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES
                                                        + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES + 1U],
                                             layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES
                                                        + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES + 2U]);

        for (g_gpLedTempRow = 0U; g_gpLedTempRow < GP_MATRIX_HEIGHT; ++g_gpLedTempRow)
        {
            g_gpLedTempRowBits = (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U];
            g_gpLedTempRowBits |= (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U + 1U] << 8;
            for (g_gpLedTempCol = 0U; g_gpLedTempCol < GP_MATRIX_WIDTH; ++g_gpLedTempCol)
            {
                if ((g_gpLedTempRowBits & (uint16_t)(0x8000U >> g_gpLedTempCol)) == 0U)
                {
                    continue;
                }

                g_gpLedTempPixelIndex = (uint16_t)g_gpLedTempRow * GP_MATRIX_WIDTH + g_gpLedTempCol;
                g_gpLedRemotePatternFrame[g_gpLedTempPixelIndex] = layerPacked;
            }
        }
    }

    (void)DrawDrv_LoadRemotePatternFrame(g_gpLedRemotePatternFrame, GP_MATRIX_RGB332_FRAME_SIZE);
}

static uint8_t GpLedAction_IsBlackColorTriplet(const uint8_t xdata *colorData)
{
    if (colorData == 0)
    {
        return 0U;
    }

    if ((colorData[0] != 0U) || (colorData[1] != 0U) || (colorData[2] != 0U))
    {
        return 0U;
    }

    return 1U;
}

static uint8_t GpLedAction_IsFullBitmapMask(const uint8_t xdata *bitmapData)
{
    uint8_t byteIndex;

    if (bitmapData == 0)
    {
        return 0U;
    }

    for (byteIndex = 0U; byteIndex < GP_MATRIX_BITMAP_ROWS_BYTES; ++byteIndex)
    {
        if (bitmapData[byteIndex] != 0xFFU)
        {
            return 0U;
        }
    }

    return 1U;
}

static void GpLedAction_CopyCanonicalLayerFrame(uint8_t frameIndex,
                                                const uint8_t xdata *bitmapData,
                                                const uint8_t xdata *colorData)
{
    uint8_t byteIndex;
    uint8_t xdata *dest;

    dest = g_gpLedAnimationFrames[frameIndex];
    dest[0] = 0x10U;
    for (byteIndex = 0U; byteIndex < GP_MATRIX_BITMAP_ROWS_BYTES; ++byteIndex)
    {
        dest[1U + byteIndex] = bitmapData[byteIndex];
    }

    dest[1U + GP_MATRIX_BITMAP_ROWS_BYTES] = colorData[0];
    dest[1U + GP_MATRIX_BITMAP_ROWS_BYTES + 1U] = colorData[1];
    dest[1U + GP_MATRIX_BITMAP_ROWS_BYTES + 2U] = colorData[2];
}

static uint8_t GpLedAction_CanonicalizeLayeredAnimationFrame(uint8_t frameIndex,
                                                             const uint8_t xdata *frameData,
                                                             uint16_t length)
{
    uint8_t totalLayers;
    const uint8_t xdata *layer0;
    const uint8_t xdata *layer1;
    const uint8_t xdata *bitmapData;
    const uint8_t xdata *colorData;

    if (GpLedAction_ValidateLayeredFrame(frameData, length) == 0U)
    {
        return 0U;
    }

    totalLayers = (uint8_t)(frameData[0] >> 4);
    if (totalLayers == 1U)
    {
        bitmapData = frameData + GP_MATRIX_BITMAP_LAYER_HEADER_BYTES;
        colorData = frameData + GP_MATRIX_BITMAP_LAYER_HEADER_BYTES + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES;
        GpLedAction_CopyCanonicalLayerFrame(frameIndex, bitmapData, colorData);
        return 1U;
    }

    if ((totalLayers != 2U) || (length != (uint16_t)(GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES * 2U)))
    {
        return 0U;
    }

    layer0 = frameData;
    layer1 = frameData + GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES;
    if (((layer0[0] & 0x0FU) != 0U) || ((layer1[0] & 0x0FU) != 1U))
    {
        return 0U;
    }

    bitmapData = layer0 + GP_MATRIX_BITMAP_LAYER_HEADER_BYTES;
    colorData = layer0 + GP_MATRIX_BITMAP_LAYER_HEADER_BYTES + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES;
    if ((GpLedAction_IsFullBitmapMask(bitmapData) == 0U) || (GpLedAction_IsBlackColorTriplet(colorData) == 0U))
    {
        return 0U;
    }

    bitmapData = layer1 + GP_MATRIX_BITMAP_LAYER_HEADER_BYTES;
    colorData = layer1 + GP_MATRIX_BITMAP_LAYER_HEADER_BYTES + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES;
    GpLedAction_CopyCanonicalLayerFrame(frameIndex, bitmapData, colorData);
    return 1U;
}

static void GpLedAction_CanonicalizeRgb888AnimationFrame(uint8_t frameIndex,
                                                         const uint8_t xdata *frameData)
{
    const uint8_t xdata *bitmapData;
    const uint8_t xdata *colorData;

    bitmapData = frameData;
    colorData = frameData + GP_MATRIX_BITMAP_ROWS_BYTES;
    GpLedAction_CopyCanonicalLayerFrame(frameIndex, bitmapData, colorData);
}

static uint8_t GpLedAction_StoreCanonicalAnimationFrame(uint8_t frameIndex,
                                                        const uint8_t xdata *frameData,
                                                        uint16_t length)
{
    if (g_gpLedAnimationFrameFormat == GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED)
    {
        return GpLedAction_CanonicalizeLayeredAnimationFrame(frameIndex, frameData, length);
    }

    if (length != GP_MATRIX_BITMAP_RGB888_FRAME_SIZE)
    {
        return 0U;
    }

    GpLedAction_CanonicalizeRgb888AnimationFrame(frameIndex, frameData);
    return 1U;
}

static void GpLedAction_BeginDirectFrame(void)
{
    (void)WS2812DRV_SetDisplayMode(WS2812DRV_MODE_16X16);
    /* Direct-frame paths rewrite the complete 16x16 payload before encoding. */
    WS2812DRV_BeginFrameWrite();
}

static void GpLedAction_EndDirectFrame(void)
{
    uint8_t retryCount;

    WS2812DRV_EndFrameWrite();
    /* Retry encoding if a previous PWM swap hasn't completed yet. */
    for (retryCount = 0U; retryCount < 3U; ++retryCount)
    {
        WS2812DRV_EncodeAllRows();
        if (WS2812DRV_IsPwmSwapPending() != 0U)
        {
            continue;
        }
        break;
    }
    g_gpLedRemoteActive = 1U;
    g_gpLedDirectFrameActive = 1U;
}

static void GpLedAction_RenderRgb332Frame(const uint8_t xdata *frameData, uint8_t brightness)
{
    GpLedAction_BeginDirectFrame();
    for (g_gpLedTempRow = 0U; g_gpLedTempRow < GP_MATRIX_HEIGHT; ++g_gpLedTempRow)
    {
        g_gpLedTempRowMapped = (uint8_t)((GP_MATRIX_HEIGHT - 1U) - g_gpLedTempRow);
        for (g_gpLedTempCol = 0U; g_gpLedTempCol < GP_MATRIX_WIDTH; ++g_gpLedTempCol)
        {
            g_gpLedTempPixelIndex = (uint16_t)g_gpLedTempRowMapped * GP_MATRIX_WIDTH + g_gpLedTempCol;
            g_gpLedTempR = (uint8_t)((frameData[g_gpLedTempPixelIndex] >> 5) & 0x07U);
            g_gpLedTempG = (uint8_t)((frameData[g_gpLedTempPixelIndex] >> 2) & 0x07U);
            g_gpLedTempB = (uint8_t)(frameData[g_gpLedTempPixelIndex] & 0x03U);
            g_gpLedTempR = (uint8_t)((uint16_t)g_gpLedTempR * 255U / 7U);
            g_gpLedTempG = (uint8_t)((uint16_t)g_gpLedTempG * 255U / 7U);
            g_gpLedTempB = (uint8_t)((uint16_t)g_gpLedTempB * 255U / 3U);
            g_gpLedTempR = GpLedAction_ScaleColor(g_gpLedTempR, brightness);
            g_gpLedTempG = GpLedAction_ScaleColor(g_gpLedTempG, brightness);
            g_gpLedTempB = GpLedAction_ScaleColor(g_gpLedTempB, brightness);
            WS2812DRV_SetPixelRgbFast(g_gpLedTempRow, g_gpLedTempCol, g_gpLedTempR, g_gpLedTempG, g_gpLedTempB);
        }
    }
    GpLedAction_EndDirectFrame();
}

static void GpLedAction_RenderBitmapFrameRgb888(const uint8_t xdata *frameData, uint8_t brightness)
{
    const uint8_t xdata *bitmapData;
    const uint8_t xdata *colorData;

    bitmapData = frameData;
    colorData = &frameData[GP_MATRIX_BITMAP_ROWS_BYTES];
    g_gpLedPrimaryR = GpLedAction_ScaleColor(colorData[0], brightness);
    g_gpLedPrimaryG = GpLedAction_ScaleColor(colorData[1], brightness);
    g_gpLedPrimaryB = GpLedAction_ScaleColor(colorData[2], brightness);
    g_gpLedBackgroundR = GpLedAction_ScaleColor(colorData[3], brightness);
    g_gpLedBackgroundG = GpLedAction_ScaleColor(colorData[4], brightness);
    g_gpLedBackgroundB = GpLedAction_ScaleColor(colorData[5], brightness);

    GpLedAction_BeginDirectFrame();
    for (g_gpLedTempRow = 0U; g_gpLedTempRow < GP_MATRIX_HEIGHT; ++g_gpLedTempRow)
    {
        g_gpLedTempRowMapped = (uint8_t)((GP_MATRIX_HEIGHT - 1U) - g_gpLedTempRow);
        g_gpLedTempRowBits = (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U];
        g_gpLedTempRowBits |= (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U + 1U] << 8;
        for (g_gpLedTempCol = 0U; g_gpLedTempCol < GP_MATRIX_WIDTH; ++g_gpLedTempCol)
        {
            if ((g_gpLedTempRowBits & (uint16_t)(0x8000U >> g_gpLedTempCol)) != 0U)
            {
                g_gpLedTempR = g_gpLedPrimaryR;
                g_gpLedTempG = g_gpLedPrimaryG;
                g_gpLedTempB = g_gpLedPrimaryB;
            }
            else
            {
                g_gpLedTempR = g_gpLedBackgroundR;
                g_gpLedTempG = g_gpLedBackgroundG;
                g_gpLedTempB = g_gpLedBackgroundB;
            }
            WS2812DRV_SetPixelRgbFast(g_gpLedTempRowMapped, g_gpLedTempCol, g_gpLedTempR, g_gpLedTempG, g_gpLedTempB);
        }
    }
    GpLedAction_EndDirectFrame();
}

static void GpLedAction_RenderBitmapLayeredFrame(const uint8_t xdata *frameData, uint16_t length, uint8_t brightness)
{
    const uint8_t xdata *layerData;
    uint8_t totalLayers;
    uint8_t layerSeqInfo;
    const uint8_t xdata *bitmapData;
    uint8_t layerR;
    uint8_t layerG;
    uint8_t layerB;
    uint8_t isFirstLayer;
    uint16_t layerOffset;
    uint16_t layerLimit;

    /* First byte of the first layer carries total count in the high nibble. */
    totalLayers = (uint8_t)(frameData[0] >> 4);
    if ((totalLayers == 0U) || ((uint16_t)totalLayers * GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES > length))
    {
        return;
    }

    layerLimit = (uint16_t)totalLayers * GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES;

    GpLedAction_BeginDirectFrame();
    /* Paint each layer in order. Layer 0 (backmost) sets black for any uncovered pixels.
       Higher layers only overwrite on bitmap=1, preserving earlier layer colors underneath. */
    for (layerOffset = 0U; layerOffset < layerLimit; layerOffset += GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)
    {
        layerData = &frameData[layerOffset];
        layerSeqInfo = layerData[0];
        if ((uint8_t)(layerSeqInfo & 0x0FU) >= totalLayers)
        {
            continue;
        }

        isFirstLayer = (uint8_t)(layerOffset == 0U);
        bitmapData = &layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES];
        layerR = GpLedAction_ScaleColor(layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES
                                                + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES], brightness);
        layerG = GpLedAction_ScaleColor(layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES
                                                + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES + 1U], brightness);
        layerB = GpLedAction_ScaleColor(layerData[GP_MATRIX_BITMAP_LAYER_HEADER_BYTES
                                                + GP_MATRIX_BITMAP_LAYER_BITMAP_BYTES + 2U], brightness);

        for (g_gpLedTempRow = 0U; g_gpLedTempRow < GP_MATRIX_HEIGHT; ++g_gpLedTempRow)
        {
            g_gpLedTempRowMapped = (uint8_t)((GP_MATRIX_HEIGHT - 1U) - g_gpLedTempRow);
            g_gpLedTempRowBits = (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U];
            g_gpLedTempRowBits |= (uint16_t)bitmapData[(uint16_t)g_gpLedTempRow * 2U + 1U] << 8;
            for (g_gpLedTempCol = 0U; g_gpLedTempCol < GP_MATRIX_WIDTH; ++g_gpLedTempCol)
            {
                if ((g_gpLedTempRowBits & (uint16_t)(0x8000U >> g_gpLedTempCol)) != 0U)
                {
                    WS2812DRV_SetPixelRgbFast(g_gpLedTempRowMapped, g_gpLedTempCol, layerR, layerG, layerB);
                }
                else if (isFirstLayer != 0U)
                {
                    WS2812DRV_SetPixelRgbFast(g_gpLedTempRowMapped, g_gpLedTempCol, 0x00U, 0x00U, 0x00U);
                }
            }
        }
    }

    GpLedAction_EndDirectFrame();
}

static uint8_t GpLedAction_ValidateLayeredFrame(const uint8_t xdata *frameData, uint16_t length)
{
    uint8_t totalLayers;
    uint8_t layerSeqInfo;
    uint8_t layerIndex;
    uint16_t layerOffset;
    uint16_t layerLimit;

    if ((length < GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)
        || ((length % GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES) != 0U))
    {
        return 0U;
    }

    totalLayers = (uint8_t)(frameData[0] >> 4);
    if ((totalLayers == 0U) || (totalLayers > GP_MATRIX_BITMAP_LAYERED_MAX_COLORS))
    {
        return 0U;
    }

    if ((uint16_t)totalLayers * GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES != length)
    {
        return 0U;
    }

    /* Verify each layer's seq field: all must share the same total, indices must be 0..total-1. */
    layerLimit = (uint16_t)totalLayers * GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES;
    for (layerOffset = 0U; layerOffset < layerLimit; layerOffset += GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)
    {
        layerSeqInfo = frameData[layerOffset];
        layerIndex = (uint8_t)(layerSeqInfo & 0x0FU);
        if ((layerSeqInfo >> 4) != totalLayers)
        {
            return 0U;
        }
        if (layerIndex >= totalLayers)
        {
            return 0U;
        }
    }

    return 1U;
}

static uint8_t GpLedAction_IsDisplayProfileValid(const GpLedDisplayProfile *profile)
{
    if (profile->effect > kGpMatrixEffectGradientReveal)
    {
        return 0U;
    }
    if (profile->direction > kGpMatrixDirectionRotateCcw90)
    {
        return 0U;
    }
    if (profile->colorMode > kGpMatrixColorModeGradient)
    {
        return 0U;
    }
    if ((profile->content != kGpMatrixActionContentSolid)
        && (profile->content != kGpMatrixActionContentPattern)
        && (profile->content != kGpMatrixActionContentGlyph))
    {
        return 0U;
    }

    return 1U;
}

static void GpLedAction_LoadProfileFromAction(const GpMatrixActionPayload xdata *payload,
                                              GpLedDisplayProfile xdata *profile)
{
    profile->version = GP_LED_PROFILE_VERSION_V1;
    profile->profileFlags = 0U;
    profile->source = payload->source;
    profile->content = payload->content;
    profile->effect = payload->effect;
    profile->direction = payload->direction;
    profile->colorMode = payload->color_mode;
    profile->brightness = payload->brightness;
    profile->primaryR = payload->primary_r;
    profile->primaryG = payload->primary_g;
    profile->primaryB = payload->primary_b;
    profile->secondaryR = payload->secondary_r;
    profile->secondaryG = payload->secondary_g;
    profile->secondaryB = payload->secondary_b;
    profile->patternId = payload->pattern_id;
    profile->glyphId = payload->glyph_id;
    profile->scrollStep = payload->scroll_step;
    profile->animStep = payload->anim_step;
    profile->gradientSpan = payload->gradient_span;
    profile->actionFlags = payload->flags;
    profile->frameIntervalMs = GP_MATRIX_READ_LE16(&payload->frame_interval_ms_lo);
    profile->timelineDurationMs = GP_MATRIX_READ_LE16(&payload->timeline_duration_ms_lo);
    profile->timelineRepeatDelayMs = GP_MATRIX_READ_LE16(&payload->timeline_repeat_delay_ms_lo);
    profile->timelineRepeatCount = payload->timeline_repeat_count;
    profile->timelinePath = payload->timeline_path;
    profile->animationFlags = payload->animation_flags;
    profile->applyFlags = payload->apply_flags;

    if (profile->frameIntervalMs == 0U)
    {
        profile->frameIntervalMs = DRAWDRV_FRAME_INTERVAL_MS_DEFAULT;
    }
    if (profile->timelinePath > GP_LED_TIMELINE_PATH_BREATH_CURVE)
    {
        profile->timelinePath = GP_LED_TIMELINE_PATH_LINEAR;
    }
    if (profile->applyFlags == 0U)
    {
        profile->applyFlags = GP_LED_PROFILE_FLAG_APPLY_PATTERN | GP_LED_PROFILE_FLAG_APPLY_GLYPH;
    }
}

static void GpLedAction_ProfileToRenderConfig(const GpLedDisplayProfile xdata *profile,
                                              DrawDrv_RenderConfig_t *renderCfg)
{
    DrawDrv_GetRenderConfig(renderCfg);

    renderCfg->fgR = profile->primaryR;
    renderCfg->fgG = profile->primaryG;
    renderCfg->fgB = profile->primaryB;

    renderCfg->bgR = 0U;
    renderCfg->bgG = 0U;
    renderCfg->bgB = 0U;
    if ((profile->actionFlags & GP_MATRIX_ACTION_FLAG_USE_SECONDARY) != 0U)
    {
        renderCfg->bgR = profile->secondaryR;
        renderCfg->bgG = profile->secondaryG;
        renderCfg->bgB = profile->secondaryB;
    }

    renderCfg->brightness = profile->brightness;
    renderCfg->colorMode = (DrawDrv_ColorMode_t)profile->colorMode;
    renderCfg->direction = (DrawDrv_Direction_t)profile->direction;
    renderCfg->effect = (DrawDrv_Effect_t)profile->effect;
    renderCfg->useGradient = (uint8_t)(((profile->actionFlags & GP_MATRIX_ACTION_FLAG_USE_SECONDARY) != 0U)
                                       || (profile->colorMode == kGpMatrixColorModeGradient));
    renderCfg->gradientSpan = (profile->gradientSpan == 0U) ? 96U : profile->gradientSpan;
    renderCfg->scrollStep = (profile->scrollStep == 0U) ? 1U : profile->scrollStep;
    renderCfg->animStep = (profile->animStep == 0U) ? 1U : profile->animStep;
    renderCfg->frameIntervalMs = DrawDrv_NormalizeFrameIntervalMs(profile->frameIntervalMs);
    renderCfg->timelineDurationMs = profile->timelineDurationMs;
    renderCfg->timelineRepeatDelayMs = profile->timelineRepeatDelayMs;
    renderCfg->timelineRepeatCount = profile->timelineRepeatCount;
    renderCfg->timelinePath = (DrawDrv_TimelinePath_t)profile->timelinePath;
}

void GpLedAction_Init(void)
{
    g_gpLedRemoteActive = 0U;
    g_gpLedDirectFrameActive = 0U;
    g_gpLedCommOnline = 0U;
    g_gpLedManualOverrideEnabled = 0U;
    g_gpLedManualOverrideOnline = 0U;
    g_gpLedEffectiveOnline = 0U;
    g_gpLedDebugFlowEnabled = 0U;
    g_gpLedDebugFlowIndex = 0U;
    g_gpLedCommActiveTicks = 0U;
    g_gpLedDebugFlowTicks = 0U;
    GpLedAction_ResetAnimationState();
}

void GpLedAction_SetBrightness(uint8_t brightness)
{
    DrawDrv_RenderConfig_t *renderCfg;

    g_gpLedDefaultBrightness = brightness;
    renderCfg = DrawDrv_GetRenderConfigStorage();
    renderCfg->brightness = brightness;
    DrawDrv_SetRenderConfig(renderCfg);
    if (g_gpLedDirectFrameActive == 0U)
    {
        DrawDrv_RequestRebuild();
        return;
    }

    if (g_gpLedAnimationActive != 0U)
    {
        GpLedAction_RenderAnimationFrame(g_gpLedAnimationFrameIndex);
    }
}

void GpLedAction_ReleaseRemoteMode(void)
{
    g_gpLedRemoteActive = 0U;
    g_gpLedDirectFrameActive = 0U;
    g_gpLedDebugFlowEnabled = 0U;
    g_gpLedDebugFlowTicks = 0U;
    GpLedAction_ResetAnimationState();
    PORT2_ClearDebugLeds();
    DrawDrv_RequestRebuild();
}

void GpLedAction_NotifyCommunicationActive(void)
{
    g_gpLedCommOnline = 1U;
    g_gpLedCommActiveTicks = GP_LED_COMM_ACTIVE_TIMEOUT_TICKS;
    GpLedAction_UpdateControlMode();
}

void GpLedAction_Task10ms(void)
{
    if (g_gpLedCommActiveTicks != 0U)
    {
        g_gpLedCommActiveTicks--;
        if (g_gpLedCommActiveTicks == 0U)
        {
            g_gpLedCommOnline = 0U;
            GpLedAction_UpdateControlMode();
        }
    }

    if (g_gpLedDebugFlowEnabled != 0U)
    {
        g_gpLedDebugFlowTicks++;
        if (g_gpLedDebugFlowTicks >= GP_LED_DEBUG_FLOW_INTERVAL_TICKS)
        {
            g_gpLedDebugFlowTicks = 0U;
            PORT2_SetDebugLedDigit(g_gpLedDebugFlowIndex);
            g_gpLedDebugFlowIndex = (uint8_t)((g_gpLedDebugFlowIndex + 1U) % (GP_MATRIX_DEBUG_LED_MAX_INDEX + 1U));
        }
    }
}

void GpLedAction_Tick1ms(void)
{
    /* Drive frame stepping from the 1 ms scheduler so host-provided intervals stay in millisecond units.
       Heavy work (rendering + PWM encoding) is deferred to the main loop via g_gpLedAnimationFramePending
       to avoid blocking the Timer0 ISR and starving the Timer1 scan DMA ISR. */
    if ((g_gpLedAnimationActive == 0U) || (g_gpLedAnimationFrameCount <= 1U))
    {
        return;
    }

    if ((uint16_t)(g_gpLedAnimationElapsedMs + 1U) < g_gpLedAnimationFrameIntervalMs)
    {
        g_gpLedAnimationElapsedMs++;
        return;
    }

    g_gpLedAnimationElapsedMs = 0U;
    g_gpLedAnimationFrameIndex++;
    if (g_gpLedAnimationFrameIndex >= g_gpLedAnimationFrameCount)
    {
        if (g_gpLedAnimationLoopEnabled == 0U)
        {
            g_gpLedAnimationFrameIndex = (uint8_t)(g_gpLedAnimationFrameCount - 1U);
            g_gpLedAnimationActive = 0U;
            return;
        }

        g_gpLedAnimationFrameIndex = 0U;
    }

    /* Defer the actual rendering to the main loop so the ISR stays short. */
    g_gpLedAnimationFramePending = 1U;
}

void GpLedAction_RenderPendingAnimationFrame(void)
{
    if (g_gpLedAnimationFramePending == 0U)
    {
        return;
    }

    g_gpLedAnimationFramePending = 0U;
    GpLedAction_RenderAnimationFrame(g_gpLedAnimationFrameIndex);
}

void GpLedAction_ToggleModeOverride(void)
{
    g_gpLedManualOverrideEnabled = 1U;
    g_gpLedManualOverrideOnline = (uint8_t)(g_gpLedEffectiveOnline == 0U);
    GpLedAction_UpdateControlMode();
}

uint8_t GpLedAction_IsRemoteModeActive(void)
{
    return g_gpLedRemoteActive;
}

uint8_t GpLedAction_IsOnlineModeActive(void)
{
    return g_gpLedEffectiveOnline;
}

uint8_t GpLedAction_IsHostControlEnabled(void)
{
    return g_gpLedEffectiveOnline;
}

GpLedControlMode GpLedAction_GetControlMode(void)
{
    if (g_gpLedEffectiveOnline != 0U)
    {
        return kGpLedControlModeOnline;
    }

    return kGpLedControlModeOffline;
}

uint8_t GpLedAction_ShouldBypassDrawScheduler(void)
{
    return g_gpLedDirectFrameActive;
}

GpMatrixStatusCode GpLedAction_SetDebugLedFlow(uint8_t enable)
{
    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if (enable != 0U)
    {
        g_gpLedDebugFlowEnabled = 1U;
        g_gpLedDebugFlowIndex = 0U;
        g_gpLedDebugFlowTicks = 0U;
        PORT2_SetDebugLedDigit(g_gpLedDebugFlowIndex);
        g_gpLedDebugFlowIndex = 1U;
        return kGpMatrixStatusOk;
    }

    g_gpLedDebugFlowEnabled = 0U;
    g_gpLedDebugFlowTicks = 0U;
    PORT2_ClearDebugLeds();
    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_BeginAnimationUpload(uint8_t frameFormat,
                                                    uint8_t frameCount,
                                                    uint16_t frameIntervalMs,
                                                    uint8_t flags)
{
    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((frameFormat != GP_MATRIX_PAYLOAD_FORMAT_BITMAP_RGB888)
        && (frameFormat != GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED))
    {
        return kGpMatrixStatusUnsupported;
    }
    if ((frameCount == 0U) || (frameCount > GP_MATRIX_ANIMATION_MAX_FRAMES))
    {
        return kGpMatrixStatusUnsupported;
    }

    GpLedAction_ResetAnimationState();
    g_gpLedAnimationUploadActive = 1U;
    g_gpLedAnimationFrameFormat = frameFormat;
    g_gpLedAnimationFrameCount = frameCount;
    g_gpLedAnimationFrameIntervalMs = frameIntervalMs;
    if (g_gpLedAnimationFrameIntervalMs < GP_MATRIX_ANIMATION_INTERVAL_MS_MIN)
    {
        g_gpLedAnimationFrameIntervalMs = GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS;
    }
    g_gpLedAnimationElapsedMs = 0U;
    g_gpLedAnimationLoopEnabled = (uint8_t)((flags & GP_MATRIX_ANIMATION_FLAG_LOOP) != 0U);
    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_StoreAnimationFrame(uint8_t frameIndex,
                                                   const uint8_t xdata *frameData,
                                                   uint16_t length)
{
    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((g_gpLedAnimationUploadActive == 0U)
        || (frameIndex >= g_gpLedAnimationFrameCount)
        || (frameData == 0))
    {
        return kGpMatrixStatusBadSequence;
    }

    if (g_gpLedAnimationFrameFormat == GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED)
    {
        if ((length < GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES)
            || (length > GP_MATRIX_ANIMATION_LAYERED_MAX_FRAME_SIZE)
            || (GpLedAction_ValidateLayeredFrame(frameData, length) == 0U))
        {
            return kGpMatrixStatusBadSequence;
        }
    }
    else
    {
        if (length != GP_MATRIX_BITMAP_RGB888_FRAME_SIZE)
        {
            return kGpMatrixStatusBadSequence;
        }
    }

    if (GpLedAction_StoreCanonicalAnimationFrame(frameIndex, frameData, length) == 0U)
    {
        return kGpMatrixStatusUnsupported;
    }

    g_gpLedAnimationFrameValid[frameIndex] = 1U;
    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_CommitAnimation(uint8_t frameCount)
{
    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((g_gpLedAnimationUploadActive == 0U)
        || (frameCount != g_gpLedAnimationFrameCount)
        || (GpLedAction_AreAnimationFramesReady(frameCount) == 0U))
    {
        return kGpMatrixStatusBadSequence;
    }

    g_gpLedAnimationUploadActive = 0U;
    g_gpLedAnimationActive = 1U;
    g_gpLedAnimationFrameIndex = 0U;
    g_gpLedAnimationElapsedMs = 0U;
    g_gpLedRemoteActive = 1U;
    g_gpLedDirectFrameActive = 1U;
    GpLedAction_RenderAnimationFrame(0U);
    return kGpMatrixStatusOk;
}

static GpMatrixStatusCode GpLedAction_ApplyDisplayProfileCore(const GpLedDisplayProfile xdata *profile,
                                                              uint8_t requireHostControl,
                                                              uint8_t markRemoteActive)
{
    DrawDrv_RenderConfig_t *renderCfg;

    if (profile == 0)
    {
        return kGpMatrixStatusInternalError;
    }

    if ((requireHostControl != 0U) && (GpLedAction_IsHostControlEnabled() == 0U))
    {
        return kGpMatrixStatusBusy;
    }

    if ((profile->actionFlags & GP_MATRIX_ACTION_FLAG_REMOTE_RELEASE) != 0U)
    {
        GpLedAction_ReleaseRemoteMode();
        return kGpMatrixStatusOk;
    }

    if (GpLedAction_IsDisplayProfileValid(profile) == 0U)
    {
        return kGpMatrixStatusUnsupported;
    }

    /* Keep animation/mode switches in one control entry to avoid path drift between local and remote routes. */
    renderCfg = DrawDrv_GetRenderConfigStorage();
    GpLedAction_ProfileToRenderConfig(profile, renderCfg);
    if (profile->content == kGpMatrixActionContentSolid)
    {
        renderCfg->contentType = DRAWDRV_CONTENT_SOLID;
        DrawDrv_SetRenderConfig(renderCfg);
    }
    else if (profile->content == kGpMatrixActionContentPattern)
    {
        renderCfg->contentType = DRAWDRV_CONTENT_PATTERN;
        DrawDrv_SetRenderConfig(renderCfg);
        if ((profile->applyFlags & GP_LED_PROFILE_FLAG_APPLY_PATTERN) != 0U)
        {
            DrawDrv_SetImageIndex(profile->patternId);
        }
    }
    else
    {
        renderCfg->contentType = DRAWDRV_CONTENT_GLYPH;
        DrawDrv_SetRenderConfig(renderCfg);
        if ((profile->actionFlags & GP_MATRIX_ACTION_FLAG_USE_UPLOADED_GLYPHS) != 0U)
        {
            if (DrawDrv_SelectCustomTextGlyphRows(1U) == 0U)
            {
                return kGpMatrixStatusBadSequence;
            }
        }
        else
        {
            (void)DrawDrv_SelectCustomTextGlyphRows(0U);
        }
        if (((profile->applyFlags & GP_LED_PROFILE_FLAG_APPLY_GLYPH) != 0U)
            && (DrawDrv_SetTextDisplayGlyph(profile->glyphId) == 0U))
        {
            return kGpMatrixStatusBadLength;
        }
    }

    if (markRemoteActive != 0U)
    {
        g_gpLedRemoteActive = 1U;
    }
    g_gpLedDirectFrameActive = 0U;
    DrawDrv_RequestRebuild();

    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_ApplyDisplayProfile(const GpLedDisplayProfile xdata *profile)
{
    return GpLedAction_ApplyDisplayProfileCore(profile, 1U, 1U);
}

GpMatrixStatusCode GpLedAction_ApplyLocalDisplayProfile(const GpLedDisplayProfile xdata *profile)
{
    if (GpLedAction_IsHostControlEnabled() != 0U)
    {
        return kGpMatrixStatusBusy;
    }

    return GpLedAction_ApplyDisplayProfileCore(profile, 0U, 0U);
}

GpMatrixStatusCode GpLedAction_ApplyAction(const GpMatrixActionPayload xdata *payload)
{
    if (payload == 0)
    {
        return kGpMatrixStatusInternalError;
    }

    GpLedAction_LoadProfileFromAction(payload, &g_gpLedProfile);
    return GpLedAction_ApplyDisplayProfile(&g_gpLedProfile);
}

GpMatrixStatusCode GpLedAction_SyncClockTime(const GpMatrixTimeSyncPayload xdata *payload)
{
    if (RtcClock_SyncTime(payload) == 0U)
    {
        return kGpMatrixStatusUnsupported;
    }

    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_ApplyFrameRgb332(const uint8_t xdata *frameData, uint16_t length, GpMatrixMode mode)
{
    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((frameData == 0) || (length < GP_MATRIX_RGB332_FRAME_SIZE))
    {
        return kGpMatrixStatusBadLength;
    }

    if (mode != kGpMatrixModeSolidFrame)
    {
        return kGpMatrixStatusUnsupported;
    }

    g_gpLedAnimationActive = 0U;
    GpLedAction_CacheRemoteRgb332Frame(frameData);
    if (DrawDrv_IsRemotePatternActive() != 0U)
    {
        GpLedAction_ReleaseRemoteMode();
        DrawDrv_RequestRebuild();
        return kGpMatrixStatusOk;
    }

    GpLedAction_RenderRgb332Frame(frameData, g_gpLedDefaultBrightness);

    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_ApplyFrameBitmapRgb888(const uint8_t xdata *frameData,
                                                      uint16_t length,
                                                      GpMatrixMode mode)
{
    const uint8_t xdata *colorData;
    GpMatrixStatusCode status;

    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((frameData == 0) || (length < GP_MATRIX_BITMAP_RGB888_FRAME_SIZE))
    {
        return kGpMatrixStatusBadLength;
    }

    if (mode != kGpMatrixModeSolidFrame)
    {
        return kGpMatrixStatusUnsupported;
    }

    g_gpLedAnimationActive = 0U;
    GpLedAction_CacheRemoteBitmapFrameRgb888(frameData);
    if (DrawDrv_IsRemotePatternActive() != 0U)
    {
        GpLedAction_ReleaseRemoteMode();
        DrawDrv_RequestRebuild();
        colorData = &frameData[GP_MATRIX_BITMAP_ROWS_BYTES];
        printf("[GP_DRAW] bmp-cache len=%u fg=%02X%02X%02X bg=%02X%02X%02X\r\n",
               (unsigned int)length,
               (unsigned int)colorData[0],
               (unsigned int)colorData[1],
               (unsigned int)colorData[2],
               (unsigned int)colorData[3],
               (unsigned int)colorData[4],
               (unsigned int)colorData[5]);
        return kGpMatrixStatusOk;
    }

    status = GpLedAction_BeginAnimationUpload(GP_MATRIX_PAYLOAD_FORMAT_BITMAP_RGB888,
                                              1U,
                                              GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS,
                                              GP_MATRIX_ANIMATION_FLAG_LOOP);
    if (status != kGpMatrixStatusOk)
    {
        return status;
    }

    status = GpLedAction_StoreAnimationFrame(0U, frameData, length);
    if (status != kGpMatrixStatusOk)
    {
        if (status == kGpMatrixStatusUnsupported)
        {
            GpLedAction_ResetAnimationState();
            GpLedAction_RenderBitmapFrameRgb888(frameData, g_gpLedDefaultBrightness);
            colorData = &frameData[GP_MATRIX_BITMAP_ROWS_BYTES];
            printf("[GP_DRAW] bmp-direct len=%u fg=%02X%02X%02X bg=%02X%02X%02X bri=%u cols=%u\r\n",
                   (unsigned int)length,
                   (unsigned int)colorData[0],
                   (unsigned int)colorData[1],
                   (unsigned int)colorData[2],
                   (unsigned int)colorData[3],
                   (unsigned int)colorData[4],
                   (unsigned int)colorData[5],
                   (unsigned int)g_gpLedDefaultBrightness,
                   (unsigned int)WS2812DRV_GetActiveCols());
            return kGpMatrixStatusOk;
        }

        return status;
    }

    status = GpLedAction_CommitAnimation(1U);
    if (status != kGpMatrixStatusOk)
    {
        return status;
    }

    colorData = &frameData[GP_MATRIX_BITMAP_ROWS_BYTES];
    printf("[GP_DRAW] bmp len=%u fg=%02X%02X%02X bg=%02X%02X%02X bri=%u cols=%u\r\n",
           (unsigned int)length,
           (unsigned int)colorData[0],
           (unsigned int)colorData[1],
           (unsigned int)colorData[2],
           (unsigned int)colorData[3],
           (unsigned int)colorData[4],
           (unsigned int)colorData[5],
           (unsigned int)g_gpLedDefaultBrightness,
           (unsigned int)WS2812DRV_GetActiveCols());

    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_ApplyFrameBitmapLayered(const uint8_t xdata *frameData,
                                                        uint16_t length,
                                                        GpMatrixMode mode)
{
    uint8_t totalLayers;

    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((frameData == 0) || (length < GP_MATRIX_BITMAP_LAYER_TOTAL_BYTES))
    {
        return kGpMatrixStatusBadLength;
    }

    if (mode != kGpMatrixModeSolidFrame)
    {
        return kGpMatrixStatusUnsupported;
    }

    if (GpLedAction_ValidateLayeredFrame(frameData, length) == 0U)
    {
        return kGpMatrixStatusBadSequence;
    }

    /* Stop any running animation so Tick1ms won't overwrite this static frame. */
    g_gpLedAnimationActive = 0U;
    GpLedAction_CacheRemoteBitmapLayeredFrame(frameData, length);
    totalLayers = (uint8_t)(frameData[0] >> 4);
    if (DrawDrv_IsRemotePatternActive() != 0U)
    {
        GpLedAction_ReleaseRemoteMode();
        DrawDrv_RequestRebuild();
        printf("[GP_DRAW] layered-cache len=%u layers=%u\r\n",
               (unsigned int)length,
               (unsigned int)totalLayers);
        return kGpMatrixStatusOk;
    }

    GpLedAction_RenderBitmapLayeredFrame(frameData, length, g_gpLedDefaultBrightness);

    printf("[GP_DRAW] layered len=%u layers=%u bri=%u cols=%u\r\n",
           (unsigned int)length,
           (unsigned int)totalLayers,
           (unsigned int)g_gpLedDefaultBrightness,
           (unsigned int)WS2812DRV_GetActiveCols());

    return kGpMatrixStatusOk;
}

GpMatrixStatusCode GpLedAction_ApplyGlyphRows(const uint8_t xdata *glyphData,
                                              uint16_t length,
                                              uint8_t glyphCount,
                                              uint8_t glyphWidth,
                                              uint8_t glyphSpacing)
{
    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((glyphData == 0) || (glyphCount == 0U) || (glyphWidth != GP_MATRIX_WIDTH))
    {
        return kGpMatrixStatusUnsupported;
    }
    if ((glyphSpacing != 0U) && (glyphSpacing != LOCALDISPLAY_SCROLL_GLYPH_SPACING))
    {
        return kGpMatrixStatusUnsupported;
    }
    if (length < GP_MATRIX_GLYPH_ROWS_SIZE)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpLedAnimationActive = 0U;
    if (DrawDrv_LoadCustomTextGlyphRows(glyphData, length, glyphCount, glyphWidth, glyphSpacing) == 0U)
    {
        return kGpMatrixStatusBadLength;
    }

    return kGpMatrixStatusOk;
}