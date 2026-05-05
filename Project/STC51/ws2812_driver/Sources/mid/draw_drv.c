#include "config.h"
#include "draw_drv.h"
#include "offline_pattern.h"
#include "test_image.h"
#include "ws2812_drv.h"

#define DRAWDRV_IMAGE_SWITCH_TICKS       4U
#define DRAWDRV_JLU_GLYPH_COUNT          TEST_SCROLL_GLYPH_COUNT
#define DRAWDRV_JLU_GLYPH_WIDTH          TEST_SCROLL_GLYPH_WIDTH
#define DRAWDRV_JLU_GLYPH_SPACING        TEST_SCROLL_GLYPH_SPACING
#define DRAWDRV_JLU_GLYPH_ADVANCE        (DRAWDRV_JLU_GLYPH_WIDTH + DRAWDRV_JLU_GLYPH_SPACING)
#define DRAWDRV_TEXT_SEQUENCE_MAX        32U
#define DRAWDRV_TASK_STEP_MS             32U

static DrawDrv_RenderConfig_t g_drawCfg;
static bit g_drawFrameDirty = 0;
static uint8_t g_drawFrameIndex = 0;
typedef struct
{
    uint8_t animPhase;
    uint16_t scrollOffset;
    uint16_t timelineElapsedMs;
    uint16_t timelineDelayMs;
    uint8_t timelineRepeatRemain;
    uint8_t timelineActive;
} DrawDrv_RuntimeState_t;
static DrawDrv_RuntimeState_t g_drawRt;
static uint8_t xdata g_drawRgbLutR[256];
static uint8_t xdata g_drawRgbLutG[256];
static uint8_t xdata g_drawRgbLutB[256];
static uint8_t xdata g_drawTextSequence[DRAWDRV_TEXT_SEQUENCE_MAX];
static uint8_t g_drawTextSequenceLen = 0U;
static uint8_t g_drawTextDisplayGlyph = 0U;
/* Reuse shared scratch storage to keep text-pixel lookup off the 8051 stack. */
static uint8_t xdata g_drawTextPixelTextRow = 0U;
static uint8_t xdata g_drawTextPixelGlyphIndex = 0U;
static uint8_t xdata g_drawTextPixelGlyphCol = 0U;
static uint8_t xdata g_drawTextPixelOffset = 0U;
static uint8_t xdata g_drawTextPixelGlyphCount = 0U;
static uint16_t xdata g_drawTextPixelTextWidth = 0U;
static uint16_t xdata g_drawTextPixelVirtualCol = 0U;
static uint16_t xdata g_drawTextPixelRowBits = 0U;
static uint16_t xdata g_drawTextPixelBitMask = 0U;

static void DrawDrv_ResetRuntimeState(void)
{
    g_drawRt.animPhase = 0U;
    g_drawRt.scrollOffset = 0U;
    g_drawRt.timelineElapsedMs = 0U;
    g_drawRt.timelineDelayMs = g_drawCfg.timelineRepeatDelayMs;
    g_drawRt.timelineRepeatRemain = g_drawCfg.timelineRepeatCount;
    g_drawRt.timelineActive = (uint8_t)((g_drawCfg.timelineDurationMs != 0U) ? 1U : 0U);
}

static uint8_t DrawDrv_ApplyTimelinePath(uint8_t phase, DrawDrv_TimelinePath_t path)
{
    uint16_t value;

    if (path == DRAWDRV_TIMELINE_PATH_EASE_IN_OUT)
    {
        if (phase <= 15U)
        {
            value = (uint16_t)phase * (uint16_t)phase;
            phase = (uint8_t)((value + 7U) / 15U);
        }
        else
        {
            value = (uint16_t)(31U - phase) * (uint16_t)(31U - phase);
            phase = (uint8_t)(31U - ((value + 7U) / 15U));
        }
    }
    else if (path == DRAWDRV_TIMELINE_PATH_BREATH_CURVE)
    {
        if (phase <= 15U)
        {
            phase = (uint8_t)((uint16_t)phase * 3U / 2U);
        }
        else
        {
            phase = (uint8_t)(31U - ((uint16_t)(31U - phase) * 3U / 2U));
        }

        if (phase > 31U)
        {
            phase = 31U;
        }
    }

    return phase;
}

