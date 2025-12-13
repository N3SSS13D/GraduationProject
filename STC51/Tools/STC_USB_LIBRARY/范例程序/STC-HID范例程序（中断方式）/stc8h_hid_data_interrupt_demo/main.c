#include "stc8h.h"
#include "string.h"

#include "stc32_stc8_usb.h"

void sys_init();
void DelayXms(int n);
void usb_callback();

void main()
{
    sys_init();
    usb_init();
    set_usb_OUT_callback(usb_callback);             //设置中断回调回调函数

    EA = 1;

    while (1);
}

void sys_init()
{
    P_SW2 = 0x80;
}

void usb_callback()
{
  
     USB_SendData(UsbOutBuffer, 64);      //原路返回, 用于测试HID
    
}
