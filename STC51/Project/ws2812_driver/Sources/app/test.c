#include "config.h"
#include "test.h"
#include "draw_drv.h"
#include "test_image.h"
#include "mid_task.h"
#include "ws2812_drv.h"

#define TEST_SCHED_TICK_US               1000UL
#define TEST_ROW_INTERVAL_US_DEFAULT     1000UL
#define TEST_ROW_INTERVAL_US_MIN         600UL
#define TEST_ROW_INTERVAL_US_MAX         1500UL
#define TEST_DRAW_FRAME_TASK_PERIOD_MS   40U
#define TEST_DRAW_ANIM_TASK_PERIOD_MS    500U
#define TEST_TIMER1_US_PRESCALE          39U

static uint32_t g_testRowIntervalUs = TEST_ROW_INTERVAL_US_DEFAULT;
static uint16_t g_testLastPwmUs = 0;
static DrawDrv_RenderConfig_t g_testRenderCfg;

static void Test_OnSchedTickExpired(void)
{
    MidTask_Tick1ms();
    TIMER0_StartOneShotUs(TEST_SCHED_TICK_US);
}

static void Test_Timer1ApplyRefreshInterval(uint32_t intervalUs)
{
    uint16_t reload;

    if (intervalUs < TEST_ROW_INTERVAL_US_MIN)
    {
        intervalUs = TEST_ROW_INTERVAL_US_MIN;
    }
    if (intervalUs > TEST_ROW_INTERVAL_US_MAX)
    {
        intervalUs = TEST_ROW_INTERVAL_US_MAX;
    }

    DisableGlobalInt();

    TIMER1_Stop();
    TIMER1_DisableInt();
    TIMER1_TimerMode();
    TIMER1_1TMode();
    TIMER1_Mode0();
    TIMER1_DisableGateINT1();
    TIMER1_SetPrescale(TEST_TIMER1_US_PRESCALE);
    reload = (uint16_t)(65536UL - intervalUs);
    TIMER1_SetReload16(reload);
    TIMER1_ClearFlag();
    TIMER1_EnableInt();
    TIMER1_Run();

    EnableGlobalInt();

    g_testRowIntervalUs = intervalUs;
    g_testLastPwmUs = (uint16_t)intervalUs;
}

static void Test_LoadDefaultRenderConfig(void)
{
    g_testRenderCfg.fgR = 0xFF;
    g_testRenderCfg.fgG = 0xFF;
    g_testRenderCfg.fgB = 0xFF;
    g_testRenderCfg.bgR = 0x00;
    g_testRenderCfg.bgG = 0x00;
    g_testRenderCfg.bgB = 0x00;
    g_testRenderCfg.useGradient = 0;
    g_testRenderCfg.gradientSpan = 96U;
    g_testRenderCfg.scrollStep = 1U;
    /* Default to text scroll mode so glyph replacement is visible immediately. */
    g_testRenderCfg.effect = DRAWDRV_EFFECT_TEXT_SCROLL_JLU;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);
    DrawDrv_SetImageIndex(TEST_IMAGE_IDX_PYTHON_DEMO);
}

void Test_Init(void)
{
    /* PWM/DMA and frame pipeline are delegated to ws2812 and draw drivers. */
    WS2812DRV_Init();
    (void)WS2812DRV_SetDisplayMode(WS2812DRV_MODE_16X16);
    DrawDrv_Init();
    Test_LoadDefaultRenderConfig();

    MidTask_Init();
    /* Register animation task first so state update runs before frame rebuild when coincident. */
    (void)MidTask_RegisterWithId(TEST_DRAW_ANIM_TASK_PERIOD_MS, DrawDrv_Task500ms);
    (void)MidTask_RegisterWithId(TEST_DRAW_FRAME_TASK_PERIOD_MS, DrawDrv_Task40ms);

    Test_Timer1ApplyRefreshInterval(g_testRowIntervalUs);

    /* Timer0 provides 1ms scheduler tick. */
    TIMER0_RegisterUsHook(Test_OnSchedTickExpired);
    TIMER0_StartOneShotUs(TEST_SCHED_TICK_US);
}

void Test_TaskLoop(void)
{
    MidTask_Process();
}

void Test_SetRowIntervalUs(uint32_t intervalUs)
{
    Test_Timer1ApplyRefreshInterval(intervalUs);
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

    if (effectId > (uint8_t)DRAWDRV_EFFECT_TEXT_SCROLL_JLU)
    {
        return 0;
    }

    g_testRenderCfg.effect = (DrawDrv_Effect_t)effectId;
    DrawDrv_SetRenderConfig(&g_testRenderCfg);

    return 1;
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

void PWMAT_DMA_ISR(void) interrupt DMA_PWMAT_VECTOR
{
    /* Forward DMA completion to ws2812 driver for unified state management. */
    WS2812DRV_OnDmaIsr();
}

void TIMER1_ISR(void) interrupt 3
{
    TIMER1_ClearFlag();
    WS2812DRV_RefreshStep();
}
