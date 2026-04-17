#include "config.h"
#include "test.h"
#include "draw_drv.h"
#include "test_image.h"
#include "mid_task.h"
#include "key_ctrl.h"
#include "gp_led_action.h"
#include "gp_led_matrix_ai8051u.h"
#include "ws2812_drv.h"

#define TEST_SCHED_TICK_US               1000UL
#define TEST_ROW_INTERVAL_US_DEFAULT_NORMAL  1000UL
#define TEST_ROW_INTERVAL_US_DEFAULT_LEGACY  1000UL
#define TEST_ROW_INTERVAL_US_MIN         300UL
#define TEST_ROW_INTERVAL_US_SAFETY_MARGIN_LEGACY  120UL
#define TEST_ROW_INTERVAL_MS_MIN         1U
#define TEST_ROW_INTERVAL_MS_MAX         50000U
#define TEST_DRAW_FRAME_TASK32MS_PERIOD_MS   36U
#define TEST_DRAW_ANIM_TASK_PERIOD_MS    500U
#define TEST_KEY_TASK_PERIOD_MS          10U
#define TEST_DEBUG_TASK_PERIOD_MS        1000U
#define TEST_TIMER1_US_PRESCALE_DEFAULT  39U
#define TEST_TIMER1_PRESCALE_MIN         39U
#define TEST_TIMER1_PRESCALE_MAX         255U
#define TEST_TIMER1_MAX_COUNTER          65535UL
#define TEST_TIMER1_SCALE_BASE           (TEST_TIMER1_US_PRESCALE_DEFAULT + 1UL)
#define TEST_TIMER2_US_PRESCALE          39U
#define TEST_PRESET_MODE_COUNT           4U

#define TEST_PRESET_DIAMOND_FADE         0U
#define TEST_PRESET_CROSS_GRADIENT       1U
#define TEST_PRESET_PYTHON_STATIC        2U
#define TEST_PRESET_JLU_SCROLL           3U

static uint32_t g_testRowIntervalUs = TEST_ROW_INTERVAL_US_DEFAULT_NORMAL;
static uint32_t g_testRowIntervalUsNormal = TEST_ROW_INTERVAL_US_DEFAULT_NORMAL;
static uint32_t g_testRowIntervalUsLegacy = TEST_ROW_INTERVAL_US_DEFAULT_LEGACY;
static uint16_t g_testLastPwmUs = 0;
static DrawDrv_RenderConfig_t xdata g_testRenderCfg;
static uint8_t g_testPresetMode = TEST_PRESET_CROSS_GRADIENT;
static volatile uint16_t g_testDbgStartUs = 0U;
static volatile uint16_t g_testDbgLastUs = 0U;
static volatile uint16_t g_testDbgMinUs = 0xFFFFU;
static volatile uint16_t g_testDbgMaxUs = 0U;
static volatile uint32_t g_testDbgSumUs = 0UL;
static volatile uint16_t g_testDbgCnt = 0U;
static volatile uint8_t g_testDbgPending = 0U;
static volatile uint8_t g_testDebugMode = 0U;
static volatile uint16_t g_testDbgRowSeq = 0U;
static volatile uint16_t g_testTimer1CycleCount = 0U;
static volatile uint16_t g_testTimer1CycleTarget = 1U;
static DrawDrv_RenderConfig_t xdata g_testDebugSavedCfg;
static uint8_t g_testDebugSavedImage = 0U;
static uint8_t g_testDebugVisualApplied = 0U;
static GpLedMatrixAi8051uContext xdata g_testAiMatrixCtx;

static uint32_t Test_GetIntervalByScanMode(void);
static uint32_t Test_ClampLegacyRowIntervalUs(uint32_t intervalUs);
static void Test_Timer1ApplyRefreshInterval(uint32_t intervalUs);
static void Test_KeyTaskProxy(void);
static void Test_DrawFrameTaskProxy(void);
static DrawDrv_Effect_t Test_GetNextOfflineEffect(DrawDrv_ContentType_t contentType, DrawDrv_Effect_t currentEffect);

