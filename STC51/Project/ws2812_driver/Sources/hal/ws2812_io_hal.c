#include "ws2812_io_hal.h"
#include "ai8051u.h"

sbit WS2812 = P4^2;    // WS2812数据引脚

// 40MHz时钟下的精确延时函数
void delay_280ns() {   // 约0.28us
    _nop_(); _nop_(); _nop_(); _nop_();
    _nop_(); _nop_(); _nop_(); _nop_();
	
}

void delay_700ns() {   // 约0.7us
    _nop_(); _nop_(); _nop_(); _nop_();
    _nop_(); _nop_(); _nop_(); _nop_();
    _nop_(); _nop_(); _nop_(); _nop_();
    _nop_(); _nop_(); _nop_(); _nop_();
	
}

// 发送一个比特
void send_bit(unsigned char bit_val) {
    if(bit_val) {
        // 发送'1': 高0.7us + 低0.6us
        WS2812 = 1;
        delay_700ns();
        WS2812 = 0;
        delay_280ns();  // 实际约0.7us，接近0.6us要求
    } else {
        // 发送'0': 高0.35us + 低0.8us
        WS2812 = 1;
        delay_280ns();  // 约0.28us，接近0.35us
        WS2812 = 0;
        delay_700ns();
    }
}

// 发送一个字节 (GRB顺序)
void send_byte(unsigned char data_) {
    unsigned char i;
    for(i = 0; i < 8; i++) {
        send_bit(data_ & 0x80);  // 发送最高位
        data_ <<= 1;
    }
}

// 发送复位信号
void send_reset() {
    unsigned int i;
    WS2812 = 0;
    for(i = 0; i < 500; i++) {  // 约50us延时
        _nop_(); _nop_(); _nop_(); _nop_();
    }
}

// 设置并显示1个LED颜色
void show_led(unsigned char r, unsigned char g, unsigned char b) {
    send_byte(g);  // G
    send_byte(r);  // R
    send_byte(b);  // B
}

// 设置并显示8个LED颜色
void show_leds(unsigned char r, unsigned char g, unsigned char b) {
    unsigned char i;
    
    // 关闭中断确保时序稳定
    EA = 0;
    
    // 发送8个LED的相同颜色
    for(i = 0; i < 9; i++) {
        send_byte(g);  // G
        send_byte(r);  // R
        send_byte(b);  // B
    }
    
    // 发送复位信号
    send_reset();
    
    // 恢复中断
    EA = 1;
}

