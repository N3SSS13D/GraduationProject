/*
 * @file gp_led_matrix_usb_debug.c
 * @brief Temporary USB/key DEBUG handler: stops normal scan, locks two HC595 rows ON,
 *        cycles all-white 5s / all-black 5s on rows 0-1.
 *        Enter via USB "DEBUG" or P32+P33 2s combo; exit same combo during loop.
 */

#include "config.h"
#include "gp_led_matrix_usb_debug.h"
#include "app.h"
#include "ws2812_drv.h"
#include "hc595_drv.h"

#define GP_USB_DEBUG_CYCLE_MS       5000U
#define GP_USB_DEBUG_POLL_MS        100U
#define GP_USB_DEBUG_EXIT_TICKS     20U
#define GP_USB_DEBUG_ROW_A          0U
#define GP_USB_DEBUG_ROW_B          1U

static void GpLedMatrixUsbDebug_SendRowColor(uint8_t r, uint8_t g, uint8_t b);
static uint8_t GpLedMatrixUsbDebug_PollExit(void);

static volatile uint8_t g_gpUsbDebugActive = 0U;

void GpLedMatrixUsbDebug_Enter(void)
{
    g_gpUsbDebugActive = 1U;
}

void GpLedMatrixUsbDebug_Exit(void)
{
    g_gpUsbDebugActive = 0U;
}

void GpLedMatrixUsbDebug_Toggle(void)
{
    if (g_gpUsbDebugActive != 0U)
    {
        g_gpUsbDebugActive = 0U;
        printf("[DEBUG] Toggle: exit requested\r\n");
    }
    else
    {
        g_gpUsbDebugActive = 1U;
        printf("[DEBUG] Toggle: enter requested\r\n");
    }
}

static void GpLedMatrixUsbDebug_SendRowColor(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t col;
    uint8_t activeCols;

    activeCols = WS2812DRV_GetActiveCols();
    WS2812DRV_BeginFrameWrite();
    for (col = 0; col < activeCols; col++)
    {
        WS2812DRV_SetPixelRgbFast(GP_USB_DEBUG_ROW_A, col, r, g, b);
        WS2812DRV_SetPixelRgbFast(GP_USB_DEBUG_ROW_B, col, r, g, b);
    }
    WS2812DRV_EndFrameWrite();
    WS2812DRV_EncodeAllRows();
    (void)WS2812DRV_CommitAndSendRowPair(GP_USB_DEBUG_ROW_A, GP_USB_DEBUG_ROW_B);
}

static uint8_t GpLedMatrixUsbDebug_PollExit(void)
{
    uint16_t pollCount;
    uint16_t pressTicks;

    pressTicks = 0U;
    for (pollCount = 0U; pollCount < (GP_USB_DEBUG_CYCLE_MS / GP_USB_DEBUG_POLL_MS); pollCount++)
    {
        delay_ms(GP_USB_DEBUG_POLL_MS);

        if ((P32 == 0) && (P33 == 0))
        {
            pressTicks++;
            if (pressTicks >= GP_USB_DEBUG_EXIT_TICKS)
            {
                return 1U;
            }
        }
        else
        {
            pressTicks = 0U;
        }

        /* Honour exit flag set externally (e.g. USB command or key task toggle). */
        if (g_gpUsbDebugActive == 0U)
        {
            return 1U;
        }
    }

    return 0U;
}

void GpLedMatrixUsbDebug_Run(void)
{
    if (g_gpUsbDebugActive == 0U)
    {
        return;
    }

    printf("[DEBUG] Entering LED debug mode (rows 0-1, white/black cycle 5s)\r\n");

    /* Stop normal row scan so RefreshStep won't interfere. */
    TIMER1_Stop();
    TIMER1_DisableInt();

    while (g_gpUsbDebugActive != 0U)
    {
        /* Phase A: two rows all white. */
        printf("[DEBUG] White ON (rows 0-1)\r\n");
        GpLedMatrixUsbDebug_SendRowColor(0xFF, 0xFF, 0xFF);
        if (GpLedMatrixUsbDebug_PollExit() != 0U)
        {
            break;
        }

        /* Phase B: two rows all black. */
        printf("[DEBUG] Black OFF (rows 0-1)\r\n");
        GpLedMatrixUsbDebug_SendRowColor(0x00, 0x00, 0x00);
        if (GpLedMatrixUsbDebug_PollExit() != 0U)
        {
            break;
        }
    }

    /* Restore clean state: rows off and resume normal scan. */
    HC595_AllOff();
    WS2812DRV_StopPwmDualChannels();
    printf("[DEBUG] Exited debug mode\r\n");

    /* Let the caller re-apply normal Timer1 settings. */
    APP_RestoreNormalScan();
}