static uint32_t Test_ClampLegacyRowIntervalUs(uint32_t intervalUs)
{
    uint32_t activeCols;
    uint32_t pwmSlotsPerRow;
    uint32_t effectiveCycles;
    uint32_t txUs;
    uint32_t minLegacyUs;

    if (intervalUs < TEST_ROW_INTERVAL_US_MIN)
    {
        intervalUs = TEST_ROW_INTERVAL_US_MIN;
    }

    activeCols = (uint32_t)WS2812DRV_GetActiveCols();
    pwmSlotsPerRow = (uint32_t)WS2812DRV_ROW_RESET_PREFIX_SLOTS + activeCols * 24UL + 2UL;
    effectiveCycles = pwmSlotsPerRow + (uint32_t)WS2812DRV_RESET_TAIL_SLOTS + 1UL;
    txUs = (effectiveCycles * 5UL + 3UL) / 4UL;
    minLegacyUs = txUs + TEST_ROW_INTERVAL_US_SAFETY_MARGIN_LEGACY;

    if (intervalUs < minLegacyUs)
    {
        intervalUs = minLegacyUs;
    }

    return intervalUs;
}

static void Test_DrawAnimTaskProxy(void)
{
    if (g_testDebugMode != 0U)
    {
        return;
    }
    if (GpLedAction_ShouldBypassDrawScheduler() != 0U)
    {
        return;
    }

    DrawDrv_Task500ms();
}

static void Test_DrawFrameTaskProxy(void)
{
    if (GpLedAction_ShouldBypassDrawScheduler() != 0U)
    {
        return;
    }

    DrawDrv_Task32ms();
}

static void Test_KeyTaskProxy(void)
{
    GpLedAction_Task10ms();
    KeyCtrl_Task10ms();
}

static DrawDrv_Effect_t Test_GetNextOfflineEffect(DrawDrv_ContentType_t contentType, DrawDrv_Effect_t currentEffect)
{
    if (contentType == DRAWDRV_CONTENT_GLYPH)
    {
        switch (currentEffect)
        {
            case DRAWDRV_EFFECT_TEXT_SCROLL_JLU:
                return DRAWDRV_EFFECT_STATIC;

            case DRAWDRV_EFFECT_STATIC:
                return DRAWDRV_EFFECT_BREATH;

            case DRAWDRV_EFFECT_BREATH:
                return DRAWDRV_EFFECT_COLOR_CYCLE;

            case DRAWDRV_EFFECT_COLOR_CYCLE:
            default:
                return DRAWDRV_EFFECT_TEXT_SCROLL_JLU;
        }
    }

    switch (currentEffect)
    {
        case DRAWDRV_EFFECT_STATIC:
            return DRAWDRV_EFFECT_BREATH;

        case DRAWDRV_EFFECT_BREATH:
            return DRAWDRV_EFFECT_GRADIENT;

        case DRAWDRV_EFFECT_GRADIENT:
            return DRAWDRV_EFFECT_COLOR_CYCLE;

        case DRAWDRV_EFFECT_COLOR_CYCLE:
        default:
            return DRAWDRV_EFFECT_STATIC;
    }
}

static void Test_ApplyDebugStaticDisplay(void)
{
    if (g_testDebugVisualApplied != 0U)
    {
        return;
    }

    DrawDrv_GetRenderConfig(&g_testDebugSavedCfg);
    g_testDebugSavedImage = DrawDrv_GetImageIndex();

    g_testRenderCfg = g_testDebugSavedCfg;
    /* Keep display content deterministic in debug mode for visual inspection. */
    g_testRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
    g_testRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
    g_testRenderCfg.direction = DRAWDRV_DIR_NORMAL;
    g_testRenderCfg.useGradient = 0U;
    g_testRenderCfg.effect = DRAWDRV_EFFECT_STATIC;
    g_testRenderCfg.scrollStep = 1U;
    g_testRenderCfg.animStep = 1U;

    DrawDrv_SetRenderConfig(&g_testRenderCfg);
    DrawDrv_SetImageIndex(TEST_IMAGE_IDX_DIAMOND);
    DrawDrv_RequestRebuild();
    g_testDebugVisualApplied = 1U;
}

