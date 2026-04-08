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
    uint32_t intervalUs;

    hasInterval = USBLIB_ParseIntervalCommand(UsbOutBuffer, OutNumber, &intervalUs);
    if (hasInterval != 0)
    {
        Test_SetRowIntervalUs(intervalUs);
        printf("[USB] row_interval_us=%lu\r\n", (unsigned long)Test_GetRowIntervalUs());
    }
    else
    {
        printf("[USB] cmd err, use T=dddd(us|ms|s)\r\n");
    }

    printf("[STATE] row_interval_us=%lu pwm_us=%u\r\n",
        (unsigned long)Test_GetRowIntervalUs(),
        (unsigned int)Test_GetLastPwmUs());
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>
