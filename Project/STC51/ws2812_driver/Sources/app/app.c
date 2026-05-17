#include "config.h"
#include "app.h"
#include "draw_drv.h"
#include "offline_pattern.h"
#include "local_display_scheme.h"
#include "local_display_assets.h"
#include "mid_task.h"
#include "key_ctrl.h"
#include "gp_led_action.h"
#include "gp_led_matrix_ai8051u.h"
#include "ws2812_drv.h"
#include "gp_led_matrix_usb_debug.h"

#define APP_SCHED_TICK_US               500UL
#define APP_ROW_INTERVAL_US_DEFAULT_NORMAL  1000UL
#define APP_ROW_INTERVAL_US_DEFAULT_LEGACY  1000UL
#define APP_ROW_INTERVAL_US_MIN         300UL
#define APP_ROW_INTERVAL_US_SAFETY_MARGIN_LEGACY  120UL
#define APP_ROW_INTERVAL_MS_MIN         1U
#define APP_ROW_INTERVAL_MS_MAX         50000U
#define APP_DRAW_FRAME_TASK_PERIOD_MS_DEFAULT  32U
#define APP_KEY_TASK_PERIOD_MS          10U
#define APP_TIMER1_US_PRESCALE_DEFAULT  0U
#define APP_TIMER1_PRESCALE_MIN         0U
#define APP_TIMER1_PRESCALE_MAX         255U
#define APP_TIMER1_MAX_COUNTER          65535UL
#define APP_TIMER1_TICK_SCALE           1024UL
#define APP_PRESET_MODE_COUNT           8U

#define APP_PRESET_DIAMOND_FADE         0U
#define APP_PRESET_CROSS_GRADIENT       1U
#define APP_PRESET_JLU_EMBLEM_STATIC    2U
#define APP_PRESET_JLU_SCROLL           3U
#define APP_PRESET_DIAMOND_TIMELINE     4U
#define APP_PRESET_BORDER_FADE_IN       5U
#define APP_PRESET_CHECKER_FADE_OUT     6U
#define APP_PRESET_DIAGONAL_COLOR_CYCLE  7U

static uint32_t g_appRowIntervalUs = APP_ROW_INTERVAL_US_DEFAULT_NORMAL;
static uint32_t g_appRowIntervalUsNormal = APP_ROW_INTERVAL_US_DEFAULT_NORMAL;
static uint32_t g_appRowIntervalUsLegacy = APP_ROW_INTERVAL_US_DEFAULT_LEGACY;
static uint16_t g_appLastPwmUs = 0;
static DrawDrv_RenderConfig_t xdata g_appRenderCfg;
static uint8_t g_appPresetMode = APP_PRESET_CROSS_GRADIENT;
static volatile uint8_t g_appDebugMode = 0U;
static volatile uint16_t g_appTimer1CycleCount = 0U;
static volatile uint16_t g_appTimer1CycleTarget = 1U;
static DrawDrv_RenderConfig_t xdata g_appDebugSavedCfg;
static GpLedDisplayProfile xdata g_appDisplayProfile;
static uint8_t g_appDebugSavedImage = 0U;
static uint8_t g_appDebugVisualApplied = 0U;
static GpLedMatrixAi8051uContext xdata g_appAiMatrixCtx;
static uint8_t g_appDrawFrameTaskId = MIDTASK_INVALID_ID;
static uint16_t g_appDrawFrameTaskPeriodMs = APP_DRAW_FRAME_TASK_PERIOD_MS_DEFAULT;

static uint32_t APP_GetIntervalByScanMode(void);
static uint32_t APP_ClampLegacyRowIntervalUs(uint32_t intervalUs);
static void APP_Timer1ApplyRefreshInterval(uint32_t intervalUs);
static uint32_t APP_Timer1GetTicksPerUsScaled(uint8_t prescale);
static void APP_KeyTaskProxy(void);
static void APP_DrawFrameTaskProxy(void);
static uint16_t APP_ResolveLocalDrawFramePeriodMs(const DrawDrv_RenderConfig_t *renderCfg);
static void APP_SyncLocalDrawTaskPeriod(const DrawDrv_RenderConfig_t *renderCfg);
static void APP_SyncLocalDrawTaskPeriodFromDriver(void);
static void APP_BuildProfileFromRenderConfig(const DrawDrv_RenderConfig_t xdata *renderCfg,
                                              GpLedDisplayProfile xdata *profile);
