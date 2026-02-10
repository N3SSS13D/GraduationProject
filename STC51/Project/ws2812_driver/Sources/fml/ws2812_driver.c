#include "ws2812_driver.h"

// WS2812 data input on P1.0
static sbit WS2812_DI = P1^0;

// WS2812 timing constant
#define WS_RESET_US   100  // latch time >= 80 us

////////////////////////////////////////
// 精确延时函数 @ 40MHz
////////////////////////////////////////

// 延时约300纳秒
static void Delay300ns(void)
{
    unsigned long edata i;
    
    _nop_();
    _nop_();
    _nop_();
    i = 1UL;
    while (i) i--;
}

// 延时约600纳秒
static void Delay600ns(void)
{
    unsigned long edata i;
    
    _nop_();
    _nop_();
    _nop_();
    i = 4UL;
    while (i) i--;
}

void ws2812_init(void)
{
    // Set P1.0 as push-pull output for clean edges
    P1M1 &= ~BIT0;
    P1M0 |= BIT0;

    WS2812_DI = 0;
    ws2812_reset_latch();
}

void ws2812_reset_latch(void)
{
    WS2812_DI = 0;
    delay_us(WS_RESET_US);
}

void ws2812_send_bit(uint8_t bit_val)
{
    if (bit_val)
    {
        // T1H: high time for "1" code (~0.6us), T1L: low time (~0.3us)
        WS2812_DI = 1;
        Delay600ns();
        WS2812_DI = 0;
        Delay300ns();
    }
    else
    {
        // T0H: high time for "0" code (~0.3us), T0L: low time (~0.6us)
        WS2812_DI = 1;
        Delay300ns();
        WS2812_DI = 0;
        Delay600ns();
    }
}

void ws2812_send_byte(uint8_t value)
{
    uint8_t mask;
    for (mask = 0x80; mask != 0; mask >>= 1)
    {
        ws2812_send_bit(value & mask);
    }
}

void ws2812_send_grb(uint8_t g, uint8_t r, uint8_t b)
{
    ws2812_send_byte(g);
    ws2812_send_byte(r);
    ws2812_send_byte(b);
}

void ws2812_send_buffer(const uint8_t *grb, uint16_t led_count)
{
    uint16_t i;
    uint16_t idx = 0;

    for (i = 0; i < led_count; i++)
    {
        uint8_t g = grb[idx++];
        uint8_t r = grb[idx++];
        uint8_t b = grb[idx++];
        ws2812_send_grb(g, r, b);
    }

    ws2812_reset_latch();
}