static void Test_RestoreDebugDisplay(void)
{
    if (g_testDebugVisualApplied == 0U)
    {
        return;
    }

    g_testRenderCfg = g_testDebugSavedCfg;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
    DrawDrv_SetImageIndex(g_testDebugSavedImage);
    DrawDrv_RequestRebuild();
    g_testDebugVisualApplied = 0U;
}

static uint8_t Test_Timer1SelectPrescale(uint32_t intervalUs)
{
    uint8_t prescale;
    uint32_t maxSingleUs;

    prescale = TEST_TIMER1_PRESCALE_MIN;
    while (prescale < TEST_TIMER1_PRESCALE_MAX)
    {
        maxSingleUs = (TEST_TIMER1_MAX_COUNTER * ((uint32_t)prescale + 1UL)) / TEST_TIMER1_SCALE_BASE;
        if (intervalUs <= maxSingleUs)
        {
            break;
        }

        prescale++;
    }

    return prescale;
}

static uint16_t Test_DebugReadTimer2Us(void)
{
    uint8_t hi1;
    uint8_t lo;
    uint8_t hi2;

    do
    {
        hi1 = T2H;
        lo = T2L;
        hi2 = T2H;
    } while (hi1 != hi2);

    return (uint16_t)(((uint16_t)hi2 << 8) | lo);
}

static void Test_DebugTimer2Init(void)
{
    DisableGlobalInt();

    TIMER2_Stop();
    TIMER2_DisableInt();
    TIMER2_TimerMode();
    TIMER2_1TMode();
    TIMER2_SetPrescale(TEST_TIMER2_US_PRESCALE);
    TIMER2_SetReload16(0U);
    TIMER2_ClearFlag();
    TIMER2_Run();

    EnableGlobalInt();
}

static void Test_DebugTask1s(void)
{
    uint16_t cnt;
    uint16_t lastUs;
    uint16_t minUs;
    uint16_t maxUs;
    uint32_t sumUs;
    uint32_t avgUs;

    DisableGlobalInt();
    cnt = g_testDbgCnt;
    lastUs = g_testDbgLastUs;
    minUs = g_testDbgMinUs;
    maxUs = g_testDbgMaxUs;
    sumUs = g_testDbgSumUs;
    g_testDbgCnt = 0U;
    g_testDbgSumUs = 0UL;
    g_testDbgMinUs = 0xFFFFU;
    g_testDbgMaxUs = 0U;
    EnableGlobalInt();

    if (cnt == 0U)
    {
        printf("[DBG] pwm_dma_us no sample\r\n");
        return;
    }

    avgUs = sumUs / (uint32_t)cnt;
    printf("[DBG] pwm_dma_us last=%u min=%u max=%u avg=%lu cnt=%u\r\n",
           (unsigned int)lastUs,
           (unsigned int)minUs,
           (unsigned int)maxUs,
           (unsigned long)avgUs,
           (unsigned int)cnt);
}

void Test_DebugMarkRowSwitchStart(void)
{
    g_testDbgStartUs = Test_DebugReadTimer2Us();
    g_testDbgPending = 1U;
}

