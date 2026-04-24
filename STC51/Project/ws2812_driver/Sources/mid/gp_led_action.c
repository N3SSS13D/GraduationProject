#include "config.h"
#include "draw_drv.h"
#include "gp_led_action.h"
#include "test_image.h"
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
static uint16_t g_gpLedAnimationFrameIntervalMs = GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS;
static uint16_t g_gpLedAnimationElapsedMs = 0U;
static uint16_t g_gpLedCommActiveTicks = 0U;
static uint16_t g_gpLedDebugFlowTicks = 0U;
/* Direct-frame rendering shares a single scratch area to reduce EDATA usage. */
static DrawDrv_RenderConfig_t xdata g_gpLedRenderCfg;
/* Compact remote animations keep up to 24 bitmap frames for loop playback. */
static uint8_t xdata g_gpLedAnimationFrames[GP_MATRIX_ANIMATION_MAX_FRAMES][GP_MATRIX_BITMAP_RGB888_FRAME_SIZE];
static uint8_t xdata g_gpLedAnimationFrameValid[GP_MATRIX_ANIMATION_MAX_FRAMES];
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

static void GpLedAction_RenderBitmapFrameRgb888(const uint8_t xdata *frameData, uint8_t brightness);

static void GpLedAction_ResetAnimationState(void)
{
    uint8_t frameIndex;

    g_gpLedAnimationUploadActive = 0U;
    g_gpLedAnimationLoopEnabled = 0U;
    g_gpLedAnimationActive = 0U;
    g_gpLedAnimationFrameCount = 0U;
    g_gpLedAnimationFrameIndex = 0U;
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

    GpLedAction_RenderBitmapFrameRgb888(g_gpLedAnimationFrames[frameIndex], g_gpLedDefaultBrightness);
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

static void GpLedAction_BeginDirectFrame(void)
{
    (void)WS2812DRV_SetDisplayMode(WS2812DRV_MODE_16X16);
    WS2812DRV_ClearImage();
    WS2812DRV_BeginFrameWrite();
}

static void GpLedAction_EndDirectFrame(void)
{
    WS2812DRV_EndFrameWrite();
    WS2812DRV_EncodeAllRows();
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

static void GpLedAction_RenderGlyphFrame(const uint8_t xdata *glyphData)
{
    GpLedAction_BeginDirectFrame();
    for (g_gpLedTempRow = 0U; g_gpLedTempRow < GP_MATRIX_HEIGHT; ++g_gpLedTempRow)
    {
        g_gpLedTempRowMapped = (uint8_t)((GP_MATRIX_HEIGHT - 1U) - g_gpLedTempRow);
        g_gpLedTempRowBits = (uint16_t)glyphData[(uint16_t)g_gpLedTempRow * 2U];
        g_gpLedTempRowBits |= (uint16_t)glyphData[(uint16_t)g_gpLedTempRow * 2U + 1U] << 8;
        for (g_gpLedTempCol = 0U; g_gpLedTempCol < GP_MATRIX_WIDTH; ++g_gpLedTempCol)
        {
            if ((g_gpLedTempRowBits & (uint16_t)(0x8000U >> g_gpLedTempCol)) != 0U)
            {
                WS2812DRV_SetPixelRgbFast(g_gpLedTempRowMapped, g_gpLedTempCol, 0xFFU, 0xFFU, 0xFFU);
            }
            else
            {
                WS2812DRV_SetPixelRgbFast(g_gpLedTempRowMapped, g_gpLedTempCol, 0x00U, 0x00U, 0x00U);
            }
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

static uint8_t GpLedAction_IsRenderActionValid(const GpMatrixActionPayload *payload)
{
    if (payload->effect > kGpMatrixEffectColorCycle)
    {
        return 0U;
    }
    if (payload->direction > kGpMatrixDirectionRotateCcw90)
    {
        return 0U;
    }
    if (payload->color_mode > kGpMatrixColorModeGradient)
    {
        return 0U;
    }

    return 1U;
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
    g_gpLedDefaultBrightness = brightness;
    DrawDrv_GetRenderConfig(&g_gpLedRenderCfg);
    g_gpLedRenderCfg.brightness = brightness;
    DrawDrv_SetRenderConfig(&g_gpLedRenderCfg);
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
    /* Drive frame stepping from the 1 ms scheduler so host-provided intervals stay in millisecond units. */
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
        || (frameCount == 0U)
        || (frameCount > GP_MATRIX_ANIMATION_MAX_FRAMES))
    {
        return kGpMatrixStatusUnsupported;
    }

    GpLedAction_ResetAnimationState();
    g_gpLedAnimationUploadActive = 1U;
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
    uint8_t byteIndex;

    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((g_gpLedAnimationUploadActive == 0U)
        || (frameIndex >= g_gpLedAnimationFrameCount)
        || (length != GP_MATRIX_BITMAP_RGB888_FRAME_SIZE)
        || (frameData == 0))
    {
        return kGpMatrixStatusBadSequence;
    }

    for (byteIndex = 0U; byteIndex < GP_MATRIX_BITMAP_RGB888_FRAME_SIZE; ++byteIndex)
    {
        g_gpLedAnimationFrames[frameIndex][byteIndex] = frameData[byteIndex];
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

GpMatrixStatusCode GpLedAction_ApplyAction(const GpMatrixActionPayload xdata *payload)
{
    if (payload == 0)
    {
        return kGpMatrixStatusInternalError;
    }

    if (GpLedAction_IsHostControlEnabled() == 0U)
    {
        return kGpMatrixStatusBusy;
    }

    if ((payload->flags & GP_MATRIX_ACTION_FLAG_REMOTE_RELEASE) != 0U)
    {
        GpLedAction_ReleaseRemoteMode();
        return kGpMatrixStatusOk;
    }

    if (GpLedAction_IsRenderActionValid(payload) == 0U)
    {
        return kGpMatrixStatusUnsupported;
    }

    DrawDrv_GetRenderConfig(&g_gpLedRenderCfg);
    g_gpLedRenderCfg.fgR = payload->primary_r;
    g_gpLedRenderCfg.fgG = payload->primary_g;
    g_gpLedRenderCfg.fgB = payload->primary_b;
    g_gpLedRenderCfg.bgR = 0U;
    g_gpLedRenderCfg.bgG = 0U;
    g_gpLedRenderCfg.bgB = 0U;
    if ((payload->flags & GP_MATRIX_ACTION_FLAG_USE_SECONDARY) != 0U)
    {
        g_gpLedRenderCfg.bgR = payload->secondary_r;
        g_gpLedRenderCfg.bgG = payload->secondary_g;
        g_gpLedRenderCfg.bgB = payload->secondary_b;
    }
    g_gpLedRenderCfg.brightness = payload->brightness;
    g_gpLedRenderCfg.colorMode = (DrawDrv_ColorMode_t)payload->color_mode;
    g_gpLedRenderCfg.direction = (DrawDrv_Direction_t)payload->direction;
    g_gpLedRenderCfg.effect = (DrawDrv_Effect_t)payload->effect;
    g_gpLedRenderCfg.useGradient = (uint8_t)(((payload->flags & GP_MATRIX_ACTION_FLAG_USE_SECONDARY) != 0U)
                                             || (payload->color_mode == kGpMatrixColorModeGradient));
    g_gpLedRenderCfg.gradientSpan = (payload->gradient_span == 0U) ? 96U : payload->gradient_span;
    g_gpLedRenderCfg.scrollStep = (payload->scroll_step == 0U) ? 1U : payload->scroll_step;
    g_gpLedRenderCfg.animStep = (payload->anim_step == 0U) ? 1U : payload->anim_step;

    if (payload->content == kGpMatrixActionContentSolid)
    {
        g_gpLedRenderCfg.contentType = DRAWDRV_CONTENT_SOLID;
        DrawDrv_SetRenderConfig(&g_gpLedRenderCfg);
    }
    else if (payload->content == kGpMatrixActionContentPattern)
    {
        g_gpLedRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        DrawDrv_SetRenderConfig(&g_gpLedRenderCfg);
        DrawDrv_SetImageIndex((uint8_t)(payload->pattern_id % TEST_IMAGE_COUNT));
    }
    else if (payload->content == kGpMatrixActionContentGlyph)
    {
        g_gpLedRenderCfg.contentType = DRAWDRV_CONTENT_GLYPH;
        DrawDrv_SetRenderConfig(&g_gpLedRenderCfg);
        if (DrawDrv_SetTextDisplayGlyph(payload->glyph_id) == 0U)
        {
            return kGpMatrixStatusBadLength;
        }
    }
    else
    {
        return kGpMatrixStatusUnsupported;
    }

    g_gpLedRemoteActive = 1U;
    g_gpLedDirectFrameActive = 0U;
    DrawDrv_RequestRebuild();

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
    if ((glyphSpacing != 0U) && (glyphSpacing != TEST_SCROLL_GLYPH_SPACING))
    {
        return kGpMatrixStatusUnsupported;
    }
    if (length < GP_MATRIX_GLYPH_ROWS_SIZE)
    {
        return kGpMatrixStatusBadLength;
    }

    GpLedAction_RenderGlyphFrame(glyphData);

    return kGpMatrixStatusOk;
}