static void APP_ApplyLocalProfileWithSelection(const DrawDrv_RenderConfig_t xdata *renderCfg,
                                                uint8_t applyPattern,
                                                uint8_t patternId,
                                                uint8_t applyGlyph,
                                                uint8_t glyphId);

static uint16_t APP_ResolveLocalDrawFramePeriodMs(const DrawDrv_RenderConfig_t *renderCfg)
{
    if (renderCfg == 0)
    {
        return DRAWDRV_FRAME_INTERVAL_MS_DEFAULT;
    }

    return DrawDrv_NormalizeFrameIntervalMs(renderCfg->frameIntervalMs);
}

static void APP_SyncLocalDrawTaskPeriod(const DrawDrv_RenderConfig_t *renderCfg)
{
    uint16_t periodMs;

    if (g_appDrawFrameTaskId == MIDTASK_INVALID_ID)
    {
        return;
    }

    periodMs = APP_ResolveLocalDrawFramePeriodMs(renderCfg);
    if (periodMs == g_appDrawFrameTaskPeriodMs)
    {
        return;
    }

    if (MidTask_SetPeriod(g_appDrawFrameTaskId, periodMs) != 0U)
    {
        g_appDrawFrameTaskPeriodMs = periodMs;
    }
}

static void APP_SyncLocalDrawTaskPeriodFromDriver(void)
{
    const DrawDrv_RenderConfig_t *renderCfg;

    /* Keep the cooperative draw task aligned with the driver's current time base, including remote profile changes. */
    renderCfg = DrawDrv_GetRenderConfigStorage();
    APP_SyncLocalDrawTaskPeriod(renderCfg);
}

void APP_ApplyLocalRenderConfig(const DrawDrv_RenderConfig_t *renderCfg)
{
    if (renderCfg == 0)
    {
        return;
    }

    g_appRenderCfg = *renderCfg;
    DrawDrv_SetRenderConfig(&g_appRenderCfg);
    APP_SyncLocalDrawTaskPeriod(&g_appRenderCfg);
}

void APP_ApplyLocalPatternConfig(const DrawDrv_RenderConfig_t *renderCfg, uint8_t patternId)
{
    if (renderCfg == 0)
    {
        return;
    }

    g_appRenderCfg = *renderCfg;
    APP_ApplyLocalProfileWithSelection(&g_appRenderCfg, 1U, patternId, 0U, 0U);
}

void APP_ApplyLocalGlyphConfig(const DrawDrv_RenderConfig_t *renderCfg, uint8_t glyphId)
{
    if (renderCfg == 0)
    {
        return;
    }

    g_appRenderCfg = *renderCfg;
    APP_ApplyLocalProfileWithSelection(&g_appRenderCfg, 0U, 0U, 1U, glyphId);
}

void APP_ReleaseRemoteModeForLocalDisplay(void)
{
    GpLedAction_ReleaseRemoteMode();
}

uint8_t APP_RequestRemoteCachedBitmap(void)
{
    return GpLedMatrixAi8051u_RequestCachedBitmap();
}

static void APP_BuildProfileFromRenderConfig(const DrawDrv_RenderConfig_t xdata *renderCfg,
                                              GpLedDisplayProfile xdata *profile)
{
    profile->version = GP_LED_PROFILE_VERSION_V1;
    profile->profileFlags = 0U;
    profile->source = kGpMatrixActionSourceLocal;
    profile->content = (uint8_t)renderCfg->contentType;
    profile->effect = (uint8_t)renderCfg->effect;
    profile->direction = (uint8_t)renderCfg->direction;
    profile->colorMode = (uint8_t)renderCfg->colorMode;
    profile->brightness = renderCfg->brightness;
    profile->primaryR = renderCfg->fgR;
    profile->primaryG = renderCfg->fgG;
    profile->primaryB = renderCfg->fgB;
    profile->secondaryR = renderCfg->bgR;
    profile->secondaryG = renderCfg->bgG;
    profile->secondaryB = renderCfg->bgB;
    profile->patternId = 0U;
    profile->glyphId = 0U;
    profile->scrollStep = renderCfg->scrollStep;
    profile->animStep = renderCfg->animStep;
    profile->gradientSpan = renderCfg->gradientSpan;
    profile->actionFlags = 0U;
    if ((renderCfg->useGradient != 0U) || (renderCfg->colorMode == DRAWDRV_COLOR_GRADIENT)
        || (renderCfg->bgR != 0U) || (renderCfg->bgG != 0U) || (renderCfg->bgB != 0U))
    {
        profile->actionFlags |= GP_MATRIX_ACTION_FLAG_USE_SECONDARY;
    }
    profile->frameIntervalMs = APP_ResolveLocalDrawFramePeriodMs(renderCfg);
    profile->timelineDurationMs = renderCfg->timelineDurationMs;
    profile->timelineRepeatDelayMs = renderCfg->timelineRepeatDelayMs;
    profile->timelineRepeatCount = renderCfg->timelineRepeatCount;
    profile->timelinePath = (uint8_t)renderCfg->timelinePath;
    profile->animationFlags = 0U;
    profile->applyFlags = 0U;
}