void Test_DebugMarkPwmSendDone(void)
{
    uint16_t endUs;
    uint16_t deltaUs;

    if (g_testDbgPending == 0U)
    {
        return;
    }

    endUs = Test_DebugReadTimer2Us();
    if (endUs >= g_testDbgStartUs)
    {
        deltaUs = (uint16_t)(endUs - g_testDbgStartUs);
    }
    else
    {
        deltaUs = (uint16_t)(0x10000UL + (uint32_t)endUs - (uint32_t)g_testDbgStartUs);
    }

    g_testDbgLastUs = deltaUs;
    if (deltaUs < g_testDbgMinUs)
    {
        g_testDbgMinUs = deltaUs;
    }
    if (deltaUs > g_testDbgMaxUs)
    {
        g_testDbgMaxUs = deltaUs;
    }
    g_testDbgSumUs += (uint32_t)deltaUs;
    g_testDbgCnt++;
    g_testDbgPending = 0U;
}

void Test_SetDebugMode(uint8_t enable)
{
    g_testDebugMode = (uint8_t)(enable != 0U);

    if (g_testDebugMode != 0U)
    {
        Test_ApplyDebugStaticDisplay();
    }
    else
    {
        Test_RestoreDebugDisplay();
    }

    Test_Timer1ApplyRefreshInterval(Test_GetIntervalByScanMode());

    if (g_testDebugMode == 0U)
    {
        g_testDbgRowSeq = 0U;
    }
}

uint8_t Test_GetDebugMode(void)
{
    return g_testDebugMode;
}

static uint32_t Test_GetIntervalByScanMode(void)
{
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        return g_testRowIntervalUsLegacy;
    }

    return g_testRowIntervalUsNormal;
}

static void Test_ApplyPresetMode(uint8_t presetMode)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);

    if (presetMode >= TEST_PRESET_MODE_COUNT)
    {
        presetMode = 0U;
    }

    if (presetMode == TEST_PRESET_DIAMOND_FADE)
    {
        g_testRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_testRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_testRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_testRenderCfg.useGradient = 0U;
        g_testRenderCfg.effect = DRAWDRV_EFFECT_BREATH;
        g_testRenderCfg.scrollStep = 1U;
        g_testRenderCfg.animStep = 2U;
        g_testRenderCfg.fgR = 0xFF;
        g_testRenderCfg.fgG = 0xC0;
        g_testRenderCfg.fgB = 0x50;
        g_testRenderCfg.bgR = 0x00;
        g_testRenderCfg.bgG = 0x00;
        g_testRenderCfg.bgB = 0x00;
        DrawDrv_SetImageIndex(TEST_IMAGE_IDX_DIAMOND);
    }
    else if (presetMode == TEST_PRESET_CROSS_GRADIENT)
    {
        g_testRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_testRenderCfg.colorMode = DRAWDRV_COLOR_GRADIENT;
        g_testRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_testRenderCfg.useGradient = 1U;
        g_testRenderCfg.gradientSpan = 180U;
        g_testRenderCfg.effect = DRAWDRV_EFFECT_GRADIENT;
        g_testRenderCfg.scrollStep = 1U;
        g_testRenderCfg.animStep = 2U;
        g_testRenderCfg.fgR = 0x70;
        g_testRenderCfg.fgG = 0xE0;
        g_testRenderCfg.fgB = 0xFF;
        g_testRenderCfg.bgR = 0x00;
        g_testRenderCfg.bgG = 0x00;
        g_testRenderCfg.bgB = 0x00;
        DrawDrv_SetImageIndex(TEST_IMAGE_IDX_CROSS);
    }
    else if (presetMode == TEST_PRESET_PYTHON_STATIC)
    {
        g_testRenderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
        g_testRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_testRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_testRenderCfg.useGradient = 0U;
        g_testRenderCfg.effect = DRAWDRV_EFFECT_STATIC;
        g_testRenderCfg.scrollStep = 1U;
        g_testRenderCfg.animStep = 1U;
        g_testRenderCfg.fgR = 0xFF;
        g_testRenderCfg.fgG = 0xFF;
        g_testRenderCfg.fgB = 0xFF;
        g_testRenderCfg.bgR = 0x00;
        g_testRenderCfg.bgG = 0x00;
        g_testRenderCfg.bgB = 0x00;
        DrawDrv_SetImageIndex(TEST_IMAGE_IDX_PYTHON_DEMO);
    }
    else
    {
        g_testRenderCfg.contentType = DRAWDRV_CONTENT_GLYPH;
        g_testRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
        g_testRenderCfg.direction = DRAWDRV_DIR_NORMAL;
        g_testRenderCfg.useGradient = 0U;
        g_testRenderCfg.effect = DRAWDRV_EFFECT_TEXT_SCROLL_JLU;
        g_testRenderCfg.scrollStep = 1U;
        g_testRenderCfg.animStep = 1U;
        g_testRenderCfg.fgR = 0xFF;
        g_testRenderCfg.fgG = 0xFF;
        g_testRenderCfg.fgB = 0xFF;
        g_testRenderCfg.bgR = 0x00;
        g_testRenderCfg.bgG = 0x00;
        g_testRenderCfg.bgB = 0x00;
    }

    g_testPresetMode = presetMode;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

