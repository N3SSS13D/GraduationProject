#include "config.h"
#include "draw_drv.h"
#include "gp_led_matrix_protocol.h"
#include "offline_pattern.h"
#include "rtc_clock.h"
#include "local_display_assets.h"
#include "ws2812_drv.h"

#define DRAWDRV_IMAGE_SWITCH_TICKS       4U
#define DRAWDRV_JLU_GLYPH_COUNT          LOCALDISPLAY_SCROLL_GLYPH_COUNT
#define DRAWDRV_DEFAULT_TEXT_SEQUENCE_COUNT  LOCALDISPLAY_SCROLL_GLYPH_DEFAULT_SEQUENCE_COUNT
#define DRAWDRV_JLU_GLYPH_WIDTH          LOCALDISPLAY_SCROLL_GLYPH_WIDTH
#define DRAWDRV_JLU_GLYPH_SPACING        LOCALDISPLAY_SCROLL_GLYPH_SPACING
#define DRAWDRV_JLU_GLYPH_ADVANCE        (DRAWDRV_JLU_GLYPH_WIDTH + DRAWDRV_JLU_GLYPH_SPACING)
#define DRAWDRV_TEXT_SEQUENCE_MAX        32U

static DrawDrv_RenderConfig_t g_drawCfg;
static bit g_drawFrameDirty = 0;
static uint8_t g_drawFrameIndex = 0;
static uint8_t xdata g_drawRemotePatternFrame[GP_MATRIX_RGB332_FRAME_SIZE];
static uint8_t g_drawRemotePatternLoaded = 0U;
typedef struct
{
    uint8_t animPhase;
    uint16_t scrollOffset;
    uint16_t timelineElapsedMs;
    uint16_t timelineDelayMs;
    uint8_t timelineRepeatRemain;
    uint8_t timelineActive;
    uint8_t timelineStartPending;
} DrawDrv_RuntimeState_t;
static DrawDrv_RuntimeState_t g_drawRt;
static uint8_t xdata g_drawRgbLutR[256];
static uint8_t xdata g_drawRgbLutG[256];
static uint8_t xdata g_drawRgbLutB[256];
static uint8_t xdata g_drawTextSequence[DRAWDRV_TEXT_SEQUENCE_MAX];
static uint8_t g_drawTextSequenceLen = 0U;
static uint8_t g_drawTextDisplayGlyph = 0U;
static uint8_t xdata g_drawCustomTextGlyphRows[GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES];
static uint8_t g_drawCustomTextGlyphLoaded = 0U;
static uint8_t g_drawCustomTextGlyphSelected = 0U;
static uint8_t g_drawCustomTextGlyphCount = 0U;
static uint8_t g_drawCustomTextGlyphWidth = DRAWDRV_JLU_GLYPH_WIDTH;
static uint8_t g_drawCustomTextGlyphSpacing = DRAWDRV_JLU_GLYPH_SPACING;
/* Reuse shared scratch storage to keep text-pixel lookup off the 8051 stack. */
static uint8_t xdata g_drawTextPixelTextRow = 0U;
static uint8_t xdata g_drawTextPixelGlyphIndex = 0U;
static uint8_t xdata g_drawTextPixelGlyphCol = 0U;
static uint8_t xdata g_drawTextPixelGlyphCount = 0U;
static uint16_t xdata g_drawTextPixelTextWidth = 0U;
static uint16_t xdata g_drawTextPixelVirtualCol = 0U;
static uint16_t xdata g_drawTextPixelRowBits = 0U;
static uint16_t xdata g_drawTextPixelBitMask = 0U;

static uint8_t DrawDrv_IsCustomTextGlyphActive(void)
{
    if ((g_drawCustomTextGlyphLoaded == 0U) || (g_drawCustomTextGlyphSelected == 0U))
    {
        return 0U;
    }

    return 1U;
}

static uint8_t DrawDrv_GetTextGlyphWidth(void)
{
    if (DrawDrv_IsCustomTextGlyphActive() != 0U)
    {
        return g_drawCustomTextGlyphWidth;
    }

    return DRAWDRV_JLU_GLYPH_WIDTH;
}

static uint8_t DrawDrv_GetTextGlyphSpacing(void)
{
    if (DrawDrv_IsCustomTextGlyphActive() != 0U)
    {
        return g_drawCustomTextGlyphSpacing;
    }

    return DRAWDRV_JLU_GLYPH_SPACING;
}

