#include "stc32g.h"
#include "stc32_stc8_usb.h"

void usb_callback();

void main()
{
    EAXFR = 1;							//允许访问扩展的特殊寄存器，XFR
	WTST = 0;							//设置取程序代码等待时间，
										//赋值为0表示不等待，程序以最快速度运行
	CKCON = 0;							//设置访问片内的xdata速度，
										//赋值为 0表示用最快速度访问，
										//不增加额外的等待时间    
    
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