static void Test_OnSchedTickExpired(void)
{
    MidTask_Tick1ms();
    TIMER0_StartOneShotUs(TEST_SCHED_TICK_US);
}

static void Test_Timer1ApplyRefreshInterval(uint32_t intervalUs)
{
    uint32_t scaledTicks;
    uint32_t totalTimerTicks;
    uint32_t ticksPerCycle;
    uint32_t cycleTarget;
    uint8_t prescale;
    uint16_t reload;

    if (intervalUs < TEST_ROW_INTERVAL_US_MIN)
    {
        intervalUs = TEST_ROW_INTERVAL_US_MIN;
    }

    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        intervalUs = Test_ClampLegacyRowIntervalUs(intervalUs);
    }

    if (g_testDebugMode != 0U)
    {
        prescale = Test_Timer1SelectPrescale(intervalUs);
    }
    else
    {
        prescale = TEST_TIMER1_US_PRESCALE_DEFAULT;
    }

    if (intervalUs > (0xFFFFFFFFUL / TEST_TIMER1_SCALE_BASE))
    {
        scaledTicks = 0xFFFFFFFFUL;
    }
    else
    {
        scaledTicks = intervalUs * TEST_TIMER1_SCALE_BASE;
    }

    totalTimerTicks = (scaledTicks + (uint32_t)prescale) / ((uint32_t)prescale + 1UL);
    if (totalTimerTicks == 0UL)
    {
        totalTimerTicks = 1UL;
    }

    /* Split very long intervals into multiple Timer1 overflows to avoid 16-bit overflow. */
    cycleTarget = (totalTimerTicks + TEST_TIMER1_MAX_COUNTER - 1UL) / TEST_TIMER1_MAX_COUNTER;
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
    if (ticksPerCycle > TEST_TIMER1_MAX_COUNTER)
    {
        ticksPerCycle = TEST_TIMER1_MAX_COUNTER;
    }

    DisableGlobalInt();

    g_testTimer1CycleCount = 0U;
    g_testTimer1CycleTarget = (uint16_t)cycleTarget;

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

    g_testRowIntervalUs = intervalUs;
    if (intervalUs > 65535UL)
    {
        g_testLastPwmUs = 65535U;
    }
    else
    {
        g_testLastPwmUs = (uint16_t)intervalUs;
    }
}

