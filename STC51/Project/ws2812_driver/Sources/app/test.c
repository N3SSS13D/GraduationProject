#include "config.h"
#include "test.h"
#include "ws2812_drv.h"

#define TEST_ROW_INTERVAL_US_DEFAULT     1200UL
#define TEST_ROW_INTERVAL_US_MIN         1000UL
#define TEST_ROW_INTERVAL_US_MAX         999000000UL
static volatile bit g_testRowUpdatePending = 1;

static uint8_t g_testScanRowIndex = 0;
static uint32_t g_testRowIntervalUs = TEST_ROW_INTERVAL_US_DEFAULT;
static uint16_t g_testLastPwmUs = 0;

static void Test_OnRowTimerExpired(void)
{
    g_testRowUpdatePending = 1;
}

static void Test_BuildFixedTrapezoidImage(void)
{
    uint8_t row;
    uint8_t col;
    uint8_t litCount;

    /* Build fixed 16x8 trapezoid in image buffer, then let driver encode all rows. */
    WS2812DRV_ClearImage();

    for (row = 0; row < WS2812DRV_ROW_NUM; row++)
    {
        if (row < 8U)
        {
            litCount = (uint8_t)(row + 1U);
        }
        else
        {
            litCount = (uint8_t)(16U - row);
        }

        for (col = 0; col < WS2812DRV_COL_NUM; col++)
        {
            if (col < litCount)
            {
                WS2812DRV_SetPixelRgb(row, col, 0x00, 0xFF, 0x00);
            }
            else
            {
                WS2812DRV_SetPixelRgb(row, col, 0x00, 0x00, 0x00);
            }
        }
    }

    WS2812DRV_EncodeAllRows();
}

void Test_Init(void)
{
    /* All PWM/DMA low-level initialization is delegated to ws2812 driver. */
    WS2812DRV_Init();
    Test_BuildFixedTrapezoidImage();

    g_testScanRowIndex = 0;
    g_testRowUpdatePending = 1;
    g_testLastPwmUs = 0;

    /* Timer0 hook only sets scan pending flag to keep ISR path deterministic. */
    TIMER0_RegisterUsHook(Test_OnRowTimerExpired);
    TIMER0_StartOneShotUs(g_testRowIntervalUs);
}

void Test_TaskLoop(void)
{
    uint8_t rowA;
    uint8_t rowB;
    if (WS2812DRV_IsDmaBusy() != 0)
    {
        return;
    }

    if (g_testRowUpdatePending == 0)
    {
        return;
    }

    g_testRowUpdatePending = 0;

    rowA = g_testScanRowIndex;
    rowB = (uint8_t)(g_testScanRowIndex + 1U);

    /* Send one row pair using unified ws2812 driver path. */
    if (WS2812DRV_SendRowPair(rowA, rowB) == 0)
    {
        TIMER0_StartOneShotUs(g_testRowIntervalUs);

        return;
    }

    g_testLastPwmUs = 0;

    g_testScanRowIndex = (uint8_t)(g_testScanRowIndex + 2U);
    if (g_testScanRowIndex >= WS2812DRV_ROW_NUM)
    {
        g_testScanRowIndex = 0;
    }

    TIMER0_StartOneShotUs(g_testRowIntervalUs);
}

void Test_SetRowIntervalUs(uint32_t intervalUs)
{
    if (intervalUs < TEST_ROW_INTERVAL_US_MIN)
    {
        intervalUs = TEST_ROW_INTERVAL_US_MIN;
    }
    if (intervalUs > TEST_ROW_INTERVAL_US_MAX)
    {
        intervalUs = TEST_ROW_INTERVAL_US_MAX;
    }

    g_testRowIntervalUs = intervalUs;
}

uint32_t Test_GetRowIntervalUs(void)
{
    return g_testRowIntervalUs;
}

uint16_t Test_GetLastPwmUs(void)
{
    return g_testLastPwmUs;
}

void PWMAT_DMA_ISR(void) interrupt DMA_PWMAT_VECTOR
{
    /* Forward DMA completion to ws2812 driver for unified state management. */
    WS2812DRV_OnDmaIsr();
}