static void DrawDrv_AdvanceEffectTimeline32ms(void)
{
    uint16_t elapsed;
    uint16_t duration;
    uint16_t scaled;
    uint8_t phase;

    duration = g_drawCfg.timelineDurationMs;
    if (duration == 0U)
    {
        return;
    }
    if ((g_drawCfg.effect != DRAWDRV_EFFECT_BREATH)
        && (g_drawCfg.effect != DRAWDRV_EFFECT_FADE_IN)
        && (g_drawCfg.effect != DRAWDRV_EFFECT_FADE_OUT))
    {
        return;
    }
    if (g_drawRt.timelineActive == 0U)
    {
        return;
    }

    if (g_drawRt.timelineDelayMs != 0U)
    {
        if (g_drawRt.timelineDelayMs > DRAWDRV_TASK_STEP_MS)
        {
            g_drawRt.timelineDelayMs = (uint16_t)(g_drawRt.timelineDelayMs - DRAWDRV_TASK_STEP_MS);
            return;
        }

        g_drawRt.timelineDelayMs = 0U;
    }

    elapsed = (uint16_t)(g_drawRt.timelineElapsedMs + DRAWDRV_TASK_STEP_MS);
    if (elapsed >= duration)
    {
        if (g_drawRt.timelineRepeatRemain == 0U)
        {
            elapsed = duration;
            g_drawRt.timelineActive = 0U;
        }
        else
        {
            if (g_drawRt.timelineRepeatRemain != 0xFFU)
            {
                g_drawRt.timelineRepeatRemain--;
            }

            elapsed = 0U;
            g_drawRt.timelineDelayMs = g_drawCfg.timelineRepeatDelayMs;
        }
    }

    g_drawRt.timelineElapsedMs = elapsed;
    scaled = (uint16_t)((uint32_t)g_drawRt.timelineElapsedMs * 31UL / duration);
    if (scaled > 31U)
    {
        scaled = 31U;
    }

    phase = (uint8_t)scaled;
    g_drawRt.animPhase = DrawDrv_ApplyTimelinePath(phase, g_drawCfg.timelinePath);
}

static uint8_t DrawDrv_GetTextSequenceGlyphCount(void)
{
    if (g_drawTextSequenceLen == 0U)
    {
        return DRAWDRV_JLU_GLYPH_COUNT;
    }

    return g_drawTextSequenceLen;
}

static uint8_t DrawDrv_GetTextSequenceGlyph(uint8_t seqIndex)
{
    uint8_t idx;

    if (g_drawTextSequenceLen == 0U)
    {
        if (DRAWDRV_JLU_GLYPH_COUNT == 0U)
        {
            return 0U;
        }

        return (uint8_t)(seqIndex % DRAWDRV_JLU_GLYPH_COUNT);
    }

    idx = (uint8_t)(seqIndex % g_drawTextSequenceLen);
    return g_drawTextSequence[idx];
}

static uint16_t DrawDrv_GetTextVirtualWidth(void)
{
    return (uint16_t)DrawDrv_GetTextSequenceGlyphCount() * DRAWDRV_JLU_GLYPH_ADVANCE;
}

static void DrawDrv_SetDefaultTextSequence(void)
{
    uint8_t idx;

    g_drawTextSequenceLen = DRAWDRV_JLU_GLYPH_COUNT;
    if (g_drawTextSequenceLen > DRAWDRV_TEXT_SEQUENCE_MAX)
    {
        g_drawTextSequenceLen = DRAWDRV_TEXT_SEQUENCE_MAX;
    }

    for (idx = 0U; idx < g_drawTextSequenceLen; idx++)
    {
        g_drawTextSequence[idx] = idx;
    }

    g_drawTextDisplayGlyph = 0U;
}

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
    g_drawCfg.brightness = 255U;
    g_drawCfg.contentType = DRAWDRV_CONTENT_PATTERN;
    g_drawCfg.colorMode = DRAWDRV_COLOR_SOLID;
    g_drawCfg.direction = DRAWDRV_DIR_NORMAL;
    g_drawCfg.useGradient = 0;
    g_drawCfg.gradientSpan = 96U;
    g_drawCfg.scrollStep = 1U;
    g_drawCfg.animStep = 1U;
    g_drawCfg.effect = DRAWDRV_EFFECT_GRADIENT;
    g_drawCfg.timelineDurationMs = 0U;
    g_drawCfg.timelineRepeatDelayMs = 0U;
    g_drawCfg.timelineRepeatCount = 0U;
    g_drawCfg.timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
}

