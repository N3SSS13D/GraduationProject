#include "config.h"
#include "draw_drv.h"
#include "ws2812_drv.h"

#define DRAWDRV_TRAPEZOID_COLOR_R       0x00
#define DRAWDRV_TRAPEZOID_COLOR_G       0xFF
#define DRAWDRV_TRAPEZOID_COLOR_B       0x00
#define DRAWDRV_DEBUG_ROW_COLOR_R       0x00
#define DRAWDRV_DEBUG_ROW_COLOR_G       0xFF
#define DRAWDRV_DEBUG_ROW_COLOR_B       0x00

static uint8_t g_drawVisibleRows = 1;
static uint8_t g_drawIsShowing = 1;
static bit g_drawFrameDirty = 0;
static bit g_drawSingleRowDebugEnabled = 0;
static uint8_t g_drawSingleRowDebugRow = 0;

static uint8_t DrawDrv_GetTrapezoidLitCount(uint8_t row)
{
    if (row < 8U)
    {
        return (uint8_t)(row + 1U);
    }

    return (uint8_t)(16U - row);
}

static void DrawDrv_RebuildTrapezoidByVisibleRows(uint8_t visibleRows)
{
    uint8_t row;
    uint8_t col;
    uint8_t litCount;

    if (visibleRows > WS2812DRV_ROW_NUM)
    {
        visibleRows = WS2812DRV_ROW_NUM;
    }

    WS2812DRV_ClearImage();

    for (row = 0; row < visibleRows; row++)
    {
        litCount = DrawDrv_GetTrapezoidLitCount(row);
        for (col = 0; col < litCount; col++)
        {
            WS2812DRV_SetPixelRgb(row, col,
                DRAWDRV_TRAPEZOID_COLOR_R,
                DRAWDRV_TRAPEZOID_COLOR_G,
                DRAWDRV_TRAPEZOID_COLOR_B);
        }
    }

    WS2812DRV_EncodeAllRows();
}

static void DrawDrv_RebuildSingleRowDebug(uint8_t row)
{
    uint8_t col;

    if (row >= WS2812DRV_ROW_NUM)
    {
        return;
    }

    WS2812DRV_ClearImage();
    for (col = 0; col < WS2812DRV_COL_NUM; col++)
    {
        WS2812DRV_SetPixelRgb(row, col,
            DRAWDRV_DEBUG_ROW_COLOR_R,
            DRAWDRV_DEBUG_ROW_COLOR_G,
            DRAWDRV_DEBUG_ROW_COLOR_B);
    }

    WS2812DRV_EncodeAllRows();
}

void DrawDrv_Init(void)
{
    g_drawVisibleRows = 1;
    g_drawIsShowing = 1;
    g_drawFrameDirty = 1;
    g_drawSingleRowDebugEnabled = 0;
    g_drawSingleRowDebugRow = 0;

    DrawDrv_RebuildTrapezoidByVisibleRows(g_drawVisibleRows);
    g_drawFrameDirty = 0;
}

void DrawDrv_Task40ms(void)
{
    /* Keep 25fps task slot, but only rebuild when state changed. */
    if (g_drawFrameDirty == 0)
    {
        return;
    }

    if (g_drawSingleRowDebugEnabled != 0)
    {
        DrawDrv_RebuildSingleRowDebug(g_drawSingleRowDebugRow);
    }
    else
    {
        DrawDrv_RebuildTrapezoidByVisibleRows(g_drawVisibleRows);
    }

    g_drawFrameDirty = 0;
}

void DrawDrv_Task500ms(void)
{
    if (g_drawSingleRowDebugEnabled != 0)
    {
        return;
    }

    if (g_drawIsShowing != 0)
    {
        if (g_drawVisibleRows < WS2812DRV_ROW_NUM)
        {
            g_drawVisibleRows++;
        }
        else
        {
            g_drawIsShowing = 0;
            g_drawVisibleRows = (uint8_t)(WS2812DRV_ROW_NUM - 1U);
        }
    }
    else
    {
        if (g_drawVisibleRows > 0U)
        {
            g_drawVisibleRows--;
        }
        else
        {
            g_drawIsShowing = 1;
            g_drawVisibleRows = 1;
        }
    }

    g_drawFrameDirty = 1;
}

uint8_t DrawDrv_EnableSingleRowDebug(uint8_t row)
{
    if (row >= WS2812DRV_ROW_NUM)
    {
        return 0;
    }

    g_drawSingleRowDebugRow = row;
    g_drawSingleRowDebugEnabled = 1;
    g_drawFrameDirty = 1;

    return 1;
}

void DrawDrv_DisableSingleRowDebug(void)
{
    g_drawSingleRowDebugEnabled = 0;
    g_drawFrameDirty = 1;
}
