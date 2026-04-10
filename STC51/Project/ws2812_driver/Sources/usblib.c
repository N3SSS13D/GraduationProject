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
#define USBLIB_TEXT_SEQUENCE_MAX         32U

typedef struct
{
    uint8_t isPlay;
    uint8_t hasContent;
    uint8_t hasEffect;
    uint8_t hasDirection;
    uint8_t hasSpeed;
    uint8_t hasAnim;
    uint8_t hasColorMode;
    uint8_t hasGradient;
    uint8_t hasBrightness;
    uint8_t hasImage;
    uint8_t hasGlyphIndex;
    uint8_t hasTextSequence;
    uint8_t hasFg;
    uint8_t hasBg;
    uint8_t content;
    uint8_t effect;
    uint8_t direction;
    uint8_t speed;
    uint8_t anim;
    uint8_t colorMode;
    uint8_t gradient;
    uint8_t brightness;
    uint8_t image;
    uint8_t glyphIndex;
    uint8_t textSequenceLen;
    uint8_t textSequence[USBLIB_TEXT_SEQUENCE_MAX];
    uint8_t fgR;
    uint8_t fgG;
    uint8_t fgB;
    uint8_t bgR;
    uint8_t bgG;
    uint8_t bgB;
} USBLIB_PlayCmd_t;

static uint8_t USBLIB_ParseRgb24(uint8_t *buf, uint8_t off, uint8_t len, uint8_t *r, uint8_t *g, uint8_t *b);
static uint8_t USBLIB_ParseColorCommand(uint8_t *buf, uint8_t len, uint8_t *isFg, uint8_t *r, uint8_t *g, uint8_t *b);

static uint8_t USBLIB_ParseU8At(uint8_t *buf, uint8_t off, uint8_t len, uint8_t *value)
{
    uint8_t v;

    if ((off >= len) || (buf[off] < '0') || (buf[off] > '9'))
    {
        return 0;
    }

    v = (uint8_t)(buf[off] - '0');
    if ((uint8_t)(off + 1U) < len)
    {
        if ((buf[(uint8_t)(off + 1U)] >= '0') && (buf[(uint8_t)(off + 1U)] <= '9'))
        {
            v = (uint8_t)(v * 10U + (uint8_t)(buf[(uint8_t)(off + 1U)] - '0'));
            if ((uint8_t)(off + 2U) < len)
            {
                if ((buf[(uint8_t)(off + 2U)] >= '0') && (buf[(uint8_t)(off + 2U)] <= '9'))
                {
                    v = (uint8_t)(v * 10U + (uint8_t)(buf[(uint8_t)(off + 2U)] - '0'));
                }
            }
        }
    }

    *value = v;
    return 1;
}