static void DrawDrv_ApplyBrightness(uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t brightness;

    brightness = g_drawCfg.brightness;
    if (brightness >= 255U)
    {
        return;
    }

    *r = (uint8_t)((uint16_t)(*r) * (uint16_t)brightness / 255U);
    *g = (uint8_t)((uint16_t)(*g) * (uint16_t)brightness / 255U);
    *b = (uint8_t)((uint16_t)(*b) * (uint16_t)brightness / 255U);
}

static void DrawDrv_InitRgbLut(void)
{
    uint16_t idx;
    uint8_t r3;
    uint8_t g3;
    uint8_t b2;

    for (idx = 0; idx < 256U; idx++)
    {
        r3 = (uint8_t)(((uint8_t)idx >> 5) & 0x07U);
        g3 = (uint8_t)(((uint8_t)idx >> 2) & 0x07U);
        b2 = (uint8_t)((uint8_t)idx & 0x03U);

        g_drawRgbLutR[idx] = (uint8_t)((uint16_t)r3 * 255U / 7U);
        g_drawRgbLutG[idx] = (uint8_t)((uint16_t)g3 * 255U / 7U);
        g_drawRgbLutB[idx] = (uint8_t)((uint16_t)b2 * 255U / 3U);
    }
}

static void DrawDrv_DecodeRgb332(uint8_t packed, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = g_drawRgbLutR[packed];
    *g = g_drawRgbLutG[packed];
    *b = g_drawRgbLutB[packed];
}

static uint8_t DrawDrv_GetPatternCount(void)
{
    return OfflinePattern_GetCount();
}

static uint8_t DrawDrv_GetPatternPixel(uint8_t imageIndex, uint8_t row, uint8_t col)
{
    return OfflinePattern_GetPixel(imageIndex, row, col);
}

static uint8_t DrawDrv_GetSolidPixel(void)
{
    return 0xFFU;
}