static void APP_ApplyLocalProfileWithSelection(const DrawDrv_RenderConfig_t xdata *renderCfg,
                                                uint8_t applyPattern,
                                                uint8_t patternId,
                                                uint8_t applyGlyph,
                                                uint8_t glyphId)
{
    APP_BuildProfileFromRenderConfig(renderCfg, &g_appDisplayProfile);

    if (applyPattern != 0U)
    {
        g_appDisplayProfile.applyFlags |= GP_LED_PROFILE_FLAG_APPLY_PATTERN;
        g_appDisplayProfile.patternId = patternId;
    }
    if (applyGlyph != 0U)
    {
        g_appDisplayProfile.applyFlags |= GP_LED_PROFILE_FLAG_APPLY_GLYPH;
        g_appDisplayProfile.glyphId = glyphId;
    }

    if (GpLedAction_ApplyLocalDisplayProfile(&g_appDisplayProfile) == kGpMatrixStatusOk)
    {
        APP_SyncLocalDrawTaskPeriod(renderCfg);
        return;
    }

    APP_ApplyLocalRenderConfig(renderCfg);
    if (applyPattern != 0U)
    {
        DrawDrv_SetImageIndex(patternId);
    }
    if (applyGlyph != 0U)
    {
        (void)DrawDrv_SetTextDisplayGlyph(glyphId);
    }
    DrawDrv_RequestRebuild();
}

static uint32_t APP_ClampLegacyRowIntervalUs(uint32_t intervalUs)
{
    uint32_t activeCols;
    uint32_t pwmSlotsPerRow;
    uint32_t effectiveCycles;
    uint32_t txUs;
    uint32_t minLegacyUs;

    if (intervalUs < APP_ROW_INTERVAL_US_MIN)
    {
        intervalUs = APP_ROW_INTERVAL_US_MIN;
    }

    activeCols = (uint32_t)WS2812DRV_GetActiveCols();
    pwmSlotsPerRow = (uint32_t)WS2812DRV_ROW_RESET_PREFIX_SLOTS + activeCols * 24UL + 2UL;
    effectiveCycles = pwmSlotsPerRow + (uint32_t)WS2812DRV_RESET_TAIL_SLOTS + 1UL;
    txUs = (effectiveCycles * 5UL + 3UL) / 4UL;
    minLegacyUs = txUs + APP_ROW_INTERVAL_US_SAFETY_MARGIN_LEGACY;

    if (intervalUs < minLegacyUs)
    {
        intervalUs = minLegacyUs;
    }

    return intervalUs;
}

static void APP_DrawFrameTaskProxy(void)
{
    APP_SyncLocalDrawTaskPeriodFromDriver();

    if (GpLedAction_ShouldBypassDrawScheduler() != 0U)
    {
        return;
    }

    DrawDrv_Task();
}

static void APP_KeyTaskProxy(void)
{
    GpLedAction_Task10ms();
    KeyCtrl_Task10ms();
    LocalDisplayScheme_Task10ms();
}

static void APP_ApplyDebugStaticDisplay(void)
{
    if (g_appDebugVisualApplied != 0U)
    {
        return;
    }

    DrawDrv_GetRenderConfig(&g_appDebugSavedCfg);
    g_appDebugSavedImage = DrawDrv_GetImageIndex();

    g_appRenderCfg = g_appDebugSavedCfg;
    /* Keep display content deterministic in debug mode for visual inspection. */
    g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
    g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
    g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
    g_appRenderCfg.useGradient = 0U;
    g_appRenderCfg.effect = DRAWDRV_EFFECT_STATIC;
    g_appRenderCfg.scrollStep = 1U;
    g_appRenderCfg.animStep = 1U;

    APP_ApplyLocalProfileWithSelection(&g_appRenderCfg, 1U, OFFLINE_PATTERN_IDX_DIAMOND, 0U, 0U);
    g_appDebugVisualApplied = 1U;
}