static void Test_LoadDefaultRenderConfig(void)
{
    g_testRenderCfg.fgR = 0xFF;
    g_testRenderCfg.fgG = 0xFF;
    g_testRenderCfg.fgB = 0xFF;
    g_testRenderCfg.bgR = 0x00;
    g_testRenderCfg.bgG = 0x00;
    g_testRenderCfg.bgB = 0x00;
    g_testRenderCfg.brightness = 200U;
    g_testRenderCfg.contentType = DRAWDRV_CONTENT_GLYPH;
    g_testRenderCfg.colorMode = DRAWDRV_COLOR_SOLID;
    g_testRenderCfg.direction = DRAWDRV_DIR_NORMAL;
    g_testRenderCfg.useGradient = 0;
    g_testRenderCfg.gradientSpan = 96U;
    g_testRenderCfg.scrollStep = 1U;
    g_testRenderCfg.animStep = 1U;
    g_testRenderCfg.effect = DRAWDRV_EFFECT_TEXT_SCROLL_JLU;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
    Test_ApplyPresetMode(g_testPresetMode);
}

void Test_Init(void)
{
    /* PWM/DMA and frame pipeline are delegated to ws2812 and draw drivers. */
    WS2812DRV_Init();
    (void)WS2812DRV_SetDisplayMode(WS2812DRV_MODE_16X16);
    DrawDrv_Init();
    GpLedMatrixAi8051u_Init(&g_testAiMatrixCtx, GP_MATRIX_DEFAULT_I2C_ADDRESS);
    /* Default to TX DMA only; RX DMA stays opt-in until hardware validation confirms it does not perturb refresh timing. */
    GpLedMatrixAi8051u_SetDmaMode(&g_testAiMatrixCtx, 0U, 1U);
    Test_LoadDefaultRenderConfig();

    MidTask_Init();
    KeyCtrl_Init();
    Test_DebugTimer2Init();
    /* Register animation task first so state update runs before frame rebuild when coincident. */
    (void)MidTask_RegisterWithId(TEST_KEY_TASK_PERIOD_MS, Test_KeyTaskProxy);
    (void)MidTask_RegisterWithId(TEST_DRAW_ANIM_TASK_PERIOD_MS, Test_DrawAnimTaskProxy);
    (void)MidTask_RegisterWithId(TEST_DRAW_FRAME_TASK32MS_PERIOD_MS, Test_DrawFrameTaskProxy);
    (void)MidTask_RegisterWithId(TEST_DEBUG_TASK_PERIOD_MS, Test_DebugTask1s);

    Test_Timer1ApplyRefreshInterval(Test_GetIntervalByScanMode());

    /* Timer0 provides 1ms scheduler tick. */
    TIMER0_RegisterUsHook(Test_OnSchedTickExpired);
    TIMER0_StartOneShotUs(TEST_SCHED_TICK_US);
}

void Test_TaskLoop(void)
{
    GpLedMatrixAi8051u_Poll(&g_testAiMatrixCtx);
    MidTask_Process();
}

void Test_SetRowIntervalUs(uint32_t intervalUs)
{
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        g_testRowIntervalUsLegacy = intervalUs;
    }
    else
    {
        g_testRowIntervalUsNormal = intervalUs;
    }

    Test_Timer1ApplyRefreshInterval(intervalUs);
}

void Test_SetNormalRowIntervalUs(uint32_t intervalUs)
{
    g_testRowIntervalUsNormal = intervalUs;
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_NORMAL_PAIR)
    {
        Test_Timer1ApplyRefreshInterval(intervalUs);
    }
}

void Test_SetLegacyRowIntervalUs(uint32_t intervalUs)
{
    intervalUs = Test_ClampLegacyRowIntervalUs(intervalUs);
    g_testRowIntervalUsLegacy = intervalUs;
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        Test_Timer1ApplyRefreshInterval(intervalUs);
    }
}

void Test_SetNormalRowIntervalMs(uint16_t intervalMs)
{
    uint32_t intervalUs;

    if (intervalMs < TEST_ROW_INTERVAL_MS_MIN)
    {
        intervalMs = TEST_ROW_INTERVAL_MS_MIN;
    }
    if ((g_testDebugMode != 0U) && (intervalMs > TEST_ROW_INTERVAL_MS_MAX))
    {
        intervalMs = TEST_ROW_INTERVAL_MS_MAX;
    }
    intervalUs = (uint32_t)intervalMs * 1000UL;
    Test_SetNormalRowIntervalUs(intervalUs);
}