static uint8_t DrawDrv_GetScrollSourceCol(uint8_t col, uint8_t activeCols)
{
    uint16_t offset;
    uint8_t srcCol;

    if (activeCols == 0U)
    {
        return col;
    }

    offset = (uint8_t)(g_drawRt.scrollOffset % activeCols);
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

static void DrawDrv_MapByDirection(uint8_t row, uint8_t col, uint8_t *srcRow, uint8_t *srcCol)
{
    switch (g_drawCfg.direction)
    {
        case DRAWDRV_DIR_ROTATE_180:
            *srcRow = (uint8_t)((TEST_IMAGE_ROWS - 1U) - row);
            *srcCol = (uint8_t)((TEST_IMAGE_COLS - 1U) - col);
            break;

        case DRAWDRV_DIR_ROTATE_CW_90:
            *srcRow = (uint8_t)((TEST_IMAGE_ROWS - 1U) - col);
            *srcCol = row;
            break;

        case DRAWDRV_DIR_ROTATE_CCW_90:
            *srcRow = col;
            *srcCol = (uint8_t)((TEST_IMAGE_COLS - 1U) - row);
            break;

        case DRAWDRV_DIR_NORMAL:
        default:
            *srcRow = row;
            *srcCol = col;
            break;
    }
}

static uint8_t DrawDrv_GetJluTextPixel(uint8_t row, uint8_t col)
{
    if ((row >= TEST_IMAGE_ROWS) || (col >= TEST_IMAGE_COLS))
    {
        return (uint8_t)TEST_IMAGE_BG_RGB332;
    }

    if ((g_drawCfg.effect == DRAWDRV_EFFECT_TEXT_SCROLL_JLU)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT))
    {
        g_drawTextPixelTextWidth = DrawDrv_GetTextVirtualWidth();
        if (g_drawTextPixelTextWidth == 0U)
        {
            return (uint8_t)TEST_IMAGE_BG_RGB332;
        }

        g_drawTextPixelOffset = (uint8_t)(g_drawRt.scrollOffset % g_drawTextPixelTextWidth);

        g_drawTextPixelVirtualCol = (uint16_t)col + (uint16_t)g_drawTextPixelOffset;
        if (g_drawTextPixelVirtualCol >= g_drawTextPixelTextWidth)
        {
            g_drawTextPixelVirtualCol = (uint16_t)(g_drawTextPixelVirtualCol - g_drawTextPixelTextWidth);
        }

        g_drawTextPixelGlyphCount = DrawDrv_GetTextSequenceGlyphCount();
        if (g_drawTextPixelGlyphCount == 0U)
        {
            return (uint8_t)TEST_IMAGE_BG_RGB332;
        }

        g_drawTextPixelGlyphIndex = DrawDrv_GetTextSequenceGlyph((uint8_t)(g_drawTextPixelVirtualCol / DRAWDRV_JLU_GLYPH_ADVANCE));
        g_drawTextPixelGlyphCol = (uint8_t)(g_drawTextPixelVirtualCol % DRAWDRV_JLU_GLYPH_ADVANCE);
    }
    else
    {
        g_drawTextPixelOffset = 0U;
        g_drawTextPixelGlyphIndex = g_drawTextDisplayGlyph;
        g_drawTextPixelGlyphCol = col;
    }

    if ((g_drawTextPixelGlyphIndex >= DRAWDRV_JLU_GLYPH_COUNT) || (g_drawTextPixelGlyphCol >= DRAWDRV_JLU_GLYPH_WIDTH))
    {
        return (uint8_t)TEST_IMAGE_BG_RGB332;
    }

    /* Physical LED rows are indexed bottom-to-top (0..15). */
    g_drawTextPixelTextRow = (uint8_t)((TEST_IMAGE_ROWS - 1U) - row);
    g_drawTextPixelRowBits = g_testScrollGlyphRows[g_drawTextPixelGlyphIndex][g_drawTextPixelTextRow];
    g_drawTextPixelBitMask = (uint16_t)(0x8000U >> g_drawTextPixelGlyphCol);

    if ((g_drawTextPixelRowBits & g_drawTextPixelBitMask) != 0U)
    {
        return 0xFFU;
    }

    return (uint8_t)TEST_IMAGE_BG_RGB332;
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

    mix = DrawDrv_Mod255((uint16_t)row * 16U + (uint16_t)col + (uint16_t)g_drawRt.animPhase * 4U);
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

    phase = (uint8_t)(g_drawRt.animPhase & 0x1FU);
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

static void DrawDrv_ApplyFade(uint8_t fadeIn, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t phase;
    uint8_t scale;

    phase = (uint8_t)(g_drawRt.animPhase & 0x1FU);
    if (fadeIn != 0U)
    {
        scale = (uint8_t)(phase * 8U);
    }
    else
    {
        scale = (uint8_t)(255U - phase * 8U);
    }

    *r = (uint8_t)((uint16_t)(*r) * scale / 255U);
    *g = (uint8_t)((uint16_t)(*g) * scale / 255U);
    *b = (uint8_t)((uint16_t)(*b) * scale / 255U);
}

static void DrawDrv_ApplyColorCycle(uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t phase;
    uint8_t tr;
    uint8_t tg;
    uint8_t tb;

    phase = (uint8_t)((g_drawRt.animPhase >> 2) % 3U);
    tr = *r;
    tg = *g;
    tb = *b;

    if (phase == 1U)
    {
        *r = tg;
        *g = tb;
        *b = tr;
    }
    else if (phase == 2U)
    {
        *r = tb;
        *g = tr;
        *b = tg;
    }
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
    uint8_t mappedRow;
    uint8_t mappedCol;

    activeCols = WS2812DRV_GetActiveCols();
    if (activeCols > TEST_IMAGE_COLS)
    {
        activeCols = TEST_IMAGE_COLS;
    }

    /* Full-frame rebuild overwrites every active pixel, so pre-clearing the back buffer is redundant here. */
    WS2812DRV_BeginFrameWrite();

    for (row = 0; row < WS2812DRV_ROW_NUM; row++)
    {
        for (col = 0; col < activeCols; col++)
        {
            DrawDrv_MapByDirection(row, col, &mappedRow, &mappedCol);

            if (g_drawCfg.contentType == DRAWDRV_CONTENT_GLYPH)
            {
                packed = DrawDrv_GetJluTextPixel(mappedRow, mappedCol);
            }
            else if (g_drawCfg.contentType == DRAWDRV_CONTENT_SOLID)
            {
                packed = DrawDrv_GetSolidPixel();
            }
            else
            {
                srcCol = mappedCol;
                if ((g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT)
                    || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT))
                {
                    srcCol = DrawDrv_GetScrollSourceCol(mappedCol, activeCols);
                }

                packed = DrawDrv_GetPatternPixel(g_drawFrameIndex, mappedRow, srcCol);
            }

            isBg = (uint8_t)(packed == (uint8_t)TEST_IMAGE_BG_RGB332);

            DrawDrv_DecodeRgb332(packed, &r, &g, &b);
            DrawDrv_ApplyColorConfig((uint8_t)(isBg == 0U), &r, &g, &b);

            /* Background keeps plain color, no gradient/breath/scroll enhancement. */
            if ((isBg == 0U) && ((g_drawCfg.useGradient != 0U)
                || (g_drawCfg.colorMode == DRAWDRV_COLOR_GRADIENT)
                || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT)))
            {
                DrawDrv_ApplyGradient(row, col, &r, &g, &b);
            }

            if ((isBg == 0U) && (g_drawCfg.effect == DRAWDRV_EFFECT_BREATH))
            {
                DrawDrv_ApplyBreath(&r, &g, &b);
            }

            if ((isBg == 0U) && (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_IN))
            {
                DrawDrv_ApplyFade(1U, &r, &g, &b);
            }

            if ((isBg == 0U) && (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_OUT))
            {
                DrawDrv_ApplyFade(0U, &r, &g, &b);
            }

            if ((isBg == 0U) && (g_drawCfg.effect == DRAWDRV_EFFECT_COLOR_CYCLE))
            {
                DrawDrv_ApplyColorCycle(&r, &g, &b);
            }

            DrawDrv_ApplyBrightness(&r, &g, &b);

            WS2812DRV_SetPixelRgbFast(row, col, r, g, b);
        }
    }

    WS2812DRV_EndFrameWrite();

    WS2812DRV_EncodeAllRows();
}
void DrawDrv_Init(void)
{
    DrawDrv_InitRgbLut();
    DrawDrv_SetDefaultConfig();
    DrawDrv_SetDefaultTextSequence();
    g_drawFrameDirty = 1;
    g_drawFrameIndex = 0;
    DrawDrv_ResetRuntimeState();

    DrawDrv_RebuildFrame();
    g_drawFrameDirty = 0;
}