static void APP_RestoreDebugDisplay(void)
{
    if (g_appDebugVisualApplied == 0U)
    {
        return;
    }

    g_appRenderCfg = g_appDebugSavedCfg;
    APP_ApplyLocalProfileWithSelection(&g_appRenderCfg, 1U, g_appDebugSavedImage, 0U, 0U);
    g_appDebugVisualApplied = 0U;
}

static uint8_t APP_Timer1SelectPrescale(uint32_t intervalUs)
{
    uint8_t prescale;
    uint32_t maxSingleUs;
    uint32_t ticksPerUsScaled;

    prescale = APP_TIMER1_PRESCALE_MIN;
    while (prescale < APP_TIMER1_PRESCALE_MAX)
    {
        ticksPerUsScaled = APP_Timer1GetTicksPerUsScaled(prescale);
        maxSingleUs = (APP_TIMER1_MAX_COUNTER * APP_TIMER1_TICK_SCALE) / ticksPerUsScaled;
        if (intervalUs <= maxSingleUs)
        {
            break;
        }

        prescale++;
    }

    return prescale;
}

static uint32_t APP_Timer1GetTicksPerUsScaled(uint8_t prescale)
{
    uint32_t clocksPerUsScaled;
    uint32_t divider;

    divider = (uint32_t)prescale + 1UL;
    clocksPerUsScaled = (((MAIN_Fosc / divider) / 1000UL) * APP_TIMER1_TICK_SCALE + 500UL) / 1000UL;
    if (clocksPerUsScaled == 0UL)
    {
        clocksPerUsScaled = 1UL;
    }

    return clocksPerUsScaled;
}

void APP_SetDebugMode(uint8_t enable)
{
    g_appDebugMode = (uint8_t)(enable != 0U);

    if (g_appDebugMode != 0U)
    {
        APP_ApplyDebugStaticDisplay();
    }
    else
    {
        APP_RestoreDebugDisplay();
    }

    APP_Timer1ApplyRefreshInterval(APP_GetIntervalByScanMode());

}

uint8_t APP_GetDebugMode(void)
{
    return g_appDebugMode;
}

void APP_RestoreNormalScan(void)
{
    /* Re-apply Timer1 row scan settings after debug mode tear-down. */
    APP_Timer1ApplyRefreshInterval(APP_GetIntervalByScanMode());
}

static uint32_t APP_GetIntervalByScanMode(void)
{
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        return g_appRowIntervalUsLegacy;
    }

    return g_appRowIntervalUsNormal;
}

