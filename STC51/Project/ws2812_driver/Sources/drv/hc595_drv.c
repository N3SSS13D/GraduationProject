#include "config.h"
#include "hc595_drv.h"

static sbit g_hc595Ser = P0^0;
static sbit g_hc595Rclk = P0^1;
static sbit g_hc595Srclk = P0^2;

static void HC595_PulseSrclk(void)
{
    g_hc595Srclk = 1;
    _nop_();
    g_hc595Srclk = 0;
}

static void HC595_PulseRclk(void)
{
    g_hc595Rclk = 1;
    _nop_();
    g_hc595Rclk = 0;
}

void HC595_Init(void)
{
    /* 74HC595控制脚(P0.0/P0.1/P0.2)使用推挽输出，提升边沿质量。 */
    SetP0nPushPullMode(PIN_0 | PIN_1 | PIN_2);
    SetP0nInitLevelLow(PIN_0 | PIN_1 | PIN_2);

    g_hc595Ser = 0;
    g_hc595Rclk = 0;
    g_hc595Srclk = 0;
    HC595_AllOff();
}

void HC595_Write16(uint16_t value)
{
    int8_t bitIdx;

    for (bitIdx = 15; bitIdx >= 0; bitIdx--)
    {
        g_hc595Ser = (value >> bitIdx) & 0x01;
        HC595_PulseSrclk();
    }

    HC595_PulseRclk();
}

void HC595_SelectRows(uint8_t rowA, uint8_t rowB)
{
    uint16_t value;

    value = 0xFFFF;
    if (rowA < 16)
    {
        value &= ~((uint16_t)1 << rowA);
    }
    if (rowB < 16)
    {
        value &= ~((uint16_t)1 << rowB);
    }

    HC595_Write16(value);
}

void HC595_AllOff(void)
{
    HC595_Write16(0xFFFF);
}