void Test_SetLegacyRowIntervalMs(uint16_t intervalMs)
{
    uint32_t intervalUs;

    if (intervalMs < TEST_ROW_INTERVAL_MS_MIN)
    {
        intervalMs = TEST_ROW_INTERVAL_MS_MIN;
    }
    if ((g_testDebugMode != 0U) && (intervalMs > TEST_ROW_INTERVAL_MS_MAX))
    {
        intervalMs = TEST_ROW_INTERVAL_MS_MAX;
    }
    intervalUs = (uint32_t)intervalMs * 1000UL;
    Test_SetLegacyRowIntervalUs(intervalUs);
}

uint32_t Test_GetRowIntervalUs(void)
{
    return g_testRowIntervalUs;
}

uint16_t Test_GetLastPwmUs(void)
{
    return g_testLastPwmUs;
}

uint8_t Test_SetDisplayMode(uint8_t mode16x)
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

uint8_t Test_GetDisplayMode(void)
{
    if (WS2812DRV_GetDisplayMode() == WS2812DRV_MODE_16X16)
    {
        return 16U;
    }

    return 8U;
}

uint8_t Test_SetImageIndex(uint8_t imageIndex)
{
    DrawDrv_SetImageIndex(imageIndex);

    return 1;
}

uint8_t Test_GetImageIndex(void)
{
    return DrawDrv_GetImageIndex();
}

void Test_NextImage(void)
{
    DrawDrv_NextImage();
}

uint8_t Test_SetRenderEffect(uint8_t effectId)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);

    if (effectId > (uint8_t)DRAWDRV_EFFECT_COLOR_CYCLE)
    {
        return 0;
    }

    g_testRenderCfg.effect = (DrawDrv_Effect_t)effectId;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);

    return 1;
}

uint8_t Test_SetContentType(uint8_t contentType)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    if (contentType > (uint8_t)DRAWDRV_CONTENT_GLYPH)
    {
        return 0;
    }

    g_testRenderCfg.contentType = (DrawDrv_ContentType_t)contentType;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);

    return 1;
}

uint8_t Test_SetDirection(uint8_t direction)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    if (direction > (uint8_t)DRAWDRV_DIR_ROTATE_CCW_90)
    {
        return 0;
    }

    g_testRenderCfg.direction = (DrawDrv_Direction_t)direction;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);

    return 1;
}

uint8_t Test_SetColorMode(uint8_t colorMode)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    if (colorMode > (uint8_t)DRAWDRV_COLOR_GRADIENT)
    {
        return 0;
    }

    g_testRenderCfg.colorMode = (DrawDrv_ColorMode_t)colorMode;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);

    return 1;
}

void Test_SetScrollStep(uint8_t step)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    if (step == 0U)
    {
        step = 1U;
    }
    g_testRenderCfg.scrollStep = step;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

void Test_SetAnimStep(uint8_t step)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    g_testRenderCfg.animStep = step;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

void Test_SetGradientSpan(uint8_t span)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    g_testRenderCfg.gradientSpan = span;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

void Test_SetBrightness(uint8_t brightness)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    g_testRenderCfg.brightness = brightness;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

void Test_SetRenderUseGradient(uint8_t enable)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    g_testRenderCfg.useGradient = (uint8_t)(enable != 0U);
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

void Test_SetForegroundColor(uint8_t r, uint8_t g, uint8_t b)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    g_testRenderCfg.fgR = r;
    g_testRenderCfg.fgG = g;
    g_testRenderCfg.fgB = b;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

void Test_SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    g_testRenderCfg.bgR = r;
    g_testRenderCfg.bgG = g;
    g_testRenderCfg.bgB = b;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
}

