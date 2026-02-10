#ifndef __WS2812_DISPLAY_H__
#define __WS2812_DISPLAY_H__

#include "config.h"
#include "ws2812_driver.h"

#define WS2812_MATRIX_WIDTH   8
#define WS2812_MATRIX_HEIGHT  8
#define WS2812_MATRIX_LEDS    (WS2812_MATRIX_WIDTH * WS2812_MATRIX_HEIGHT)

// Color modes
#define COLOR_RED    0
#define COLOR_GREEN  1
#define COLOR_BLUE   2

void ws2812_display_clear(void);
void ws2812_display_digit(uint8_t digit);
void ws2812_display_loop_digits(uint16_t hold_ms);
void ws2812_display_color(uint8_t color_mode);
void ws2812_display_update(void);
uint8_t ws2812_get_current_color(void);

#endif
