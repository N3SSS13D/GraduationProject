#include "ws2812_display.h"

#define WS_COLOR_OFF  0x00
#define WS_INTENSITY  0x40

static uint8_t xdata g_frame[WS2812_MATRIX_LEDS * 3];
static uint8_t display_color_state = COLOR_RED;  // 当前显示颜色状态（内部 static）

// Each byte is one row, bit7 = column 0 (left)
static const uint8_t digit_rows[10][8] = {
    // 0
    {0x7E, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7E},
    // 1
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C},
    // 2
    {0x7E, 0x81, 0x01, 0x06, 0x18, 0x60, 0x80, 0xFF},
    // 3
    {0x7E, 0x81, 0x01, 0x0E, 0x0E, 0x01, 0x81, 0x7E},
    // 4
    {0x06, 0x0E, 0x16, 0x26, 0x46, 0xFF, 0x06, 0x06},
    // 5
    {0xFF, 0x80, 0x80, 0xFE, 0x01, 0x01, 0x81, 0x7E},
    // 6
    {0x3E, 0x40, 0x80, 0xFE, 0x81, 0x81, 0x81, 0x7E},
    // 7
    {0xFF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20},
    // 8
    {0x7E, 0x81, 0x81, 0x7E, 0x81, 0x81, 0x81, 0x7E},
    // 9
    {0x7E, 0x81, 0x81, 0x81, 0x7F, 0x01, 0x02, 0x7C}
};

static void fill_frame_with_digit(uint8_t digit, uint8_t g, uint8_t r, uint8_t b)
{
    uint8_t row;
    uint8_t col;
    uint16_t led = 0;

    if (digit > 9)
    {
        digit = 0;
    }

    for (row = 0; row < WS2812_MATRIX_HEIGHT; row++)
    {
        uint8_t row_bits = digit_rows[digit][row];
        for (col = 0; col < WS2812_MATRIX_WIDTH; col++)
        {
            uint8_t on = row_bits & (0x80 >> col);
            uint16_t base = led * 3;
            g_frame[base]     = on ? g : WS_COLOR_OFF; // G
            g_frame[base + 1] = on ? r : WS_COLOR_OFF; // R
            g_frame[base + 2] = on ? b : WS_COLOR_OFF; // B
            led++;
        }
    }
}

static void fill_frame_solid(uint8_t g, uint8_t r, uint8_t b)
{
    uint16_t i;
    for (i = 0; i < WS2812_MATRIX_LEDS; i++)
    {
        uint16_t base = i * 3;
        g_frame[base]     = g;
        g_frame[base + 1] = r;
        g_frame[base + 2] = b;
    }
}

void ws2812_display_clear(void)
{
    uint16_t i;
    for (i = 0; i < WS2812_MATRIX_LEDS * 3; i++)
    {
        g_frame[i] = WS_COLOR_OFF;
    }
    ws2812_send_buffer(g_frame, WS2812_MATRIX_LEDS);
}

void ws2812_display_digit(uint8_t digit)
{
    fill_frame_with_digit(digit, WS_INTENSITY, WS_INTENSITY, WS_INTENSITY);
    ws2812_send_buffer(g_frame, WS2812_MATRIX_LEDS);
}

void ws2812_display_color(uint8_t color_mode)
{
    uint8_t g = WS_COLOR_OFF;
    uint8_t r = WS_COLOR_OFF;
    uint8_t b = WS_COLOR_OFF;

    switch (color_mode)
    {
        case COLOR_RED:
            r = WS_INTENSITY;
            break;
        case COLOR_GREEN:
            g = WS_INTENSITY;
            break;
        case COLOR_BLUE:
            b = WS_INTENSITY;
            break;
        default:
            break;
    }

    fill_frame_solid(g, r, b);
    ws2812_send_buffer(g_frame, WS2812_MATRIX_LEDS);
}

void ws2812_display_loop_digits(uint16_t hold_ms)
{
    uint8_t d;
    for (d = 0; d < 10; d++)
    {
        ws2812_display_digit(d);
        delay_ms(hold_ms);
    }
}

////////////////////////////////////////
// 定时器中断回调函数
// 功能: 由timer.c的中断处理程序调用，用于切换颜色
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void timer0_tick_callback(void)
{
    display_color_state = (display_color_state >= COLOR_BLUE) ? COLOR_RED : (display_color_state + 1);
}

////////////////////////////////////////
// 显示更新函数
// 功能: 根据当前color_state显示对应的颜色
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void ws2812_display_update(void)
{
    ws2812_display_color(display_color_state);
}

////////////////////////////////////////
// 获取当前颜色状态
// 功能: 返回当前显示的颜色模式
// 入口参数: 无
// 函数返回: 颜色状态（COLOR_RED/GREEN/BLUE）
////////////////////////////////////////
uint8_t ws2812_get_current_color(void)
{
    return display_color_state;
}
