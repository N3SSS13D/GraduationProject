#include "config.h"
#include "draw_drv.h"
#include "offline_pattern.h"
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
static uint8_t g_gpLedAnimationFrameFormat = 0U;
static uint8_t g_gpLedAnimationFramePending = 0U;
static uint16_t g_gpLedAnimationFrameIntervalMs = GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS;
static uint16_t g_gpLedAnimationElapsedMs = 0U;
static uint16_t g_gpLedCommActiveTicks = 0U;
static uint16_t g_gpLedDebugFlowTicks = 0U;
/* Direct-frame rendering shares a single scratch area to reduce EDATA usage. */
static DrawDrv_RenderConfig_t xdata g_gpLedRenderCfg;
/* Compact remote animations keep up to 24 variable-size frames for loop playback.
   BITMAP_RGB888 frames use 38 bytes; BITMAP_LAYERED frames use up to GP_MATRIX_ANIMATION_LAYERED_MAX_FRAME_SIZE. */
static uint8_t xdata g_gpLedAnimationFrames[GP_MATRIX_ANIMATION_MAX_FRAMES][GP_MATRIX_ANIMATION_LAYERED_MAX_FRAME_SIZE];
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
/* Reuse one profile buffer to keep stack usage stable on AI8051U. */
static GpLedDisplayProfile xdata g_gpLedProfile;

static void GpLedAction_RenderBitmapFrameRgb888(const uint8_t xdata *frameData, uint8_t brightness);
static void GpLedAction_RenderBitmapLayeredFrame(const uint8_t xdata *frameData, uint16_t length, uint8_t brightness);
static GpMatrixStatusCode GpLedAction_ApplyDisplayProfileCore(const GpLedDisplayProfile xdata *profile,
                                                              uint8_t requireHostControl,
                                                              uint8_t markRemoteActive);

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

    if (g_gpLedAnimationFrameFormat == GP_MATRIX_PAYLOAD_FORMAT_BITMAP_LAYERED)
    {
        GpLedAction_RenderBitmapLayeredFrame(g_gpLedAnimationFrames[frameIndex],
                                              GP_MATRIX_ANIMATION_LAYERED_MAX_FRAME_SIZE,
                                              g_gpLedDefaultBrightness);
    }
    else
    {
        GpLedAction_RenderBitmapFrameRgb888(g_gpLedAnimationFrames[frameIndex], g_gpLedDefaultBrightness);
    }
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
    if (profile->effect > kGpMatrixEffectColorCycle)
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
    profile->frameIntervalMs = GP_MATRIX_ANIMATION_DEFAULT_INTERVAL_MS;
    profile->timelineDurationMs = 0U;
    profile->timelineRepeatDelayMs = 0U;
    profile->timelineRepeatCount = 0U;
    profile->timelinePath = GP_LED_TIMELINE_PATH_LINEAR;
    profile->animationFlags = 0U;
    profile->applyFlags = GP_LED_PROFILE_FLAG_APPLY_PATTERN | GP_LED_PROFILE_FLAG_APPLY_GLYPH;
}

static void GpLedAction_ProfileToRenderConfig(const GpLedDisplayProfile xdata *profile,
                                              DrawDrv_RenderConfig_t xdata *renderCfg)
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
    uint8_t byteIndex;

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

    for (byteIndex = 0U; byteIndex < (uint8_t)length; ++byteIndex)
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

static GpMatrixStatusCode GpLedAction_ApplyDisplayProfileCore(const GpLedDisplayProfile xdata *profile,
                                                              uint8_t requireHostControl,
                                                              uint8_t markRemoteActive)
{
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
    GpLedAction_ProfileToRenderConfig(profile, &g_gpLedRenderCfg);
    if (profile->content == kGpMatrixActionContentSolid)
    {
        g_gpLedRenderCfg.contentType = DRAWDRV_CONTENT_SOLID;
        DrawDrv_SetRenderConfig(&g_gpLedRenderCfg);
    }
    else if (profile->content == kGpMatrixActionContentPattern)
    {
        g_gpLedRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        DrawDrv_SetRenderConfig(&g_gpLedRenderCfg);
        if ((profile->applyFlags & GP_LED_PROFILE_FLAG_APPLY_PATTERN) != 0U)
        {
            DrawDrv_SetImageIndex((uint8_t)(profile->patternId % OfflinePattern_GetCount()));
        }
    }
    else
    {
        g_gpLedRenderCfg.contentType = DRAWDRV_CONTENT_GLYPH;
        DrawDrv_SetRenderConfig(&g_gpLedRenderCfg);
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
    GpLedAction_RenderBitmapLayeredFrame(frameData, length, g_gpLedDefaultBrightness);

    totalLayers = (uint8_t)(frameData[0] >> 4);
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
    if ((glyphSpacing != 0U) && (glyphSpacing != TEST_SCROLL_GLYPH_SPACING))
    {
        return kGpMatrixStatusUnsupported;
    }
    if (length < GP_MATRIX_GLYPH_ROWS_SIZE)
    {
        return kGpMatrixStatusBadLength;
    }

    g_gpLedAnimationActive = 0U;
    GpLedAction_RenderGlyphFrame(glyphData);

    return kGpMatrixStatusOk;
}