static void APP_ApplyPresetMode(uint8_t presetMode)
{
    uint8_t applyPattern;
    uint8_t applyGlyph;
    uint8_t targetPattern;

    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    applyPattern = 0U;
    applyGlyph = 0U;
    targetPattern = 0U;

    if (presetMode >= APP_PRESET_MODE_COUNT)
    {
        presetMode = 0U;
    }

    if (presetMode == APP_PRESET_DIAMOND_FADE)
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 0U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_BREATH;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 2U;
        g_appRenderCfg.fgR = 0xFF;
        g_appRenderCfg.fgG = 0xC0;
        g_appRenderCfg.fgB = 0x50;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 0U;
        g_appRenderCfg.timelineRepeatDelayMs = 0U;
        g_appRenderCfg.timelineRepeatCount = 0U;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
        applyPattern = 1U;
        targetPattern = OFFLINE_PATTERN_IDX_DIAMOND;
    }
    else if (presetMode == APP_PRESET_CROSS_GRADIENT)
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_GRADIENT;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 1U;
        g_appRenderCfg.gradientSpan = 180U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_GRADIENT;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 2U;
        g_appRenderCfg.fgR = 0x70;
        g_appRenderCfg.fgG = 0xE0;
        g_appRenderCfg.fgB = 0xFF;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 0U;
        g_appRenderCfg.timelineRepeatDelayMs = 0U;
        g_appRenderCfg.timelineRepeatCount = 0U;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
        applyPattern = 1U;
        targetPattern = OFFLINE_PATTERN_IDX_CROSS;
    }
    else if (presetMode == APP_PRESET_JLU_EMBLEM_STATIC)
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 0U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_STATIC;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 1U;
        g_appRenderCfg.fgR = 0xFF;
        g_appRenderCfg.fgG = 0xFF;
        g_appRenderCfg.fgB = 0xFF;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 0U;
        g_appRenderCfg.timelineRepeatDelayMs = 0U;
        g_appRenderCfg.timelineRepeatCount = 0U;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
        applyPattern = 1U;
        targetPattern = OFFLINE_PATTERN_IDX_JLU_EMBLEM;
    }
    else if (presetMode == APP_PRESET_DIAMOND_TIMELINE)
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 0U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_BREATH;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 1U;
        g_appRenderCfg.fgR = 0xFF;
        g_appRenderCfg.fgG = 0x90;
        g_appRenderCfg.fgB = 0x60;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 1600U;
        g_appRenderCfg.timelineRepeatDelayMs = 240U;
        g_appRenderCfg.timelineRepeatCount = 0xFFU;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_BREATH_CURVE;
        applyPattern = 1U;
        targetPattern = OFFLINE_PATTERN_IDX_DIAMOND;
    }
    else if (presetMode == APP_PRESET_BORDER_FADE_IN)
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 0U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_FADE_IN;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 1U;
        g_appRenderCfg.fgR = 0x40;
        g_appRenderCfg.fgG = 0xE0;
        g_appRenderCfg.fgB = 0xFF;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 1200U;
        g_appRenderCfg.timelineRepeatDelayMs = 200U;
        g_appRenderCfg.timelineRepeatCount = 0xFFU;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_EASE_IN_OUT;
        applyPattern = 1U;
        targetPattern = OFFLINE_PATTERN_IDX_BORDER;
    }
    else if (presetMode == APP_PRESET_CHECKER_FADE_OUT)
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 0U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_FADE_OUT;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 1U;
        g_appRenderCfg.fgR = 0xFF;
        g_appRenderCfg.fgG = 0xFF;
        g_appRenderCfg.fgB = 0x70;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 1200U;
        g_appRenderCfg.timelineRepeatDelayMs = 160U;
        g_appRenderCfg.timelineRepeatCount = 0xFFU;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
        applyPattern = 1U;
        targetPattern = OFFLINE_PATTERN_IDX_CHECKER;
    }
    else if (presetMode == APP_PRESET_DIAGONAL_COLOR_CYCLE)
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 0U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_COLOR_CYCLE;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 1U;
        g_appRenderCfg.fgR = 0xFF;
        g_appRenderCfg.fgG = 0xFF;
        g_appRenderCfg.fgB = 0xFF;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 0U;
        g_appRenderCfg.timelineRepeatDelayMs = 0U;
        g_appRenderCfg.timelineRepeatCount = 0U;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
        applyPattern = 1U;
        targetPattern = OFFLINE_PATTERN_IDX_DIAGONAL_X;
    }
    else
    {
        g_appRenderCfg.contentType = DRAWDRV_CONTENT_GLYPH;
        g_appRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_appRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_appRenderCfg.useGradient = 0U;
        g_appRenderCfg.effect = DRAWDRV_EFFECT_TEXT_SCROLL_JLU;
        g_appRenderCfg.scrollStep = 1U;
        g_appRenderCfg.animStep = 1U;
        g_appRenderCfg.fgR = 0xFF;
        g_appRenderCfg.fgG = 0xFF;
        g_appRenderCfg.fgB = 0xFF;
        g_appRenderCfg.bgR = 0x00;
        g_appRenderCfg.bgG = 0x00;
        g_appRenderCfg.bgB = 0x00;
        g_appRenderCfg.timelineDurationMs = 0U;
        g_appRenderCfg.timelineRepeatDelayMs = 0U;
        g_appRenderCfg.timelineRepeatCount = 0U;
        g_appRenderCfg.timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
        applyGlyph = 1U;
    }

    g_appPresetMode = presetMode;
    APP_ApplyLocalProfileWithSelection(&g_appRenderCfg, applyPattern, targetPattern, applyGlyph, 0U);
}

static void APP_OnSchedTickExpired(void)
{
    MidTask_Tick1ms();
    /* Animation playback needs the raw 1 ms cadence instead of the 10 ms cooperative task slot. */
    GpLedAction_Tick1ms();
    TIMER0_StartOneShotUs(APP_SCHED_TICK_US);
}

