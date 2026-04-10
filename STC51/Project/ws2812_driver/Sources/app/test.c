#include "config.h"
#include "test.h"
#include "draw_drv.h"
#include "test_image.h"
#include "mid_task.h"
#include "key_ctrl.h"
#include "ws2812_drv.h"

#define TEST_SCHED_TICK_US               1000UL
#define TEST_ROW_INTERVAL_US_DEFAULT     1000UL
#define TEST_ROW_INTERVAL_US_MIN         600UL
#define TEST_ROW_INTERVAL_US_MAX         1500UL
#define TEST_DRAW_FRAME_TASK_PERIOD_MS   40U
#define TEST_DRAW_ANIM_TASK_PERIOD_MS    500U
#define TEST_KEY_TASK_PERIOD_MS          10U
#define TEST_TIMER1_US_PRESCALE          39U
#define TEST_PRESET_MODE_COUNT           4U

#define TEST_PRESET_DIAMOND_FADE         0U
#define TEST_PRESET_CROSS_GRADIENT       1U
#define TEST_PRESET_PYTHON_STATIC        2U
#define TEST_PRESET_JLU_SCROLL           3U

static uint32_t g_testRowIntervalUs = TEST_ROW_INTERVAL_US_DEFAULT;
static uint16_t g_testLastPwmUs = 0;
static DrawDrv_RenderConfig_t g_testRenderCfg;
static uint8_t g_testPresetMode = TEST_PRESET_JLU_SCROLL;

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
    Test_LoadDefaultRenderConfig();

    MidTask_Init();
    KeyCtrl_Init();
    /* Register animation task first so state update runs before frame rebuild when coincident. */
    (void)MidTask_RegisterWithId(TEST_KEY_TASK_PERIOD_MS, KeyCtrl_Task10ms);
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

uint8_t Test_GetPresetMode(void)
{
    return g_testPresetMode;
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
