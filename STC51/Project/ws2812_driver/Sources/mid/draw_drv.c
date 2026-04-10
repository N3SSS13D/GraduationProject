#include "config.h"
#include "draw_drv.h"
#include "test_image.h"
#include "ws2812_drv.h"

#define DRAWDRV_IMAGE_SWITCH_TICKS       4U

static DrawDrv_RenderConfig_t g_drawCfg;
static bit g_drawFrameDirty = 0;
static uint8_t g_drawFrameIndex = 0;
static uint8_t g_drawAnimPhase = 0;
static uint8_t g_drawScrollOffset = 0;
static uint8_t g_drawImageTick = 0;

static uint8_t DrawDrv_Mod255(uint16_t value)
{
    while (value > 255U)
    {
        value = (uint16_t)(value - 256U);
    }

    return (uint8_t)value;
}

static void DrawDrv_SetDefaultConfig(void)
{
    g_drawCfg.fgR = 0xFF;
    g_drawCfg.fgG = 0xFF;
    g_drawCfg.fgB = 0xFF;
    g_drawCfg.bgR = 0x00;
    g_drawCfg.bgG = 0x00;
    g_drawCfg.bgB = 0x00;
    g_drawCfg.useGradient = 0;
    g_drawCfg.gradientSpan = 96U;
    g_drawCfg.scrollStep = 1U;
    g_drawCfg.effect = DRAWDRV_EFFECT_GRADIENT;
}

static void DrawDrv_DecodeRgb332(uint8_t packed, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t r3;
    uint8_t g3;
    uint8_t b2;

    r3 = (uint8_t)((packed >> 5) & 0x07U);
    g3 = (uint8_t)((packed >> 2) & 0x07U);
    b2 = (uint8_t)(packed & 0x03U);

    *r = (uint8_t)((uint16_t)r3 * 255U / 7U);
    *g = (uint8_t)((uint16_t)g3 * 255U / 7U);
    *b = (uint8_t)((uint16_t)b2 * 255U / 3U);
}

static uint8_t DrawDrv_GetPatternCount(void)
{
    return TEST_IMAGE_COUNT;
}

static uint8_t DrawDrv_GetPatternPixel(uint8_t imageIndex, uint8_t row, uint8_t col)
{
    const uint8_t code *frame;
    uint16_t idx;
    uint8_t imageRow;

    if ((row >= TEST_IMAGE_ROWS) || (col >= TEST_IMAGE_COLS))
    {
        return 0;
    }

    if (imageIndex >= DrawDrv_GetPatternCount())
    {
        imageIndex = 0;
    }

    frame = g_testImageFrames[imageIndex];
    /* Physical LED rows are indexed bottom-to-top (0..15). */
    imageRow = (uint8_t)((TEST_IMAGE_ROWS - 1U) - row);
    idx = (uint16_t)imageRow * TEST_IMAGE_COLS + (uint16_t)col;

    return frame[idx];
}

static uint8_t DrawDrv_GetScrollSourceCol(uint8_t col, uint8_t activeCols)
{
    uint8_t offset;
    uint8_t srcCol;

    if (activeCols == 0U)
    {
        return col;
    }

    offset = (uint8_t)(g_drawScrollOffset % activeCols);
    if (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT)
    {
        srcCol = (uint8_t)(col + offset);
        if (srcCol >= activeCols)
        {
            srcCol = (uint8_t)(srcCol - activeCols);
        }
    }
    else if (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT)
    {
        if (col >= offset)
        {
            srcCol = (uint8_t)(col - offset);
        }
        else
        {
            srcCol = (uint8_t)(activeCols + col - offset);
        }
    }
    else
    {
        srcCol = col;
    }

    return srcCol;
}

static void DrawDrv_ApplyColorConfig(uint8_t isFg, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint16_t rr;
    uint16_t gg;
    uint16_t bb;

    if (isFg == 0U)
    {
        *r = g_drawCfg.bgR;
        *g = g_drawCfg.bgG;
        *b = g_drawCfg.bgB;
        return;
    }

    rr = (uint16_t)(*r) * (uint16_t)g_drawCfg.fgR;
    gg = (uint16_t)(*g) * (uint16_t)g_drawCfg.fgG;
    bb = (uint16_t)(*b) * (uint16_t)g_drawCfg.fgB;

    *r = (uint8_t)(rr / 255U);
    *g = (uint8_t)(gg / 255U);
    *b = (uint8_t)(bb / 255U);
}

static void DrawDrv_ApplyGradient(uint8_t row, uint8_t col, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t gradR;
    uint8_t gradG;
    uint8_t gradB;
    uint8_t alpha;
    uint8_t mix;
    uint16_t outR;
    uint16_t outG;
    uint16_t outB;

    mix = DrawDrv_Mod255((uint16_t)row * 16U + (uint16_t)col + (uint16_t)g_drawAnimPhase * 4U);
    gradR = mix;
    gradG = (uint8_t)(255U - mix);
    gradB = (uint8_t)(mix >> 1);

    alpha = g_drawCfg.gradientSpan;
    outR = (uint16_t)(*r) * (uint16_t)(255U - alpha) + (uint16_t)gradR * (uint16_t)alpha;
    outG = (uint16_t)(*g) * (uint16_t)(255U - alpha) + (uint16_t)gradG * (uint16_t)alpha;
    outB = (uint16_t)(*b) * (uint16_t)(255U - alpha) + (uint16_t)gradB * (uint16_t)alpha;

    *r = (uint8_t)(outR / 255U);
    *g = (uint8_t)(outG / 255U);
    *b = (uint8_t)(outB / 255U);
}