static void APP_Timer1ApplyRefreshInterval(uint32_t intervalUs)
{
    uint32_t ticksPerUsScaled;
    uint32_t totalTimerTicks;
    uint32_t ticksPerCycle;
    uint32_t cycleTarget;
    uint8_t prescale;
    uint16_t reload;

    if (intervalUs < APP_ROW_INTERVAL_US_MIN)
    {
        intervalUs = APP_ROW_INTERVAL_US_MIN;
    }

    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        intervalUs = APP_ClampLegacyRowIntervalUs(intervalUs);
    }

    if (g_appDebugMode != 0U)
    {
        prescale = APP_Timer1SelectPrescale(intervalUs);
    }
    else
    {
        prescale = APP_TIMER1_US_PRESCALE_DEFAULT;
    }

    ticksPerUsScaled = APP_Timer1GetTicksPerUsScaled(prescale);
    totalTimerTicks = (intervalUs * ticksPerUsScaled + (APP_TIMER1_TICK_SCALE / 2UL)) / APP_TIMER1_TICK_SCALE;
    if (totalTimerTicks == 0UL)
    {
        totalTimerTicks = 1UL;
    }

    /* Split very long intervals into multiple Timer1 overflows to avoid 16-bit overflow. */
    cycleTarget = (totalTimerTicks + APP_TIMER1_MAX_COUNTER - 1UL) / APP_TIMER1_MAX_COUNTER;
    if (cycleTarget == 0UL)
    {
        cycleTarget = 1UL;
    }
    if (cycleTarget > 65535UL)
    {
        cycleTarget = 65535UL;
    }

    ticksPerCycle = (totalTimerTicks + cycleTarget - 1UL) / cycleTarget;
    if (ticksPerCycle == 0UL)
    {
        ticksPerCycle = 1UL;
    }
    if (ticksPerCycle > APP_TIMER1_MAX_COUNTER)
    {
        ticksPerCycle = APP_TIMER1_MAX_COUNTER;
    }

    DisableGlobalInt();

    g_appTimer1CycleCount = 0U;
    g_appTimer1CycleTarget = (uint16_t)cycleTarget;

    TIMER1_Stop();
    TIMER1_DisableInt();
    TIMER1_TimerMode();
    TIMER1_1TMode();
    TIMER1_Mode0();
    TIMER1_DisableGateINT1();
    TIMER1_SetPrescale(prescale);
    reload = (uint16_t)(65536UL - ticksPerCycle);
    TIMER1_SetReload16(reload);
    TIMER1_ClearFlag();
    TIMER1_EnableInt();
    TIMER1_Run();

    EnableGlobalInt();

    g_appRowIntervalUs = intervalUs;
    if (intervalUs > 65535UL)
    {
        g_appLastPwmUs = 65535U;
    }
    else
    {
        g_appLastPwmUs = (uint16_t)intervalUs;
    }
}

static void APP_LoadDefaultRenderConfig(void)
{
    /* The dedicated local scheme module owns the startup carousel and offline key-driven presentation flow. */
    LocalDisplayScheme_Init();
}

void APP_Init(void)
{
    /* PWM/DMA and frame pipeline are delegated to ws2812 and draw drivers. */
    WS2812DRV_Init();
    (void)WS2812DRV_SetDisplayMode(WS2812DRV_MODE_16X16);
    DrawDrv_Init();
    GpLedAction_Init();
    GpLedMatrixAi8051u_Init(&g_appAiMatrixCtx, GP_MATRIX_TRANSPORT_ENDPOINT_ID);
    APP_LoadDefaultRenderConfig();

    MidTask_Init();
    KeyCtrl_Init();
    /* Keep key scan ahead of frame rebuild so local input is reflected in the next draw step. */
    (void)MidTask_RegisterWithId(APP_KEY_TASK_PERIOD_MS, APP_KeyTaskProxy);
    g_appDrawFrameTaskId = MidTask_RegisterWithId(APP_DRAW_FRAME_TASK_PERIOD_MS_DEFAULT, APP_DrawFrameTaskProxy);
    g_appDrawFrameTaskPeriodMs = APP_ResolveLocalDrawFramePeriodMs(&g_appRenderCfg);
    APP_SyncLocalDrawTaskPeriod(&g_appRenderCfg);

    APP_Timer1ApplyRefreshInterval(APP_GetIntervalByScanMode());

    /* Timer0 provides 1ms scheduler tick. */
    TIMER0_RegisterUsHook(APP_OnSchedTickExpired);
    TIMER0_StartOneShotUs(APP_SCHED_TICK_US);
}

