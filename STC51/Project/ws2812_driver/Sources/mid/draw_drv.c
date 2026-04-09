#include "config.h"
#include "draw_drv.h"
#include "test_image.h"
#include "ws2812_drv.h"

#define DRAWDRV_FRAME_COLOR_R           0x00
#define DRAWDRV_FRAME_COLOR_G           0xFF
#define DRAWDRV_FRAME_COLOR_B           0x00
#define DRAWDRV_DEBUG_ROW_COLOR_R       0x00
#define DRAWDRV_DEBUG_ROW_COLOR_G       0xFF
#define DRAWDRV_DEBUG_ROW_COLOR_B       0x00

static uint8_t g_drawVisibleRows = 1;
static uint8_t g_drawIsShowing = 1;
static bit g_drawFrameDirty = 0;
static bit g_drawSingleRowDebugEnabled = 0;
static uint8_t g_drawSingleRowDebugRow = 0;
static uint8_t g_drawFrameIndex = 0;

static void DrawDrv_RebuildFrameByVisibleRows(uint8_t frameIndex, uint8_t visibleRows)
{
    uint8_t row;
    uint8_t col;
    uint8_t activeCols;

    if (visibleRows > WS2812DRV_ROW_NUM)
    {
        visibleRows = WS2812DRV_ROW_NUM;
    }

    if (frameIndex >= TEST_IMAGE_COUNT)
    {
        frameIndex = 0;
    }

    activeCols = WS2812DRV_GetActiveCols();
    if (activeCols > TEST_IMAGE_COLS)
    {
        activeCols = TEST_IMAGE_COLS;
    }

    WS2812DRV_ClearImage();

    for (row = 0; row < visibleRows; row++)
    {
        for (col = 0; col < activeCols; col++)
        {
            if (g_testImage16x16[frameIndex][row][col] != 0U)
            {
                WS2812DRV_SetPixelRgb(row, col,
                    DRAWDRV_FRAME_COLOR_R,
                    DRAWDRV_FRAME_COLOR_G,
                    DRAWDRV_FRAME_COLOR_B);
            }
        }
    }

    WS2812DRV_EncodeAllRows();
}

static void DrawDrv_RebuildSingleRowDebug(uint8_t row)
{
    uint8_t col;
    uint8_t activeCols;

    if (row >= WS2812DRV_ROW_NUM)
    {
        return;
    }

    activeCols = WS2812DRV_GetActiveCols();
    if (activeCols > TEST_IMAGE_COLS)
    {
        activeCols = TEST_IMAGE_COLS;
    }

    WS2812DRV_ClearImage();
    for (col = 0; col < activeCols; col++)
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
    g_drawFrameIndex = 0;

    DrawDrv_RebuildFrameByVisibleRows(g_drawFrameIndex, g_drawVisibleRows);
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
        DrawDrv_RebuildFrameByVisibleRows(g_drawFrameIndex, g_drawVisibleRows);
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
            g_drawFrameIndex++;
            if (g_drawFrameIndex >= TEST_IMAGE_COUNT)
            {
                g_drawFrameIndex = 0;
            }
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

void DrawDrv_RequestRebuild(void)
{
    g_drawFrameDirty = 1;
}
