#ifndef __WS2812_DRIVER_H__
#define __WS2812_DRIVER_H__

#include "config.h"

#define WS2812_LED_COUNT 64

void ws2812_init(void);
void ws2812_reset_latch(void);
void ws2812_send_bit(uint8_t bit_val);
void ws2812_send_byte(uint8_t value);
void ws2812_send_grb(uint8_t g, uint8_t r, uint8_t b);
void ws2812_send_buffer(const uint8_t *grb, uint16_t led_count);

#endif