static uint8_t DrawDrv_GetTextGlyphCount(void)
{
    if (DrawDrv_IsCustomTextGlyphActive() != 0U)
    {
        return g_drawCustomTextGlyphCount;
    }

    return DRAWDRV_JLU_GLYPH_COUNT;
}

static uint8_t DrawDrv_GetTextGlyphAdvance(void)
{
    return (uint8_t)(DrawDrv_GetTextGlyphWidth() + DrawDrv_GetTextGlyphSpacing());
}

static uint16_t DrawDrv_GetTextGlyphRowBits(uint8_t glyphIndex, uint8_t row)
{
    uint16_t offset;

    if (row >= LOCALDISPLAY_ASSET_ROWS)
    {
        return 0U;
    }

    if (DrawDrv_IsCustomTextGlyphActive() != 0U)
    {
        if (glyphIndex >= g_drawCustomTextGlyphCount)
        {
            return 0U;
        }

        offset = (uint16_t)glyphIndex * (uint16_t)LOCALDISPLAY_ASSET_ROWS * 2U;
        offset = (uint16_t)(offset + (uint16_t)row * 2U);
        return (uint16_t)g_drawCustomTextGlyphRows[offset]
               | ((uint16_t)g_drawCustomTextGlyphRows[offset + 1U] << 8U);
    }

    if (glyphIndex >= DRAWDRV_JLU_GLYPH_COUNT)
    {
        return 0U;
    }

    return g_localDisplayScrollGlyphRows[glyphIndex][row];
}

uint16_t DrawDrv_NormalizeFrameIntervalMs(uint16_t frameIntervalMs)
{
    if (frameIntervalMs < DRAWDRV_FRAME_INTERVAL_MS_MIN)
    {
        return DRAWDRV_FRAME_INTERVAL_MS_DEFAULT;
    }

    return frameIntervalMs;
}

static uint16_t DrawDrv_GetFrameIntervalMs(void)
{
    return DrawDrv_NormalizeFrameIntervalMs(g_drawCfg.frameIntervalMs);
}

