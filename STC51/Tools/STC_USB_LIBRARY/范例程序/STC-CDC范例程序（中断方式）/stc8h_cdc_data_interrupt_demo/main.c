#include "stc8h.h"
#include "stc32_stc8_usb.h"

void usb_callback();

void main()
{
    P_SW2 |= 0x80;
    
    P0M1 = 0x00;   P0M0 = 0x00;
    P1M1 = 0x00;   P1M0 = 0x00;
    P2M1 = 0x00;   P2M0 = 0x00;
    P3M1 = 0x00;   P3M0 = 0x00;
    P4M1 = 0x00;   P4M0 = 0x00;
    P5M1 = 0x00;   P5M0 = 0x00;
    P6M1 = 0x00;   P6M0 = 0x00;
    P7M1 = 0x00;   P7M0 = 0x00;
    
    usb_init();                                     //USB CDC 接口配置
    set_usb_OUT_callback(usb_callback);             //设置中断回调回调函数
    
    EA = 1;

    while (1);
}

void usb_callback()
{
    USB_SendData(UsbOutBuffer,OutNumber);           //发送数据缓冲区，长度（接收数据原样返回, 用于测试）
}