void DrawDrv_Task32ms(void)
{
    uint8_t animCnt;

    if (g_drawCfg.scrollStep == 0U)
    {
        g_drawCfg.scrollStep = 1U;
    }

    if (g_drawCfg.animStep == 0U)
    {
        g_drawCfg.animStep = 1U;
    }

    if ((g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT) || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_TEXT_SCROLL_JLU))
    {
        g_drawRt.scrollOffset = (uint16_t)(g_drawRt.scrollOffset + g_drawCfg.scrollStep);
    }

    if (((g_drawCfg.effect == DRAWDRV_EFFECT_BREATH)
         || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_IN)
         || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_OUT))
        && (g_drawCfg.timelineDurationMs != 0U))
    {
        DrawDrv_AdvanceEffectTimeline32ms();
    }
    else if ((g_drawCfg.effect == DRAWDRV_EFFECT_BREATH) || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_IN) || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_OUT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_COLOR_CYCLE))
    {
        for (animCnt = 0; animCnt < g_drawCfg.animStep; animCnt++)
        {
            g_drawRt.animPhase++;
        }
    }

    if ((g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT) || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_BREATH) || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_IN) || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_OUT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_COLOR_CYCLE)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_TEXT_SCROLL_JLU))
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
    DrawDrv_ResetRuntimeState();
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

uint8_t DrawDrv_SetTextDisplayGlyph(uint8_t glyphIndex)
{
    if (glyphIndex >= DRAWDRV_JLU_GLYPH_COUNT)
    {
        return 0U;
    }

    g_drawTextDisplayGlyph = glyphIndex;
    g_drawFrameDirty = 1;

    return 1U;
}

uint8_t DrawDrv_SetTextScrollSequence(const uint8_t *glyphList, uint8_t count)
{
    uint8_t idx;
    uint8_t validCount;

    if ((glyphList == 0) || (count == 0U))
    {
        DrawDrv_SetDefaultTextSequence();
        g_drawFrameDirty = 1;
        return 1U;
    }

    validCount = 0U;
    for (idx = 0U; (idx < count) && (idx < DRAWDRV_TEXT_SEQUENCE_MAX); idx++)
    {
        if (glyphList[idx] < DRAWDRV_JLU_GLYPH_COUNT)
        {
            g_drawTextSequence[validCount] = glyphList[idx];
            validCount++;
        }
    }

    if (validCount == 0U)
    {
        return 0U;
    }

    g_drawTextSequenceLen = validCount;
    g_drawRt.scrollOffset = 0U;
    g_drawFrameDirty = 1;

    return 1U;
}