static void DrawDrv_ResetRuntimeState(void)
{
    g_drawRt.animPhase = 0U;
    g_drawRt.scrollOffset = 0U;
    g_drawRt.timelineElapsedMs = 0U;
    g_drawRt.timelineDelayMs = g_drawCfg.timelineRepeatDelayMs;
    g_drawRt.timelineRepeatRemain = g_drawCfg.timelineRepeatCount;
    g_drawRt.timelineActive = (uint8_t)((g_drawCfg.timelineDurationMs != 0U) ? 1U : 0U);
    g_drawRt.timelineStartPending = g_drawRt.timelineActive;
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

static void DrawDrv_AdvanceEffectTimeline(uint16_t stepMs)
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
        && (g_drawCfg.effect != DRAWDRV_EFFECT_FADE_OUT)
        && (g_drawCfg.effect != DRAWDRV_EFFECT_ROW_REVEAL)
        && (g_drawCfg.effect != DRAWDRV_EFFECT_ROW_HIDE)
        && (g_drawCfg.effect != DRAWDRV_EFFECT_GRADIENT_REVEAL))
    {
        return;
    }
    if (g_drawRt.timelineActive == 0U)
    {
        return;
    }

    /* Show the effect's start endpoint for one full draw period before advancing into the row-by-row motion. */
    if (g_drawRt.timelineStartPending != 0U)
    {
        g_drawRt.timelineStartPending = 0U;
        return;
    }

    /* Repeating one-shot effects must keep their terminal frame for one draw tick.
       Otherwise row reveal/hide resets to the next cycle before the last rows are ever shown or cleared. */
    if (g_drawRt.timelineElapsedMs >= duration)
    {
        if (g_drawRt.timelineRepeatRemain == 0U)
        {
            g_drawRt.timelineActive = 0U;
            return;
        }

        if (g_drawRt.timelineRepeatRemain != 0xFFU)
        {
            g_drawRt.timelineRepeatRemain--;
        }

        g_drawRt.timelineElapsedMs = 0U;
        g_drawRt.timelineDelayMs = g_drawCfg.timelineRepeatDelayMs;
        g_drawRt.animPhase = 0U;
        return;
    }

    if (g_drawRt.timelineDelayMs != 0U)
    {
        if (g_drawRt.timelineDelayMs > stepMs)
        {
            g_drawRt.timelineDelayMs = (uint16_t)(g_drawRt.timelineDelayMs - stepMs);
            return;
        }

        g_drawRt.timelineDelayMs = 0U;
    }

    elapsed = (uint16_t)(g_drawRt.timelineElapsedMs + stepMs);
    if (elapsed >= duration)
    {
        elapsed = duration;
        if (g_drawRt.timelineRepeatRemain == 0U)
        {
            g_drawRt.timelineActive = 0U;
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
    if ((DrawDrv_IsCustomTextGlyphActive() != 0U) && (g_drawTextSequenceLen == 0U))
    {
        return g_drawCustomTextGlyphCount;
    }

    if (g_drawTextSequenceLen == 0U)
    {
        return DRAWDRV_JLU_GLYPH_COUNT;
    }

    return g_drawTextSequenceLen;
}

static uint8_t DrawDrv_GetTextSequenceGlyph(uint8_t seqIndex)
{
    uint8_t idx;

    if ((DrawDrv_IsCustomTextGlyphActive() != 0U) && (g_drawTextSequenceLen == 0U))
    {
        if (g_drawCustomTextGlyphCount == 0U)
        {
            return 0U;
        }

        return (uint8_t)(seqIndex % g_drawCustomTextGlyphCount);
    }

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
    return (uint16_t)DrawDrv_GetTextSequenceGlyphCount() * DrawDrv_GetTextGlyphAdvance();
}

static void DrawDrv_SetDefaultTextSequence(void)
{
    uint8_t idx;

    g_drawTextSequenceLen = DRAWDRV_DEFAULT_TEXT_SEQUENCE_COUNT;
    if (g_drawTextSequenceLen > DRAWDRV_JLU_GLYPH_COUNT)
    {
        g_drawTextSequenceLen = DRAWDRV_JLU_GLYPH_COUNT;
    }
    if (g_drawTextSequenceLen > DRAWDRV_TEXT_SEQUENCE_MAX)
    {
        g_drawTextSequenceLen = DRAWDRV_TEXT_SEQUENCE_MAX;
    }

    for (idx = 0U; idx < g_drawTextSequenceLen; idx++)
    {
        g_drawTextSequence[idx] = idx;
    }

    g_drawTextDisplayGlyph = 0U;
    g_drawCustomTextGlyphSelected = 0U;
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
    g_drawCfg.frameIntervalMs = DRAWDRV_FRAME_INTERVAL_MS_DEFAULT;
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
    uint8_t patternCount;

    patternCount = OfflinePattern_GetCount();
    if (g_drawRemotePatternLoaded != 0U)
    {
        patternCount++;
    }

    return patternCount;
}

static uint8_t DrawDrv_GetRemotePatternPixel(uint8_t row, uint8_t col)
{
    uint8_t imageRow;
    uint16_t pixelIndex;

    if ((g_drawRemotePatternLoaded == 0U)
        || (row >= LOCALDISPLAY_ASSET_ROWS)
        || (col >= LOCALDISPLAY_ASSET_COLS))
    {
        return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
    }

    imageRow = (uint8_t)((LOCALDISPLAY_ASSET_ROWS - 1U) - row);
    pixelIndex = (uint16_t)imageRow * LOCALDISPLAY_ASSET_COLS + col;
    return g_drawRemotePatternFrame[pixelIndex];
}

static uint8_t DrawDrv_GetPatternPixel(uint8_t imageIndex, uint8_t row, uint8_t col)
{
    uint8_t offlineCount;

    offlineCount = OfflinePattern_GetCount();
    if (imageIndex < offlineCount)
    {
        return OfflinePattern_GetPixel(imageIndex, row, col);
    }

    if ((g_drawRemotePatternLoaded != 0U) && (imageIndex == offlineCount))
    {
        return DrawDrv_GetRemotePatternPixel(row, col);
    }

    return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
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
            *srcRow = (uint8_t)((LOCALDISPLAY_ASSET_ROWS - 1U) - row);
            *srcCol = (uint8_t)((LOCALDISPLAY_ASSET_COLS - 1U) - col);
            break;

        case DRAWDRV_DIR_ROTATE_CW_90:
            *srcRow = (uint8_t)((LOCALDISPLAY_ASSET_ROWS - 1U) - col);
            *srcCol = row;
            break;

        case DRAWDRV_DIR_ROTATE_CCW_90:
            *srcRow = col;
            *srcCol = (uint8_t)((LOCALDISPLAY_ASSET_COLS - 1U) - row);
            break;

        case DRAWDRV_DIR_NORMAL:
        default:
            *srcRow = row;
            *srcCol = col;
            break;
    }
}

