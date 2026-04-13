#include "config.h"
#include "draw_drv.h"
#include "gp_led_action.h"
#include "test_image.h"
#include "ws2812_drv.h"

static uint8_t g_gpLedRemoteActive = 0U;
static uint8_t g_gpLedDirectFrameActive = 0U;
static uint8_t g_gpLedDefaultBrightness = 200U;
/* Direct-frame rendering shares a single scratch area to reduce EDATA usage. */
static DrawDrv_RenderConfig_t xdata g_gpLedRenderCfg;
static uint8_t xdata g_gpLedTempRow = 0U;
static uint8_t xdata g_gpLedTempCol = 0U;
static uint8_t xdata g_gpLedTempRowMapped = 0U;
static uint8_t xdata g_gpLedTempR = 0U;
static uint8_t xdata g_gpLedTempG = 0U;
static uint8_t xdata g_gpLedTempB = 0U;
static uint16_t xdata g_gpLedTempPixelIndex = 0U;
static uint16_t xdata g_gpLedTempRowBits = 0U;

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

static void GpLedAction_RenderSolidFrame(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    g_gpLedTempR = GpLedAction_ScaleColor(r, brightness);
    g_gpLedTempG = GpLedAction_ScaleColor(g, brightness);
    g_gpLedTempB = GpLedAction_ScaleColor(b, brightness);

    GpLedAction_BeginDirectFrame();
    for (g_gpLedTempRow = 0U; g_gpLedTempRow < GP_MATRIX_HEIGHT; ++g_gpLedTempRow)
    {
        for (g_gpLedTempCol = 0U; g_gpLedTempCol < GP_MATRIX_WIDTH; ++g_gpLedTempCol)
        {
            WS2812DRV_SetPixelRgbFast(g_gpLedTempRow, g_gpLedTempCol, g_gpLedTempR, g_gpLedTempG, g_gpLedTempB);
        }
    }
    GpLedAction_EndDirectFrame();
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
    }
}

void GpLedAction_ReleaseRemoteMode(void)
{
    g_gpLedRemoteActive = 0U;
    g_gpLedDirectFrameActive = 0U;
    DrawDrv_RequestRebuild();
}

uint8_t GpLedAction_IsRemoteModeActive(void)
{
    return g_gpLedRemoteActive;
}

uint8_t GpLedAction_ShouldBypassDrawScheduler(void)
{
    return g_gpLedDirectFrameActive;
}

GpMatrixStatusCode GpLedAction_ApplyAction(const GpMatrixActionPayload xdata *payload)
{
    if (payload == 0)
    {
        return kGpMatrixStatusInternalError;
    }

    if ((payload->flags & GP_MATRIX_ACTION_FLAG_REMOTE_RELEASE) != 0U)
    {
        GpLedAction_ReleaseRemoteMode();
        return kGpMatrixStatusOk;
    }

    if (payload->content == kGpMatrixActionContentSolid)
    {
        if (payload->effect != kGpMatrixEffectStatic)
        {
            return kGpMatrixStatusUnsupported;
        }

        GpLedAction_RenderSolidFrame(payload->primary_r,
                                     payload->primary_g,
                                     payload->primary_b,
                                     payload->brightness);
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

    if (payload->content == kGpMatrixActionContentPattern)
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

GpMatrixStatusCode GpLedAction_ApplyGlyphRows(const uint8_t xdata *glyphData,
                                              uint16_t length,
                                              uint8_t glyphCount,
                                              uint8_t glyphWidth,
                                              uint8_t glyphSpacing)
{
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