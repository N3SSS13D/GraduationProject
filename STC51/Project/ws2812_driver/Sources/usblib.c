//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// File name: usblib.c
// File desc: Minimal USB control for ws2812 test
// File ver : V1.0
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>

#include "config.h"

//<<AICUBE_USER_INCLUDE_BEGIN>>
#include "test.h"
//<<AICUBE_USER_INCLUDE_END>>

//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
static uint8_t USBLIB_ParseIntervalCommand(uint8_t *buf, uint8_t len, uint32_t *intervalUs)
{
    uint8_t i;
    uint16_t value;

    for (i = 0; (uint8_t)(i + 7) <= len; i++)
    {
        if ((buf[i] == 'T') && (buf[i + 1] == '=')
            && (buf[i + 2] >= '0') && (buf[i + 2] <= '9')
            && (buf[i + 3] >= '0') && (buf[i + 3] <= '9')
            && (buf[i + 4] >= '0') && (buf[i + 4] <= '9')
            && (buf[i + 5] >= '0') && (buf[i + 5] <= '9'))
        {
            value = (uint16_t)((buf[i + 2] - '0') * 1000 + (buf[i + 3] - '0') * 100
                + (buf[i + 4] - '0') * 10 + (buf[i + 5] - '0'));

            if ((uint8_t)(i + 8) <= len)
            {
                if ((buf[i + 6] == 'u') && (buf[i + 7] == 's'))
                {
                    *intervalUs = (uint32_t)value;
                    return 1;
                }
                if ((buf[i + 6] == 'm') && (buf[i + 7] == 's'))
                {
                    *intervalUs = (uint32_t)value * 1000U;
                    return 1;
                }
            }

            if (buf[i + 6] == 's')
            {
                *intervalUs = (uint32_t)value * 1000000U;
                return 1;
            }
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseRowCommand(uint8_t *buf, uint8_t len, uint8_t *isOff, uint8_t *row)
{
    uint8_t i;
    uint8_t value;

    for (i = 0; (uint8_t)(i + 5) <= len; i++)
    {
        if ((buf[i] == 'R') && (buf[i + 1] == 'O') && (buf[i + 2] == 'W') && (buf[i + 3] == '='))
        {
            if ((uint8_t)(i + 7) <= len)
            {
                if ((buf[i + 4] == 'O') && (buf[i + 5] == 'F') && (buf[i + 6] == 'F'))
                {
                    *isOff = 1;
                    *row = 0;

                    return 1;
                }
            }

            if ((buf[i + 4] < '0') || (buf[i + 4] > '9'))
            {
                return 0;
            }

            value = (uint8_t)(buf[i + 4] - '0');
            if ((uint8_t)(i + 6) <= len)
            {
                if ((buf[i + 5] >= '0') && (buf[i + 5] <= '9'))
                {
                    value = (uint8_t)(value * 10U + (uint8_t)(buf[i + 5] - '0'));
                }
            }

            if (value >= 16U)
            {
                return 0;
            }

            *isOff = 0;
            *row = value;

            return 1;
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseModeCommand(uint8_t *buf, uint8_t len, uint8_t *mode16x)
{
    uint8_t i;

    for (i = 0; (uint8_t)(i + 6) <= len; i++)
    {
        if ((buf[i] == 'M') && (buf[i + 1] == '=')
            && (buf[i + 2] == '0') && (buf[i + 3] == '0')
            && (buf[i + 4] == '0') && (buf[i + 5] == '8'))
        {
            *mode16x = 8U;

            return 1;
        }

        if ((buf[i] == 'M') && (buf[i + 1] == '=')
            && (buf[i + 2] == '0') && (buf[i + 3] == '0')
            && (buf[i + 4] == '1') && (buf[i + 5] == '6'))
        {
            *mode16x = 16U;

            return 1;
        }
    }

    for (i = 0; (uint8_t)(i + 9) <= len; i++)
    {
        if ((buf[i] == 'M') && (buf[i + 1] == 'O') && (buf[i + 2] == 'D') && (buf[i + 3] == 'E')
            && (buf[i + 4] == '=') && (buf[i + 5] == '1') && (buf[i + 6] == '6')
            && ((buf[i + 7] == 'X') || (buf[i + 7] == 'x')) && (buf[i + 8] == '8'))
        {
            *mode16x = 8U;

            return 1;
        }
    }

    for (i = 0; (uint8_t)(i + 10) <= len; i++)
    {
        if ((buf[i] == 'M') && (buf[i + 1] == 'O') && (buf[i + 2] == 'D') && (buf[i + 3] == 'E')
            && (buf[i + 4] == '=') && (buf[i + 5] == '1') && (buf[i + 6] == '6')
            && ((buf[i + 7] == 'X') || (buf[i + 7] == 'x'))
            && (buf[i + 8] == '1') && (buf[i + 9] == '6'))
        {
            *mode16x = 16U;

            return 1;
        }
    }

    return 0;
}
//<<AICUBE_USER_GLOBAL_DEFINE_END>>

////////////////////////////////////////
// USB init
////////////////////////////////////////
void USBLIB_Init(void)
{
    usb_init();
    USB_SetIntPriority(0);
    set_usb_OUT_callback(USBLIB_OUT_Callback);
    set_usb_ispcmd("@STCISP#");
}

////////////////////////////////////////
// Wait USB configured
////////////////////////////////////////
void USBLIB_WaitConfiged(void)
{
    while (DeviceState != DEVSTATE_CONFIGURED)
    {
        WDT_Clear();
    }
}

////////////////////////////////////////
// USB OUT callback
////////////////////////////////////////
void USBLIB_OUT_Callback(void)
{
    uint8_t hasInterval;
    uint8_t hasRow;
    uint8_t isRowOff;
    uint8_t row;
    uint8_t hasMode;
    uint8_t mode16x;
    uint32_t intervalUs;

    hasMode = USBLIB_ParseModeCommand(UsbOutBuffer, OutNumber, &mode16x);
    if (hasMode != 0)
    {
        if (Test_SetDisplayMode(mode16x) != 0)
        {
            printf("[USB] display_mode=16x%u\r\n", (unsigned int)Test_GetDisplayMode());
        }
        else
        {
            printf("[USB] mode err, use M=0008/M=0016 or MODE=16X8/MODE=16X16\r\n");
        }

        printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
            (unsigned long)Test_GetRowIntervalUs(),
            (unsigned int)Test_GetLastPwmUs(),
            (unsigned int)Test_GetDisplayMode());

        return;
    }

    hasRow = USBLIB_ParseRowCommand(UsbOutBuffer, OutNumber, &isRowOff, &row);
    if (hasRow != 0)
    {
        if (isRowOff != 0)
        {
            Test_ClearDebugRow();
            printf("[USB] debug_row=OFF\r\n");
        }
        else if (Test_SetDebugRow(row) != 0)
        {
            printf("[USB] debug_row=ON row=%u\r\n", (unsigned int)row);
        }
        else
        {
            printf("[USB] row err, use ROW=0..15\r\n");
        }

        printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
            (unsigned long)Test_GetRowIntervalUs(),
            (unsigned int)Test_GetLastPwmUs(),
            (unsigned int)Test_GetDisplayMode());

        return;
    }

    hasInterval = USBLIB_ParseIntervalCommand(UsbOutBuffer, OutNumber, &intervalUs);
    if (hasInterval != 0)
    {
        Test_SetRowIntervalUs(intervalUs);
        printf("[USB] row_interval_us=%lu\r\n", (unsigned long)Test_GetRowIntervalUs());
    }
    else
    {
        printf("[USB] cmd err, use T=dddd(us|ms|s), ROW=0..15/ROW=OFF, M=0008/M=0016\r\n");
    }

    printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
        (unsigned long)Test_GetRowIntervalUs(),
        (unsigned int)Test_GetLastPwmUs(),
        (unsigned int)Test_GetDisplayMode());
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>
