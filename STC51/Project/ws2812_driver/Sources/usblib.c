//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: usblib.c
// 文件描述: 
// 文件版本: V1.0
// 修改记录:
//   1. (2026-03-16) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"


//<<AICUBE_USER_INCLUDE_BEGIN>>
// 在此添加用户头文件包含  
#include "test.h"
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// 在此添加用户全局变量定义、用户宏定义以及函数声明  
static uint8_t USBLIB_IsHexChar(uint8_t ch)
{
    if (((ch >= '0') && (ch <= '9')) || ((ch >= 'A') && (ch <= 'F')) || ((ch >= 'a') && (ch <= 'f')))
    {
        return 1;
    }

    return 0;
}

static uint8_t USBLIB_HexToNibble(uint8_t ch)
{
    if ((ch >= '0') && (ch <= '9'))
    {
        return (uint8_t)(ch - '0');
    }

    if ((ch >= 'A') && (ch <= 'F'))
    {
        return (uint8_t)(ch - 'A' + 10);
    }

    return (uint8_t)(ch - 'a' + 10);
}

static uint8_t USBLIB_ParseRgbCommand(uint8_t *buf, uint8_t len, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    uint8_t i;
    uint8_t pos;

    if ((len >= 6) && USBLIB_IsHexChar(buf[0]) && USBLIB_IsHexChar(buf[1]) && USBLIB_IsHexChar(buf[2])
        && USBLIB_IsHexChar(buf[3]) && USBLIB_IsHexChar(buf[4]) && USBLIB_IsHexChar(buf[5]))
    {
        *red = (uint8_t)((USBLIB_HexToNibble(buf[0]) << 4) | USBLIB_HexToNibble(buf[1]));
        *green = (uint8_t)((USBLIB_HexToNibble(buf[2]) << 4) | USBLIB_HexToNibble(buf[3]));
        *blue = (uint8_t)((USBLIB_HexToNibble(buf[4]) << 4) | USBLIB_HexToNibble(buf[5]));

        return 1;
    }

    for (i = 0; (uint8_t)(i + 9) <= len; i++)
    {
        if ((buf[i] == 'R') && (buf[i + 1] == 'G') && (buf[i + 2] == 'B') && (buf[i + 3] == '='))
        {
            pos = (uint8_t)(i + 4);
            if (USBLIB_IsHexChar(buf[pos]) && USBLIB_IsHexChar(buf[pos + 1]) && USBLIB_IsHexChar(buf[pos + 2])
                && USBLIB_IsHexChar(buf[pos + 3]) && USBLIB_IsHexChar(buf[pos + 4]) && USBLIB_IsHexChar(buf[pos + 5]))
            {
                *red = (uint8_t)((USBLIB_HexToNibble(buf[pos]) << 4) | USBLIB_HexToNibble(buf[pos + 1]));
                *green = (uint8_t)((USBLIB_HexToNibble(buf[pos + 2]) << 4) | USBLIB_HexToNibble(buf[pos + 3]));
                *blue = (uint8_t)((USBLIB_HexToNibble(buf[pos + 4]) << 4) | USBLIB_HexToNibble(buf[pos + 5]));

                return 1;
            }
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseDelayCommand(uint8_t *buf, uint8_t len, uint16_t *delayMs)
{
    uint8_t i;
    uint8_t pos;

    if ((len == 3) && (buf[0] >= '0') && (buf[0] <= '9') && (buf[1] >= '0') && (buf[1] <= '9')
        && (buf[2] >= '0') && (buf[2] <= '9'))
    {
        *delayMs = (uint16_t)((buf[0] - '0') * 100 + (buf[1] - '0') * 10 + (buf[2] - '0'));

        return 1;
    }

    for (i = 0; (uint8_t)(i + 6) <= len; i++)
    {
        if ((buf[i] == 'M') && (buf[i + 1] == 'S') && (buf[i + 2] == '='))
        {
            pos = (uint8_t)(i + 3);
            if ((buf[pos] >= '0') && (buf[pos] <= '9') && (buf[pos + 1] >= '0') && (buf[pos + 1] <= '9')
                && (buf[pos + 2] >= '0') && (buf[pos + 2] <= '9'))
            {
                *delayMs = (uint16_t)((buf[pos] - '0') * 100 + (buf[pos + 1] - '0') * 10 + (buf[pos + 2] - '0'));

                return 1;
            }
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseUsCommand(uint8_t *buf, uint8_t len, uint16_t *delayUs)
{
    uint8_t i;
    uint8_t pos;
    uint8_t digitCount;
    uint16_t value;

    for (i = 0; (uint8_t)(i + 4) <= len; i++)
    {
        if ((buf[i] == 'U') && (buf[i + 1] == 'S') && (buf[i + 2] == '='))
        {
            pos = (uint8_t)(i + 3);
            value = 0;
            digitCount = 0;
            while ((pos < len) && (buf[pos] >= '0') && (buf[pos] <= '9'))
            {
                value = (uint16_t)(value * 10 + (buf[pos] - '0'));
                pos++;
                digitCount++;
                if (digitCount >= 5)
                {
                    break;
                }
            }

            if (digitCount > 0)
            {
                *delayUs = value;
                return 1;
            }
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseTimerIntervalCommand(uint8_t *buf, uint8_t len, uint16_t *value, uint8_t *unitType)
{
    uint8_t i;

    for (i = 0; (uint8_t)(i + 7) <= len; i++)
    {
        if ((buf[i] == 'T') && (buf[i + 1] == '=')
            && (buf[i + 2] >= '0') && (buf[i + 2] <= '9')
            && (buf[i + 3] >= '0') && (buf[i + 3] <= '9')
            && (buf[i + 4] >= '0') && (buf[i + 4] <= '9')
            && (buf[i + 5] >= '0') && (buf[i + 5] <= '9'))
        {
            *value = (uint16_t)((buf[i + 2] - '0') * 1000 + (buf[i + 3] - '0') * 100
                + (buf[i + 4] - '0') * 10 + (buf[i + 5] - '0'));

            if ((uint8_t)(i + 8) <= len)
            {
                if ((buf[i + 6] == 'u') && (buf[i + 7] == 's'))
                {
                    *unitType = 0;

                    return 1;
                }
                if ((buf[i + 6] == 'm') && (buf[i + 7] == 's'))
                {
                    *unitType = 1;

                    return 1;
                }
            }

            if (buf[i + 6] == 's')
            {
                *unitType = 2;

                return 1;
            }
        }
    }

    return 0;
}

static uint8_t USBLIB_ParsePatternCommand(uint8_t *buf, uint8_t len, uint16_t *patternValue)
{
    uint8_t i;

    for (i = 0; (uint8_t)(i + 6) <= len; i++)
    {
        if ((buf[i] == 'P') && (buf[i + 1] == '=')
            && (buf[i + 2] >= '0') && (buf[i + 2] <= '9')
            && (buf[i + 3] >= '0') && (buf[i + 3] <= '9')
            && (buf[i + 4] >= '0') && (buf[i + 4] <= '9')
            && (buf[i + 5] >= '0') && (buf[i + 5] <= '9'))
        {
            *patternValue = (uint16_t)((buf[i + 2] - '0') * 1000 + (buf[i + 3] - '0') * 100
                + (buf[i + 4] - '0') * 10 + (buf[i + 5] - '0'));

            return 1;
        }
    }

    return 0;
}

static uint8_t USBLIB_ParseRenderModeCommand(uint8_t *buf, uint8_t len, uint16_t *modeValue)
{
    uint8_t i;

    /* M=0000: 16x64, M=0001: 16x8. */
    for (i = 0; (uint8_t)(i + 6) <= len; i++)
    {
        if ((buf[i] == 'M') && (buf[i + 1] == '=')
            && (buf[i + 2] >= '0') && (buf[i + 2] <= '9')
            && (buf[i + 3] >= '0') && (buf[i + 3] <= '9')
            && (buf[i + 4] >= '0') && (buf[i + 4] <= '9')
            && (buf[i + 5] >= '0') && (buf[i + 5] <= '9'))
        {
            *modeValue = (uint16_t)((buf[i + 2] - '0') * 1000 + (buf[i + 3] - '0') * 100
                + (buf[i + 4] - '0') * 10 + (buf[i + 5] - '0'));

            return 1;
        }
    }

    return 0;
}
//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// USB库初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void USBLIB_Init(void)
{
    usb_init();                         //初始化USB模块
    USB_SetIntPriority(0);              //设置中断为最低优先级
    set_usb_OUT_callback(USBLIB_OUT_Callback); //设置USB中断回调函数
    set_usb_ispcmd("@STCISP#");         //设置USB不停电下载命令

    //<<AICUBE_USER_USBLIB_INITIAL_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_USBLIB_INITIAL_END>>
}

////////////////////////////////////////
// 等待USB配置完成函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void USBLIB_WaitConfiged(void)
{
    while (DeviceState != DEVSTATE_CONFIGURED) //等待USB完成配置
        WDT_Clear();                    //清看门狗定时器 (防止硬件自动使能看门狗)
}

////////////////////////////////////////
// USB设备接收数据中断回调程序
// 入口参数: 无
// 函数返回: 无
// OutNumber：USB设备接收到的数据长度
// UsbOutBuffer：保存USB设备接收到的数据
////////////////////////////////////////
void USBLIB_OUT_Callback(void)
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t method;
    uint8_t scheme;
    uint8_t renderMode;
    uint8_t patternIdx;
    uint16_t timerValue;
    uint16_t patternValue;
    uint16_t renderModeValue;
    uint8_t timerUnitType;
    uint8_t hasTimerCmd;
    uint8_t hasPatternCmd;
    uint8_t hasRenderModeCmd;
    uint32_t timerUs;
    uint8_t hasRgb;

    //<<AICUBE_USER_USBLIB_ISR_CODE1_BEGIN>>
    // 在此添加中断函数用户代码  
    hasRgb = USBLIB_ParseRgbCommand(UsbOutBuffer, OutNumber, &red, &green, &blue);
    if (hasRgb != 0)
    {
        Test_SetSolidColor(red, green, blue);
        printf("[USB] rgb=%02X%02X%02X\r\n", (unsigned int)red, (unsigned int)green, (unsigned int)blue);
    }

    hasTimerCmd = USBLIB_ParseTimerIntervalCommand(UsbOutBuffer, OutNumber, &timerValue, &timerUnitType);
    if (hasTimerCmd != 0)
    {
        if (timerUnitType == 0)
        {
            timerUs = (uint32_t)timerValue;
        }
        else if (timerUnitType == 1)
        {
            timerUs = (uint32_t)timerValue * 1000U;
        }
        else
        {
            timerUs = (uint32_t)timerValue * 1000000U;
        }

        Test_SetFrameDelayUs(timerUs);
        if (timerUs >= 10000U)
        {
            printf("[USB] timer0 interval=%lu us (ms mode)\r\n", (unsigned long)Test_GetFrameDelayUs());
        }
        else
        {
            printf("[USB] timer0 interval=%lu us (us mode)\r\n", (unsigned long)Test_GetFrameDelayUs());
        }
    }

    hasPatternCmd = USBLIB_ParsePatternCommand(UsbOutBuffer, OutNumber, &patternValue);
    if (hasPatternCmd != 0)
    {
        Test_SetPatternIndex((uint8_t)patternValue);
        printf("[USB] pattern=%u\r\n", (unsigned int)Test_GetPatternIndex());
    }

    hasRenderModeCmd = USBLIB_ParseRenderModeCommand(UsbOutBuffer, OutNumber, &renderModeValue);
    if (hasRenderModeCmd != 0)
    {
        Test_SetRenderMode((uint8_t)renderModeValue);
        printf("[USB] render_mode=%u\r\n", (unsigned int)Test_GetRenderMode());
    }

    if ((hasRgb == 0) && (hasTimerCmd == 0) && (hasPatternCmd == 0) && (hasRenderModeCmd == 0))
    {
        printf("[USB] cmd err, use RRGGBB or T=dddd(us|ms|s) or P=dddd or M=0000/0001\r\n");
    }

    Test_GetSolidColor(&red, &green, &blue);
    method = Test_GetDisplayMethod();
    scheme = Test_GetScanScheme();
    renderMode = Test_GetRenderMode();
    patternIdx = Test_GetPatternIndex();
    printf("[STATE] method=%u scheme=%u render=%u pattern=%u rgb=%02X%02X%02X ms=%u interval_us=%lu\r\n", (unsigned int)method,
        (unsigned int)scheme, (unsigned int)renderMode, (unsigned int)patternIdx,
        (unsigned int)red, (unsigned int)green, (unsigned int)blue, (unsigned int)Test_GetFrameDelayMs(),
        (unsigned long)Test_GetFrameDelayUs());
    //<<AICUBE_USER_USBLIB_ISR_CODE1_END>>
}



//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码  
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