void APP_TaskLoop(void)
{
    /* Temporary: USB DEBUG command enters a blocking row-test loop. */
    GpLedMatrixUsbDebug_Run();

    GpLedMatrixAi8051u_Poll(&g_appAiMatrixCtx);
    /* Process deferred animation frame rendering so heavy encoding stays out of ISR. */
    GpLedAction_RenderPendingAnimationFrame();
    MidTask_Process();
}

void APP_SetRowIntervalUs(uint32_t intervalUs)
{
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        g_appRowIntervalUsLegacy = intervalUs;
    }
    else
    {
        g_appRowIntervalUsNormal = intervalUs;
    }

    APP_Timer1ApplyRefreshInterval(intervalUs);
}

void APP_SetNormalRowIntervalUs(uint32_t intervalUs)
{
    g_appRowIntervalUsNormal = intervalUs;
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_NORMAL_PAIR)
    {
        APP_Timer1ApplyRefreshInterval(intervalUs);
    }
}

void APP_SetLegacyRowIntervalUs(uint32_t intervalUs)
{
    intervalUs = APP_ClampLegacyRowIntervalUs(intervalUs);
    g_appRowIntervalUsLegacy = intervalUs;
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        APP_Timer1ApplyRefreshInterval(intervalUs);
    }
}

void APP_SetNormalRowIntervalMs(uint16_t intervalMs)
{
    uint32_t intervalUs;

    if (intervalMs < APP_ROW_INTERVAL_MS_MIN)
    {
        intervalMs = APP_ROW_INTERVAL_MS_MIN;
    }
    if ((g_appDebugMode != 0U) && (intervalMs > APP_ROW_INTERVAL_MS_MAX))
    {
        intervalMs = APP_ROW_INTERVAL_MS_MAX;
    }
    intervalUs = (uint32_t)intervalMs * 1000UL;
    APP_SetNormalRowIntervalUs(intervalUs);
}

void APP_SetLegacyRowIntervalMs(uint16_t intervalMs)
{
    uint32_t intervalUs;

    if (intervalMs < APP_ROW_INTERVAL_MS_MIN)
    {
        intervalMs = APP_ROW_INTERVAL_MS_MIN;
    }
    if ((g_appDebugMode != 0U) && (intervalMs > APP_ROW_INTERVAL_MS_MAX))
    {
        intervalMs = APP_ROW_INTERVAL_MS_MAX;
    }
    intervalUs = (uint32_t)intervalMs * 1000UL;
    APP_SetLegacyRowIntervalUs(intervalUs);
}

uint32_t APP_GetRowIntervalUs(void)
{
    return g_appRowIntervalUs;
}

uint16_t APP_GetLastPwmUs(void)
{
    return g_appLastPwmUs;
}

uint8_t APP_SetDisplayMode(uint8_t mode16x)
{
    WS2812DRV_DisplayMode_t mode;

    if (mode16x == 8U)
    {
        mode = WS2812DRV_MODE_16X8;
    }
    else if (mode16x == 16U)
    {
        mode = WS2812DRV_MODE_16X16;
    }
    else
    {
        return 0;
    }

    if (WS2812DRV_SetDisplayMode(mode) == 0)
    {
        return 0;
    }

    DrawDrv_RequestRebuild();

    return 1;
}

uint8_t APP_GetDisplayMode(void)
{
    if (WS2812DRV_GetDisplayMode() == WS2812DRV_MODE_16X16)
    {
        return 16U;
    }

    return 8U;
}

uint8_t APP_SetImageIndex(uint8_t imageIndex)
{
    DrawDrv_SetImageIndex(imageIndex);

    return 1;
}

uint8_t APP_GetImageIndex(void)
{
    return DrawDrv_GetImageIndex();
}

void APP_NextImage(void)
{
    DrawDrv_NextImage();
}

uint8_t APP_SetRenderEffect(uint8_t effectId)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);

    if (effectId > (uint8_t)DRAWDRV_EFFECT_GRADIENT_REVEAL)
    {
        return 0;
    }

    g_appRenderCfg.effect = (DrawDrv_Effect_t)effectId;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);

    return 1;
}

uint8_t APP_SetContentType(uint8_t contentType)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    if (contentType > (uint8_t)DRAWDRV_CONTENT_CLOCK)
    {
        return 0;
    }

    g_appRenderCfg.contentType = (DrawDrv_ContentType_t)contentType;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);

    return 1;
}

uint8_t APP_SetDirection(uint8_t direction)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    if (direction > (uint8_t)DRAWDRV_DIR_ROTATE_CCW_90)
    {
        return 0;
    }

    g_appRenderCfg.direction = (DrawDrv_Direction_t)direction;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);

    return 1;
}