static uint8_t DrawDrv_TryMapTextScrollCol(uint16_t textWidth, uint8_t col, uint16_t *virtualCol)
{
    uint16_t cycleWidth;
    uint16_t offset;

    if ((virtualCol == 0) || (textWidth == 0U))
    {
        return 0U;
    }

    if (g_drawCfg.effect != DRAWDRV_EFFECT_TEXT_SCROLL_JLU)
    {
        offset = (uint16_t)(g_drawRt.scrollOffset % textWidth);
        *virtualCol = (uint16_t)col + offset;
        if (*virtualCol >= textWidth)
        {
            *virtualCol = (uint16_t)(*virtualCol - textWidth);
        }

        return 1U;
    }

    /* Keep a full-screen blank tail after the last glyph column so the local marquee exits cleanly before looping. */
    cycleWidth = (uint16_t)(textWidth + LOCALDISPLAY_ASSET_COLS);
    if (cycleWidth == 0U)
    {
        return 0U;
    }

    offset = (uint16_t)(g_drawRt.scrollOffset % cycleWidth);
    *virtualCol = (uint16_t)col + offset;
    if (*virtualCol >= textWidth)
    {
        return 0U;
    }

    return 1U;
}

static uint8_t DrawDrv_GetJluTextPixel(uint8_t row, uint8_t col)
{
    uint8_t glyphAdvance;

    if ((row >= LOCALDISPLAY_ASSET_ROWS) || (col >= LOCALDISPLAY_ASSET_COLS))
    {
        return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
    }

    if ((g_drawCfg.effect == DRAWDRV_EFFECT_TEXT_SCROLL_JLU)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT))
    {
        g_drawTextPixelTextWidth = DrawDrv_GetTextVirtualWidth();
        if (g_drawTextPixelTextWidth == 0U)
        {
            return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
        }

        if (DrawDrv_TryMapTextScrollCol(g_drawTextPixelTextWidth, col, &g_drawTextPixelVirtualCol) == 0U)
        {
            return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
        }

        g_drawTextPixelGlyphCount = DrawDrv_GetTextSequenceGlyphCount();
        if (g_drawTextPixelGlyphCount == 0U)
        {
            return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
        }

        glyphAdvance = DrawDrv_GetTextGlyphAdvance();
        if (glyphAdvance == 0U)
        {
            return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
        }

        g_drawTextPixelGlyphIndex = DrawDrv_GetTextSequenceGlyph((uint8_t)(g_drawTextPixelVirtualCol / glyphAdvance));
        g_drawTextPixelGlyphCol = (uint8_t)(g_drawTextPixelVirtualCol % glyphAdvance);
    }
    else
    {
        g_drawTextPixelGlyphIndex = g_drawTextDisplayGlyph;
        g_drawTextPixelGlyphCol = col;
    }

    if ((g_drawTextPixelGlyphIndex >= DrawDrv_GetTextGlyphCount())
        || (g_drawTextPixelGlyphCol >= DrawDrv_GetTextGlyphWidth()))
    {
        return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
    }

    /* Physical LED rows are indexed bottom-to-top (0..15). */
    g_drawTextPixelTextRow = (uint8_t)((LOCALDISPLAY_ASSET_ROWS - 1U) - row);
    g_drawTextPixelRowBits = DrawDrv_GetTextGlyphRowBits(g_drawTextPixelGlyphIndex, g_drawTextPixelTextRow);
    g_drawTextPixelBitMask = (uint16_t)(0x8000U >> g_drawTextPixelGlyphCol);

    if ((g_drawTextPixelRowBits & g_drawTextPixelBitMask) != 0U)
    {
        return 0xFFU;
    }

    return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
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

    /* Clock content emits region-specific RGB332 colors directly, so keep its decoded foreground color intact. */
    if (g_drawCfg.contentType == DRAWDRV_CONTENT_CLOCK)
    {
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

static uint8_t DrawDrv_GetRevealVisibleRows(void)
{
    uint8_t phase;
    uint16_t scaledRows;

    phase = (uint8_t)(g_drawRt.animPhase & 0x1FU);
    scaledRows = (uint16_t)phase * LOCALDISPLAY_ASSET_ROWS;
    scaledRows /= 31U;
    if (scaledRows > LOCALDISPLAY_ASSET_ROWS)
    {
        scaledRows = LOCALDISPLAY_ASSET_ROWS;
    }

    return (uint8_t)scaledRows;
}

static uint8_t DrawDrv_ShouldMaskRowEffectPixel(uint8_t row, uint8_t isBg)
{
    uint8_t visibleRows;
    uint8_t hiddenRows;

    if (isBg != 0U)
    {
        return 0U;
    }

    if ((g_drawCfg.effect != DRAWDRV_EFFECT_ROW_REVEAL)
        && (g_drawCfg.effect != DRAWDRV_EFFECT_ROW_HIDE)
        && (g_drawCfg.effect != DRAWDRV_EFFECT_GRADIENT_REVEAL))
    {
        return 0U;
    }

    visibleRows = DrawDrv_GetRevealVisibleRows();
    if ((g_drawCfg.effect == DRAWDRV_EFFECT_ROW_REVEAL)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT_REVEAL))
    {
        if (row >= visibleRows)
        {
            return 1U;
        }

        return 0U;
    }

    hiddenRows = visibleRows;
    if (hiddenRows > LOCALDISPLAY_ASSET_ROWS)
    {
        hiddenRows = LOCALDISPLAY_ASSET_ROWS;
    }

    if (row < hiddenRows)
    {
        return 1U;
    }

    return 0U;
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
    if (activeCols > LOCALDISPLAY_ASSET_COLS)
    {
        activeCols = LOCALDISPLAY_ASSET_COLS;
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
            else if (g_drawCfg.contentType == DRAWDRV_CONTENT_CLOCK)
            {
                packed = RtcClock_GetPixel(mappedRow, mappedCol);
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

            isBg = (uint8_t)(packed == (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332);
            if (DrawDrv_ShouldMaskRowEffectPixel(row, isBg) != 0U)
            {
                packed = (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
                isBg = 1U;
            }

            DrawDrv_DecodeRgb332(packed, &r, &g, &b);
            DrawDrv_ApplyColorConfig((uint8_t)(isBg == 0U), &r, &g, &b);

            /* Background keeps plain color, no gradient/breath/scroll enhancement. */
            if ((isBg == 0U) && ((g_drawCfg.useGradient != 0U)
                || (g_drawCfg.colorMode == DRAWDRV_COLOR_GRADIENT)
                || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT)
                || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT_REVEAL)))
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
    (void)RtcClock_Init();
    g_drawFrameDirty = 1;
    g_drawFrameIndex = 0;
    g_drawRemotePatternLoaded = 0U;
    DrawDrv_ResetRuntimeState();

    DrawDrv_RebuildFrame();
    g_drawFrameDirty = 0;
}

void DrawDrv_Task(void)
{
    uint16_t frameIntervalMs;
    uint8_t animCnt;

    frameIntervalMs = DrawDrv_GetFrameIntervalMs();

    if (g_drawCfg.scrollStep == 0U)
    {
        g_drawCfg.scrollStep = 1U;
    }

    if (g_drawCfg.animStep == 0U)
    {
        g_drawCfg.animStep = 1U;
    }

    /* Clock content is cached locally and advances using the active draw cadence between Bluetooth sync packets. */
    if ((g_drawCfg.contentType == DRAWDRV_CONTENT_CLOCK) && (RtcClock_Task(frameIntervalMs) != 0U))
    {
        g_drawFrameDirty = 1;
    }

    if ((g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_LEFT) || (g_drawCfg.effect == DRAWDRV_EFFECT_SCROLL_RIGHT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_TEXT_SCROLL_JLU))
    {
        g_drawRt.scrollOffset = (uint16_t)(g_drawRt.scrollOffset + g_drawCfg.scrollStep);
    }

    if (((g_drawCfg.effect == DRAWDRV_EFFECT_BREATH)
         || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_IN)
         || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_OUT)
         || (g_drawCfg.effect == DRAWDRV_EFFECT_ROW_REVEAL)
         || (g_drawCfg.effect == DRAWDRV_EFFECT_ROW_HIDE)
         || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT_REVEAL))
        && (g_drawCfg.timelineDurationMs != 0U))
    {
        DrawDrv_AdvanceEffectTimeline(frameIntervalMs);
    }
    else if ((g_drawCfg.effect == DRAWDRV_EFFECT_BREATH) || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_IN) || (g_drawCfg.effect == DRAWDRV_EFFECT_FADE_OUT)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_COLOR_CYCLE)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_ROW_REVEAL)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_ROW_HIDE)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT_REVEAL))
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
        || (g_drawCfg.effect == DRAWDRV_EFFECT_ROW_REVEAL)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_ROW_HIDE)
        || (g_drawCfg.effect == DRAWDRV_EFFECT_GRADIENT_REVEAL)
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