uint8_t Test_SetGlyphDisplayIndex(uint8_t glyphIndex)
{
    return DrawDrv_SetTextDisplayGlyph(glyphIndex);
}

uint8_t Test_SetScrollGlyphSequence(const uint8_t *glyphList, uint8_t count)
{
    return DrawDrv_SetTextScrollSequence(glyphList, count);
}

void Test_NextPresetMode(void)
{
    uint8_t nextMode;

    nextMode = (uint8_t)(g_testPresetMode + 1U);
    if (nextMode >= TEST_PRESET_MODE_COUNT)
    {
        nextMode = 0U;
    }

    Test_ApplyPresetMode(nextMode);
}

void Test_NextOfflinePattern(void)
{
    if (GpLedAction_IsOnlineModeActive() != 0U)
    {
        return;
    }

    Test_NextPresetMode();
}

void Test_NextOfflineEffect(void)
{
    if (GpLedAction_IsOnlineModeActive() != 0U)
    {
        return;
    }

    DrawDrv_GetRenderConfig(&g_testRenderCfg);
    g_testRenderCfg.effect = Test_GetNextOfflineEffect(g_testRenderCfg.contentType, g_testRenderCfg.effect);
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
    DrawDrv_RequestRebuild();
}

void Test_ToggleControlMode(void)
{
    GpLedAction_ToggleModeOverride();
}

uint8_t Test_GetControlMode(void)
{
    return (uint8_t)GpLedAction_GetControlMode();
}

uint8_t Test_GetPresetMode(void)
{
    return g_testPresetMode;
}

uint8_t Test_ToggleScanMode(void)
{
    WS2812DRV_ScanMode_t mode;

    mode = WS2812DRV_ToggleScanMode();
    if (mode == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        Test_Timer1ApplyRefreshInterval(g_testRowIntervalUsLegacy);
        return 1U;
    }

    Test_Timer1ApplyRefreshInterval(g_testRowIntervalUsNormal);

    return 0U;
}

uint8_t Test_GetScanMode(void)
{
    if (WS2812DRV_GetScanMode() == WS2812DRV_SCAN_LEGACY_SHIFT)
    {
        return 1U;
    }

    return 0U;
}

void PWMAT_DMA_ISR(void) interrupt DMA_PWMAT_VECTOR
{
    uint8_t rowA;
    uint8_t rowB;
    uint16_t txLen;
    uint16_t lineSeq;
    uint16_t lineTimeUs;
    uint32_t intervalUs;
    uint8_t scanMode;

    /* Forward DMA completion to ws2812 driver for unified state management. */
    WS2812DRV_OnDmaIsr();

    if (g_testDebugMode == 0U)
    {
        return;
    }

    rowA = WS2812DRV_GetLastScanRowA();
    rowB = WS2812DRV_GetLastScanRowB();
    txLen = WS2812DRV_GetLastScanTxLen();
    lineTimeUs = g_testDbgLastUs;
    intervalUs = g_testRowIntervalUs;
    scanMode = (uint8_t)WS2812DRV_GetScanMode();
    g_testDbgRowSeq++;
    lineSeq = g_testDbgRowSeq;

    printf("[DBG_ROW] seq=%u mode=%u rowA=%u rowB=%u interval_ms=%lu interval_us=%lu tx=%u line_us=%u\r\n",
           (unsigned int)lineSeq,
           (unsigned int)scanMode,
           (unsigned int)rowA,
           (unsigned int)rowB,
           (unsigned long)(intervalUs / 1000UL),
           (unsigned long)intervalUs,
           (unsigned int)txLen,
           (unsigned int)lineTimeUs);
}

void TIMER1_ISR(void) interrupt 3
{
    TIMER1_ClearFlag();

    g_testTimer1CycleCount++;
    if (g_testTimer1CycleCount < g_testTimer1CycleTarget)
    {
        return;
    }

    g_testTimer1CycleCount = 0U;
    WS2812DRV_RefreshStep();
}