uint8_t APP_SetColorMode(uint8_t colorMode)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    if (colorMode > (uint8_t)DRAWDRV_COLOR_GRADIENT)
    {
        return 0;
    }

    g_appRenderCfg.colorMode = (DrawDrv_ColorMode_t)colorMode;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);

    return 1;
}

void APP_SetScrollStep(uint8_t step)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    if (step == 0U)
    {
        step = 1U;
    }
    g_appRenderCfg.scrollStep = step;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

void APP_SetAnimStep(uint8_t step)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    g_appRenderCfg.animStep = step;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

void APP_SetFrameIntervalMs(uint16_t intervalMs)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    g_appRenderCfg.frameIntervalMs = DrawDrv_NormalizeFrameIntervalMs(intervalMs);
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

void APP_SetGradientSpan(uint8_t span)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    g_appRenderCfg.gradientSpan = span;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

void APP_SetBrightness(uint8_t brightness)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    g_appRenderCfg.brightness = brightness;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

void APP_SetRenderUseGradient(uint8_t enable)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    g_appRenderCfg.useGradient = (uint8_t)(enable != 0U);
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

void APP_SetForegroundColor(uint8_t r, uint8_t g, uint8_t b)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    g_appRenderCfg.fgR = r;
    g_appRenderCfg.fgG = g;
    g_appRenderCfg.fgB = b;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

void APP_SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
    DrawDrv_GetRenderConfig(&g_appRenderCfg);
    g_appRenderCfg.bgR = r;
    g_appRenderCfg.bgG = g;
    g_appRenderCfg.bgB = b;
    APP_ApplyLocalRenderConfig(&g_appRenderCfg);
}

uint8_t APP_SetGlyphDisplayIndex(uint8_t glyphIndex)
{
    return DrawDrv_SetTextDisplayGlyph(glyphIndex);
}

uint8_t APP_SetScrollGlyphSequence(const uint8_t *glyphList, uint8_t count)
{
    return DrawDrv_SetTextScrollSequence(glyphList, count);
}

void APP_NextPresetMode(void)
{
    uint8_t nextMode;

    nextMode = (uint8_t)(g_appPresetMode + 1U);
    if (nextMode >= APP_PRESET_MODE_COUNT)
    {
        nextMode = 0U;
    }

    APP_ApplyPresetMode(nextMode);
}

void APP_NextOfflinePattern(void)
{
    LocalDisplayScheme_NextPattern();
}

void APP_ShowOfflineScrollText(void)
{
    LocalDisplayScheme_ShowTextScroll();
}

void APP_ShowOfflineClock(void)
{
    LocalDisplayScheme_ShowClock();
}

void APP_ToggleOfflineTextClock(void)
{
    LocalDisplayScheme_ToggleTextClock();
}

void APP_NextOfflineEffect(void)
{
    LocalDisplayScheme_NextEffect();
}

void APP_NextOfflineColor(void)
{
    LocalDisplayScheme_NextColor();
}

void APP_ToggleControlMode(void)
{
    GpLedAction_ToggleModeOverride();
}

uint8_t APP_GetControlMode(void)
{
    return (uint8_t)GpLedAction_GetControlMode();
}

uint8_t APP_GetPresetMode(void)
{
    return g_appPresetMode;
}

uint8_t APP_ToggleScanMode(void)
{
    WS2812DRV_ScanMode_t mode;

    mode = WS2812DRV_ToggleScanMode();
    if (mode == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        APP_Timer1ApplyRefreshInterval(g_appRowIntervalUsLegacy);
        return 1U;
    }

    APP_Timer1ApplyRefreshInterval(g_appRowIntervalUsNormal);

    return 0U;
}

uint8_t APP_GetScanMode(void)
{
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        return 1U;
    }

    return 0U;
}

void PWMAT_DMA_ISR(void) interrupt DMA_PWMAT_VECTOR
{
    /* Forward DMA completion to ws2812 driver for unified state management. */
    WS2812DRV_OnDmaIsr();
}

void TIMER1_ISR(void) interrupt 3
{
    TIMER1_ClearFlag();

    g_appTimer1CycleCount++;
    if (g_appTimer1CycleCount < g_appTimer1CycleTarget)
    {
        return;
    }

    g_appTimer1CycleCount = 0U;
    WS2812DRV_RefreshStep();
}