void DrawDrv_Task32ms(void)
{
    DrawDrv_Task();
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
    g_drawCfg.frameIntervalMs = DrawDrv_NormalizeFrameIntervalMs(g_drawCfg.frameIntervalMs);
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

DrawDrv_RenderConfig_t *DrawDrv_GetRenderConfigStorage(void)
{
    return &g_drawCfg;
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

uint8_t DrawDrv_LoadRemotePatternFrame(const uint8_t *frameData, uint16_t length)
{
    uint16_t copyIndex;

    if ((frameData == 0) || (length < GP_MATRIX_RGB332_FRAME_SIZE))
    {
        return 0U;
    }

    for (copyIndex = 0U; copyIndex < GP_MATRIX_RGB332_FRAME_SIZE; ++copyIndex)
    {
        g_drawRemotePatternFrame[copyIndex] = frameData[copyIndex];
    }

    g_drawRemotePatternLoaded = 1U;
    g_drawFrameDirty = 1;
    return 1U;
}

uint8_t DrawDrv_HasRemotePattern(void)
{
    return g_drawRemotePatternLoaded;
}

uint8_t DrawDrv_IsRemotePatternActive(void)
{
    if ((g_drawRemotePatternLoaded == 0U) || (g_drawCfg.contentType != DRAWDRV_CONTENT_PATTERN))
    {
        return 0U;
    }

    return (uint8_t)(g_drawFrameIndex == OfflinePattern_GetCount());
}

uint8_t DrawDrv_SetTextDisplayGlyph(uint8_t glyphIndex)
{
    if (glyphIndex >= DrawDrv_GetTextGlyphCount())
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
    g_drawCustomTextGlyphSelected = 0U;
    g_drawRt.scrollOffset = 0U;
    g_drawFrameDirty = 1;

    return 1U;
}

uint8_t DrawDrv_LoadCustomTextGlyphRows(const uint8_t *glyphData,
                                        uint16_t length,
                                        uint8_t glyphCount,
                                        uint8_t glyphWidth,
                                        uint8_t glyphSpacing)
{
    uint16_t expectedBytes;
    uint16_t byteIndex;

    if ((glyphData == 0) || (glyphCount == 0U))
    {
        return 0U;
    }
    if ((glyphWidth == 0U)
        || (glyphWidth > LOCALDISPLAY_ASSET_COLS)
        || (glyphSpacing > LOCALDISPLAY_ASSET_COLS))
    {
        return 0U;
    }

    expectedBytes = (uint16_t)glyphCount * (uint16_t)LOCALDISPLAY_ASSET_ROWS * 2U;
    if ((length != expectedBytes) || (expectedBytes > GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES))
    {
        return 0U;
    }

    for (byteIndex = 0U; byteIndex < expectedBytes; ++byteIndex)
    {
        g_drawCustomTextGlyphRows[byteIndex] = glyphData[byteIndex];
    }

    g_drawCustomTextGlyphLoaded = 1U;
    g_drawCustomTextGlyphCount = glyphCount;
    g_drawCustomTextGlyphWidth = glyphWidth;
    g_drawCustomTextGlyphSpacing = glyphSpacing;
    g_drawTextDisplayGlyph = 0U;
    g_drawTextSequenceLen = 0U;
    g_drawRt.scrollOffset = 0U;
    g_drawFrameDirty = 1;

    return 1U;
}

uint8_t DrawDrv_SelectCustomTextGlyphRows(uint8_t enable)
{
    if ((enable != 0U) && (g_drawCustomTextGlyphLoaded == 0U))
    {
        return 0U;
    }

    g_drawCustomTextGlyphSelected = (uint8_t)((enable != 0U) ? 1U : 0U);
    g_drawTextDisplayGlyph = 0U;
    g_drawRt.scrollOffset = 0U;
    g_drawFrameDirty = 1;

    return 1U;
}
