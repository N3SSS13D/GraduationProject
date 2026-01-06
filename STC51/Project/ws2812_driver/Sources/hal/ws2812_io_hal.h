#ifndef __WS2812_IO_HAL_H__
#define __WS2812_IO_HAL_H__

// 40MHz时钟下的精确延时函数
void delay_280ns();
void delay_700ns();

// 发送一个比特
void send_bit(unsigned char bit_val);

// 发送一个字节 (GRB顺序)
void send_byte(unsigned char data_);

// 发送复位信号
void send_reset();

// 设置并显示1个LED颜色
void show_led(unsigned char r, unsigned char g, unsigned char b);

// 设置并显示8个LED颜色
void show_leds(unsigned char r, unsigned char g, unsigned char b);

#endif
