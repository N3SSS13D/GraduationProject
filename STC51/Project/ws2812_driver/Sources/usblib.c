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

static uint8_t USBLIB_ParseImageCommand(uint8_t *buf, uint8_t len, uint8_t *isNext, uint8_t *index)
{
    uint8_t i;
    uint8_t v;

    for (i = 0; (uint8_t)(i + 8) <= len; i++)
    {
        if ((buf[i] == 'I') && (buf[i + 1] == 'M') && (buf[i + 2] == 'G') && (buf[i + 3] == '=')
            && (buf[i + 4] == 'N') && (buf[i + 5] == 'E') && (buf[i + 6] == 'X') && (buf[i + 7] == 'T'))
        {
            *isNext = 1;
            *index = 0;

            return 1;
        }
    }

    for (i = 0; (uint8_t)(i + 5) <= len; i++)
    {
        if ((buf[i] == 'I') && (buf[i + 1] == 'M') && (buf[i + 2] == 'G') && (buf[i + 3] == '='))
        {
            if ((buf[i + 4] < '0') || (buf[i + 4] > '9'))
            {
                return 0;
            }

            v = (uint8_t)(buf[i + 4] - '0');
            if ((uint8_t)(i + 6) <= len)
            {
                if ((buf[i + 5] >= '0') && (buf[i + 5] <= '9'))
                {
                    v = (uint8_t)(v * 10U + (uint8_t)(buf[i + 5] - '0'));
                }
            }

            *isNext = 0;
            *index = v;

            return 1;
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseEffectCommand(uint8_t *buf, uint8_t len, uint8_t *effectId)
{
    uint8_t i;

    for (i = 0; (uint8_t)(i + 4) <= len; i++)
    {
        if ((buf[i] == 'F') && (buf[i + 1] == 'X') && (buf[i + 2] == '=')
            && (buf[i + 3] >= '0') && (buf[i + 3] <= '5'))
        {
            *effectId = (uint8_t)(buf[i + 3] - '0');

            return 1;
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseGradientEnableCommand(uint8_t *buf, uint8_t len, uint8_t *enable)
{
    uint8_t i;

    for (i = 0; (uint8_t)(i + 6) <= len; i++)
    {
        if ((buf[i] == 'G') && (buf[i + 1] == 'R') && (buf[i + 2] == 'A') && (buf[i + 3] == 'D')
            && (buf[i + 4] == '=') && (buf[i + 5] == '0'))
        {
            *enable = 0;

            return 1;
        }
        if ((buf[i] == 'G') && (buf[i + 1] == 'R') && (buf[i + 2] == 'A') && (buf[i + 3] == 'D')
            && (buf[i + 4] == '=') && (buf[i + 5] == '1'))
        {
            *enable = 1;

            return 1;
        }
    }

    return 0;
}

static uint8_t USBLIB_HexNibble(uint8_t c, uint8_t *v)
{
    if ((c >= '0') && (c <= '9'))
    {
        *v = (uint8_t)(c - '0');

        return 1;
    }
    if ((c >= 'A') && (c <= 'F'))
    {
        *v = (uint8_t)(10U + c - 'A');

        return 1;
    }
    if ((c >= 'a') && (c <= 'f'))
    {
        *v = (uint8_t)(10U + c - 'a');

        return 1;
    }

    return 0;
}

static uint8_t USBLIB_ParseRgb24(uint8_t *buf, uint8_t off, uint8_t len, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t n[6];
    uint8_t i;

    if ((uint8_t)(off + 6) > len)
    {
        return 0;
    }

    for (i = 0; i < 6U; i++)
    {
        if (USBLIB_HexNibble(buf[(uint8_t)(off + i)], &n[i]) == 0)
        {
            return 0;
        }
    }

    *r = (uint8_t)((n[0] << 4) | n[1]);
    *g = (uint8_t)((n[2] << 4) | n[3]);
    *b = (uint8_t)((n[4] << 4) | n[5]);

    return 1;
}

static uint8_t USBLIB_ParseColorCommand(uint8_t *buf, uint8_t len, uint8_t *isFg, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t i;

    for (i = 0; (uint8_t)(i + 10) <= len; i++)
    {
        if ((buf[i] == 'F') && (buf[i + 1] == 'G') && (buf[i + 2] == '=')
            && (USBLIB_ParseRgb24(buf, (uint8_t)(i + 3), len, r, g, b) != 0))
        {
            *isFg = 1;

            return 1;
        }

        if ((buf[i] == 'B') && (buf[i + 1] == 'G') && (buf[i + 2] == '=')
            && (USBLIB_ParseRgb24(buf, (uint8_t)(i + 3), len, r, g, b) != 0))
        {
            *isFg = 0;

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
    uint8_t hasMode;
    uint8_t mode16x;
    uint8_t hasImage;
    uint8_t imageNext;
    uint8_t imageIndex;
    uint8_t hasEffect;
    uint8_t effectId;
    uint8_t hasGrad;
    uint8_t gradEnable;
    uint8_t hasColor;
    uint8_t isFg;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint32_t intervalUs;

    /* Debug command summary:
     * 1) T=dddd(us|ms|s)                   set row interval
     * 2) M=0008 / M=0016                   set display mode
     * 3) IMG=n / IMG=NEXT                  select or switch image
        * 4) FX=0..5                           set render effect
        *    FX=5: scroll text "JiLin University" glyph set
     * 5) GRAD=0|1                          enable or disable gradient
     * 6) FG=RRGGBB / BG=RRGGBB             set foreground/background color
     */

    hasImage = USBLIB_ParseImageCommand(UsbOutBuffer, OutNumber, &imageNext, &imageIndex);
    if (hasImage != 0)
    {
        if (imageNext != 0)
        {
            Test_NextImage();
            printf("[USB] image=NEXT idx=%u\r\n", (unsigned int)Test_GetImageIndex());
        }
        else
        {
            (void)Test_SetImageIndex(imageIndex);
            printf("[USB] image=%u\r\n", (unsigned int)Test_GetImageIndex());
        }

        printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
            (unsigned long)Test_GetRowIntervalUs(),
            (unsigned int)Test_GetLastPwmUs(),
            (unsigned int)Test_GetDisplayMode());

        return;
    }

    hasEffect = USBLIB_ParseEffectCommand(UsbOutBuffer, OutNumber, &effectId);
    if (hasEffect != 0)
    {
        if (Test_SetRenderEffect(effectId) != 0)
        {
            printf("[USB] effect=%u\r\n", (unsigned int)effectId);
        }
        else
        {
            printf("[USB] effect err, use FX=0..5\r\n");
        }

        printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
            (unsigned long)Test_GetRowIntervalUs(),
            (unsigned int)Test_GetLastPwmUs(),
            (unsigned int)Test_GetDisplayMode());

        return;
    }

    hasGrad = USBLIB_ParseGradientEnableCommand(UsbOutBuffer, OutNumber, &gradEnable);
    if (hasGrad != 0)
    {
        Test_SetRenderUseGradient(gradEnable);
        printf("[USB] gradient=%u\r\n", (unsigned int)gradEnable);
        printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
            (unsigned long)Test_GetRowIntervalUs(),
            (unsigned int)Test_GetLastPwmUs(),
            (unsigned int)Test_GetDisplayMode());

        return;
    }

    hasColor = USBLIB_ParseColorCommand(UsbOutBuffer, OutNumber, &isFg, &r, &g, &b);
    if (hasColor != 0)
    {
        if (isFg != 0)
        {
            Test_SetForegroundColor(r, g, b);
            printf("[USB] fg=%02X%02X%02X\r\n", (unsigned int)r, (unsigned int)g, (unsigned int)b);
        }
        else
        {
            Test_SetBackgroundColor(r, g, b);
            printf("[USB] bg=%02X%02X%02X\r\n", (unsigned int)r, (unsigned int)g, (unsigned int)b);
        }

        printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
            (unsigned long)Test_GetRowIntervalUs(),
            (unsigned int)Test_GetLastPwmUs(),
            (unsigned int)Test_GetDisplayMode());

        return;
    }

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

    hasInterval = USBLIB_ParseIntervalCommand(UsbOutBuffer, OutNumber, &intervalUs);
    if (hasInterval != 0)
    {
        Test_SetRowIntervalUs(intervalUs);
        printf("[USB] row_interval_us=%lu\r\n", (unsigned long)Test_GetRowIntervalUs());
    }
    else
    {
        printf("[USB] cmd err, use T/M, IMG=n|IMG=NEXT, FX=0..5, GRAD=0|1, FG=RRGGBB, BG=RRGGBB\r\n");
    }

    printf("[STATE] row_interval_us=%lu pwm_us=%u mode=16x%u\r\n",
        (unsigned long)Test_GetRowIntervalUs(),
        (unsigned int)Test_GetLastPwmUs(),
        (unsigned int)Test_GetDisplayMode());
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>