static void DrawDrv_ApplyBreath(uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t phase;
    uint8_t scale;

    phase = (uint8_t)(g_drawAnimPhase & 0x1FU);
    if (phase < 16U)
    {
        scale = (uint8_t)(96U + phase * 10U);
    }
    else
    {
        scale = (uint8_t)(96U + (31U - phase) * 10U);
    }

    *r = (uint8_t)((uint16_t)(*r) * scale / 255U);
    *g = (uint8_t)((uint16_t)(*g) * scale / 255U);
    *b = (uint8_t)((uint16_t)(*b) * scale / 255U);
}

static void DrawDrv_RebuildFrame(void)
{
    uint8_t row;
    uint8_t col;
    uint8_t srcCol;
    uint8_t activeCols;
    uint8_t packed;
    uint8_t isBg;
    uint8_t r;
    uint8_t g;
    uint8_t b;

    activeCols = WS2812DRV_GetActiveCols();
    if (activeCols > TEST_IMAGE_COLS)
    {
        activeCols = TEST_IMAGE_COLS;
    }

    WS2812DRV_ClearImage();

    for (row = 0; row < WS2812DRV_ROW_NUM; row++)
    {
        for (col = 0; col < activeCols; col++)
        {
            srcCol = DrawDrv_GetScrollSourceCol(col, activeCols);
            packed = DrawDrv_GetPatternPixel(g_drawFrameIndex, row, srcCol);
            isBg = (uint8_t)(packed == (uint8_t)TEST_IMAGE_BG_RGB332);

            DrawDrv_DecodeRgb332(packed, &r, &g, &b);
            DrawDrv_ApplyColorConfig((uint8_t)(isBg == 0U), &r, &g, &b);

            /* Background keeps plain color, no gradient/breath/scroll enhancement. */
            if ((isBg == 0U) && ((g_drawCfg.useGradient != 0U) || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT)))
            {
                DrawDrv_ApplyGradient(row, col, &r, &g, &b);
            }

            if ((isBg == 0U) && (g_drawCfg.effect == DRAWDRV_EFFECT_BREATH))
            {
                DrawDrv_ApplyBreath(&r, &g, &b);
            }

            WS2812DRV_SetPixelRgb(row, col, r, g, b);
        }
    }

    WS2812DRV_EncodeAllRows();
}
void DrawDrv_Init(void)
{
    DrawDrv_SetDefaultConfig();
    g_drawFrameDirty = 1;
    g_drawFrameIndex = 0;
    g_drawAnimPhase = 0;
    g_drawScrollOffset = 0;
    g_drawImageTick = 0;

    DrawDrv_RebuildFrame();
    g_drawFrameDirty = 0;
}

void DrawDrv_Task40ms(void)
{
    if ((g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT) || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_BREATH) || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT))
    {
        g_drawFrameDirty = 1;
    }

    if (g_drawFrameDirty == 0)
    {
        return;
    }

    DrawDrv_RebuildFrame();

    g_drawFrameDirty = 0;
}

void DrawDrv_Task500ms(void)
{
    uint8_t patternCount;
    uint8_t autoAnimate;

    autoAnimate = (uint8_t)(g_drawCfg.effect != DRAWDRV_EFFECT_STATIC);
    if (autoAnimate != 0U)
    {
        g_drawAnimPhase++;
    }

    if (g_drawCfg.scrollStep == 0U)
    {
        g_drawCfg.scrollStep = 1U;
    }

    if ((g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT) || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT))
    {
        g_drawScrollOffset = (uint8_t)(g_drawScrollOffset + g_drawCfg.scrollStep);
    }

    if (autoAnimate != 0U)
    {
        g_drawImageTick++;
        if (g_drawImageTick >= DRAWDRV_IMAGE_SWITCH_TICKS)
        {
            g_drawImageTick = 0;
            patternCount = DrawDrv_GetPatternCount();
            if (patternCount != 0U)
            {
                g_drawFrameIndex++;
                if (g_drawFrameIndex >= patternCount)
                {
                    g_drawFrameIndex = 0;
                }
            }
        }

        g_drawFrameDirty = 1;
    }
}

void DrawDrv_RequestRebuild(void)
{
    g_drawFrameDirty = 1;
}

void DrawDrv_SetRenderConfig(const DrawDrv_RenderConfig_t *cfg)
{
    if (cfg == 0)
    {
        return;
    }

    g_drawCfg = *cfg;
    g_drawFrameDirty = 1;
}

void DrawDrv_GetRenderConfig(DrawDrv_RenderConfig_t *cfg)
{
    if (cfg == 0)
    {
        return;
    }

    *cfg = g_drawCfg;
}

void DrawDrv_SetImageIndex(uint8_t imageIndex)
{
    uint8_t patternCount;

    patternCount = DrawDrv_GetPatternCount();
    if (patternCount == 0U)
    {
        g_drawFrameIndex = 0;
    }
    else
    {
        g_drawFrameIndex = (uint8_t)(imageIndex % patternCount);
    }

    g_drawFrameDirty = 1;
}

uint8_t DrawDrv_GetImageIndex(void)
{
    return g_drawFrameIndex;
}

void DrawDrv_NextImage(void)
{
    DrawDrv_SetImageIndex((uint8_t)(g_drawFrameIndex + 1U));
}
