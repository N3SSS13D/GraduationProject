#include "ws2812_driver.h"

static sbit WS2812_DI_P10 = P0^3;
static sbit WS2812_DI_P12 = P0^4;

static uint8_t ws2812_output_pin = WS2812_OUT_P10;

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

static void ws2812_line_high(void)
{
    if (ws2812_output_pin == WS2812_OUT_P12)
    {
        WS2812_DI_P12 = 1;
    }
    else
    {
        WS2812_DI_P10 = 1;
    }
}

static void ws2812_line_low(void)
{
    if (ws2812_output_pin == WS2812_OUT_P12)
    {
        WS2812_DI_P12 = 0;
    }
    else
    {
        WS2812_DI_P10 = 0;
    }
}

void ws2812_init(void)
{
    P0M1 &= ~(BIT3 | BIT4);
    P0M0 |= (BIT3 | BIT4);

    WS2812_DI_P10 = 0;
    WS2812_DI_P12 = 0;
    ws2812_output_pin = WS2812_OUT_P10;
    ws2812_reset_latch();
}

void ws2812_select_output_pin(uint8_t output_pin)
{
    if (output_pin == WS2812_OUT_P12)
    {
        ws2812_output_pin = WS2812_OUT_P12;
        WS2812_DI_P10 = 0;
    }
    else
    {
        ws2812_output_pin = WS2812_OUT_P10;
        WS2812_DI_P12 = 0;
    }
}

void ws2812_reset_latch(void)
{
    ws2812_line_low();
    delay_us(WS_RESET_US);
}

void ws2812_send_bit(uint8_t bit_val)
{
    if (bit_val)
    {
        // T1H: high time for "1" code (~0.6us), T1L: low time (~0.3us)
        ws2812_line_high();
        Delay600ns();
        ws2812_line_low();
        Delay300ns();
    }
    else
    {
        // T0H: high time for "0" code (~0.3us), T0L: low time (~0.6us)
        ws2812_line_high();
        Delay300ns();
        ws2812_line_low();
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