static uint8_t USBLIB_HasPlayToken(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; (uint8_t)(i + 4U) <= len; i++)
    {
        if ((buf[i] == 'P') && (buf[(uint8_t)(i + 1U)] == 'L')
            && (buf[(uint8_t)(i + 2U)] == 'A') && (buf[(uint8_t)(i + 3U)] == 'Y'))
        {
            return 1;
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseTextSequence(uint8_t *buf, uint8_t len, uint8_t *seq, uint8_t *seqLen)
{
    uint8_t i;
    uint8_t j;
    uint16_t value;
    uint8_t count;
    uint8_t hasDigit;

    for (i = 0; (uint8_t)(i + 3U) <= len; i++)
    {
        if ((buf[i] == 'S') && (buf[(uint8_t)(i + 1U)] == 'Q') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            j = (uint8_t)(i + 3U);
            count = 0U;

            while ((j < len) && (count < USBLIB_TEXT_SEQUENCE_MAX))
            {
                value = 0U;
                hasDigit = 0U;
                while ((j < len) && (buf[j] >= '0') && (buf[j] <= '9'))
                {
                    value = (uint16_t)(value * 10U + (uint16_t)(buf[j] - '0'));
                    if (value > 255U)
                    {
                        return 0U;
                    }

                    hasDigit = 1U;
                    j++;
                }

                if (hasDigit == 0U)
                {
                    break;
                }

                seq[count] = (uint8_t)value;
                count++;

                if ((j < len) && (buf[j] == ','))
                {
                    j++;
                    continue;
                }

                break;
            }

            if (count == 0U)
            {
                return 0U;
            }

            *seqLen = count;
            return 1U;
        }
    }

    return 0U;
}

static void USBLIB_ParsePlayCommand(uint8_t *buf, uint8_t len, USBLIB_PlayCmd_t *cmd)
{
    uint8_t i;
    uint8_t tmp;

    cmd->isPlay = USBLIB_HasPlayToken(buf, len);
    if (cmd->isPlay == 0U)
    {
        return;
    }

    for (i = 0; (uint8_t)(i + 3U) <= len; i++)
    {
        if ((buf[i] == 'C') && (buf[(uint8_t)(i + 1U)] == 'T') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            if ((USBLIB_ParseU8At(buf, (uint8_t)(i + 3U), len, &tmp) != 0U) && (tmp <= 1U))
            {
                cmd->hasContent = 1U;
                cmd->content = tmp;
            }
        }

        if ((uint8_t)(i + 4U) <= len)
        {
            if ((buf[i] == 'D') && (buf[(uint8_t)(i + 1U)] == 'I')
                && (buf[(uint8_t)(i + 2U)] == 'R') && (buf[(uint8_t)(i + 3U)] == '='))
            {
                if ((USBLIB_ParseU8At(buf, (uint8_t)(i + 4U), len, &tmp) != 0U) && (tmp <= 3U))
                {
                    cmd->hasDirection = 1U;
                    cmd->direction = tmp;
                }
            }

            if ((buf[i] == 'S') && (buf[(uint8_t)(i + 1U)] == 'P')
                && (buf[(uint8_t)(i + 2U)] == 'D') && (buf[(uint8_t)(i + 3U)] == '='))
            {
                if (USBLIB_ParseU8At(buf, (uint8_t)(i + 4U), len, &tmp) != 0U)
                {
                    cmd->hasSpeed = 1U;
                    cmd->speed = tmp;
                }
            }

            if ((buf[i] == 'A') && (buf[(uint8_t)(i + 1U)] == 'N')
                && (buf[(uint8_t)(i + 2U)] == 'I') && (buf[(uint8_t)(i + 3U)] == '='))
            {
                if (USBLIB_ParseU8At(buf, (uint8_t)(i + 4U), len, &tmp) != 0U)
                {
                    cmd->hasAnim = 1U;
                    cmd->anim = tmp;
                }
            }
        }

        if ((buf[i] == 'F') && (buf[(uint8_t)(i + 1U)] == 'X') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            if ((USBLIB_ParseU8At(buf, (uint8_t)(i + 3U), len, &tmp) != 0U) && (tmp <= 8U))
            {
                cmd->hasEffect = 1U;
                cmd->effect = tmp;
            }
        }

        if ((buf[i] == 'C') && (buf[(uint8_t)(i + 1U)] == 'M') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            if ((USBLIB_ParseU8At(buf, (uint8_t)(i + 3U), len, &tmp) != 0U) && (tmp <= 1U))
            {
                cmd->hasColorMode = 1U;
                cmd->colorMode = tmp;
            }
        }

        if ((buf[i] == 'G') && (buf[(uint8_t)(i + 1U)] == 'S') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            if (USBLIB_ParseU8At(buf, (uint8_t)(i + 3U), len, &tmp) != 0U)
            {
                cmd->hasGradient = 1U;
                cmd->gradient = tmp;
            }
        }

        if ((buf[i] == 'B') && (buf[(uint8_t)(i + 1U)] == 'R') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            if (USBLIB_ParseU8At(buf, (uint8_t)(i + 3U), len, &tmp) != 0U)
            {
                cmd->hasBrightness = 1U;
                cmd->brightness = tmp;
            }
        }

        if ((buf[i] == 'G') && (buf[(uint8_t)(i + 1U)] == 'I') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            if (USBLIB_ParseU8At(buf, (uint8_t)(i + 3U), len, &tmp) != 0U)
            {
                cmd->hasGlyphIndex = 1U;
                cmd->glyphIndex = tmp;
            }
        }
    }

    for (i = 0; (uint8_t)(i + 4U) <= len; i++)
    {
        if ((buf[i] == 'I') && (buf[(uint8_t)(i + 1U)] == 'M')
            && (buf[(uint8_t)(i + 2U)] == 'G') && (buf[(uint8_t)(i + 3U)] == '='))
        {
            if (USBLIB_ParseU8At(buf, (uint8_t)(i + 4U), len, &tmp) != 0U)
            {
                cmd->hasImage = 1U;
                cmd->image = tmp;
            }
        }
    }

    if (USBLIB_ParseTextSequence(buf, len, cmd->textSequence, &cmd->textSequenceLen) != 0U)
    {
        cmd->hasTextSequence = 1U;
    }

    if (USBLIB_ParseColorCommand(buf, len, &tmp, &cmd->fgR, &cmd->fgG, &cmd->fgB) != 0U)
    {
        if (tmp != 0U)
        {
            cmd->hasFg = 1U;
        }
        else
        {
            cmd->hasBg = 1U;
            cmd->bgR = cmd->fgR;
            cmd->bgG = cmd->fgG;
            cmd->bgB = cmd->fgB;
        }
    }

    for (i = 0; (uint8_t)(i + 10U) <= len; i++)
    {
        if ((buf[i] == 'B') && (buf[(uint8_t)(i + 1U)] == 'G') && (buf[(uint8_t)(i + 2U)] == '='))
        {
            if (USBLIB_ParseRgb24(buf, (uint8_t)(i + 3U), len, &cmd->bgR, &cmd->bgG, &cmd->bgB) != 0U)
            {
                cmd->hasBg = 1U;
            }
        }
    }
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
    USBLIB_PlayCmd_t playCmd = {0};
    /* USB v2 command (legacy commands removed):
     * PLAY CT=x   // content: 0=pattern image, 1=glyph text
     *      DIR=x  // direction: 0=normal, 1=rotate180, 2=rotateCW90, 3=rotateCCW90
     *      FX=x   // effect: 0=static,1=breath,2=gradient,3=scrollL,4=scrollR,5=jluScroll,6=fadeIn,7=fadeOut,8=colorCycle
     *      SPD=x  // scroll step: 1..255 (larger is faster)
     *      ANI=x  // animation step: 1..255 (larger is faster)
     *      CM=x   // color mode: 0=solid, 1=gradient
     *      GS=x   // gradient span alpha: 0..255
     *      BR=x   // brightness: 0..255 (0=off,255=max)
     *      IMG=x  // image index: 0..(TEST_IMAGE_COUNT-1)
    *      GI=x   // static glyph index: 0..(TEST_SCROLL_GLYPH_COUNT-1)
    *      SQ=a,b,c... // scroll glyph index sequence (e.g. SQ=0,1,2,3)
     *      FG=RRGGBB // foreground color in RGB888 hex
     *      BG=RRGGBB // background color in RGB888 hex
     */

    USBLIB_ParsePlayCommand(UsbOutBuffer, OutNumber, &playCmd);
    if (playCmd.isPlay != 0U)
    {
        if (playCmd.hasContent != 0U)
        {
            (void)Test_SetContentType(playCmd.content);
        }
        if (playCmd.hasDirection != 0U)
        {
            (void)Test_SetDirection(playCmd.direction);
        }
        if (playCmd.hasEffect != 0U)
        {
            (void)Test_SetRenderEffect(playCmd.effect);
        }
        if (playCmd.hasSpeed != 0U)
        {
            Test_SetScrollStep(playCmd.speed);
        }
        if (playCmd.hasAnim != 0U)
        {
            Test_SetAnimStep(playCmd.anim);
        }
        if (playCmd.hasColorMode != 0U)
        {
            (void)Test_SetColorMode(playCmd.colorMode);
        }
        if (playCmd.hasGradient != 0U)
        {
            Test_SetGradientSpan(playCmd.gradient);
        }
        if (playCmd.hasBrightness != 0U)
        {
            Test_SetBrightness(playCmd.brightness);
        }
        if (playCmd.hasImage != 0U)
        {
            (void)Test_SetImageIndex(playCmd.image);
        }
        if (playCmd.hasGlyphIndex != 0U)
        {
            (void)Test_SetGlyphDisplayIndex(playCmd.glyphIndex);
        }
        if (playCmd.hasTextSequence != 0U)
        {
            (void)Test_SetScrollGlyphSequence(playCmd.textSequence, playCmd.textSequenceLen);
        }
        if (playCmd.hasFg != 0U)
        {
            Test_SetForegroundColor(playCmd.fgR, playCmd.fgG, playCmd.fgB);
        }
        if (playCmd.hasBg != 0U)
        {
            Test_SetBackgroundColor(playCmd.bgR, playCmd.bgG, playCmd.bgB);
        }

        printf("[USB] play cfg applied\r\n");
        return;
    }

    printf("[USB] cmd err, use PLAY CT/DIR/FX/SPD/ANI/CM/GS/BR/IMG/GI/SQ/FG/BG\r\n");
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>
