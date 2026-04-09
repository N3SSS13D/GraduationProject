#include "config.h"
#include "test.h"
#include "draw_drv.h"
#include "mid_task.h"
#include "ws2812_drv.h"

#define TEST_SCHED_TICK_US               1000UL
#define TEST_ROW_INTERVAL_US_DEFAULT     1000UL
#define TEST_ROW_INTERVAL_US_MIN         1000UL
#define TEST_ROW_INTERVAL_US_MAX         65535UL
#define TEST_DRAW_FRAME_TASK_PERIOD_MS   40U
#define TEST_DRAW_ANIM_TASK_PERIOD_MS    500U
#define TEST_TIMER1_US_PRESCALE          39U

static uint32_t g_testRowIntervalUs = TEST_ROW_INTERVAL_US_DEFAULT;
static uint16_t g_testLastPwmUs = 0;

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

void Test_Init(void)
{
    /* PWM/DMA and frame pipeline are delegated to ws2812 and draw drivers. */
    WS2812DRV_Init();
    DrawDrv_Init();

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

uint8_t Test_SetDebugRow(uint8_t row)
{
    return DrawDrv_EnableSingleRowDebug(row);
}

void Test_ClearDebugRow(void)
{
    DrawDrv_DisableSingleRowDebug();
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
