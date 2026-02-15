//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 锟节达拷锟斤拷锟斤拷锟矫伙拷锟侥硷拷头�?�锟斤拷锟斤拷息  
// 锟侥硷拷锟斤拷锟斤拷: main.c
// 锟侥硷拷锟斤拷锟斤拷: 
// 锟侥硷拷锟芥�??: V1.0
// 锟睫改硷拷录:
//   1. (2025-12-13) 锟斤拷锟斤拷锟侥硷拷
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"                     //默�?�已包含stdio.h、intrins.h等头文件


//<<AICUBE_USER_INCLUDE_BEGIN>>
#include "test/ws2812_test.h"
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// WS2812 锟斤拷锟饺拷直锟斤拷锟斤拷锟斤拷锟斤拷�???app/ws2812_display.c锟斤拷static锟斤�??
// 锟缴讹拷时锟斤拷锟叫�??猴拷应锟�??接口斤拷锟叫匡拷锟斤�??

//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// 项目主函�?
// 入口参数: �?
// 函数返回: �?
////////////////////////////////////////
void main(void)
{
    //<<AICUBE_USER_MAIN_INITIAL_BEGIN>>
    // 锟节达拷锟斤拷锟斤拷锟矫伙拷锟斤拷锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷  
    //<<AICUBE_USER_MAIN_INITIAL_END>>

    SYS_Init();

    //<<AICUBE_USER_MAIN_CODE_BEGIN>>
    ws2812_test_init();
    //<<AICUBE_USER_MAIN_CODE_END>>

    while (1)
    {
        //<<AICUBE_USER_MAIN_LOOP_BEGIN>>
        ws2812_test_step();
//		hc595_write16(0xfffe);
//		ws2812_send_grb(255, 0, 0);
        delay_ms(1);
        //<<AICUBE_USER_MAIN_LOOP_END>>
    }
}

////////////////////////////////////////
// 系统初�?�化函数
// 入口参数: �?
// 函数返回: �?
////////////////////////////////////////
void SYS_Init(void)
{
    EnableAccessXFR();                  //使能访问扩展XFR
    AccessCodeFastest();                //设置最�?速度访问程序代码
    AccessIXramFastest();               //设置最�?速度访问内部XDATA
    IAP_SetTimeBase();                  //设置IAP等待参数,产生1us时基

    //<<AICUBE_USER_PREINITIAL_CODE_BEGIN>>
    // 锟节达拷锟斤拷锟斤拷锟矫伙拷预锟斤拷始锟斤拷锟斤拷锟斤拷  
    //<<AICUBE_USER_PREINITIAL_CODE_END>>

    P0M0 = 0x00; P0M1 = 0x00;           //初�?�化P0口为准双向口模式
    P1M0 = 0x00; P1M1 = 0x00;           //初�?�化P1口为准双向口模式
    P2M0 = 0x00; P2M1 = 0x00;           //初�?�化P2口为准双向口模式
    P3M0 = 0x00; P3M1 = 0x00;           //初�?�化P3口为准双向口模式
    P4M0 = 0x00; P4M1 = 0x00;           //初�?�化P4口为准双向口模式
    P5M0 = 0x00; P5M1 = 0x00;           //初�?�化P5口为准双向口模式
    P6M0 = 0x00; P6M1 = 0x00;           //初�?�化P6口为准双向口模式
    P7M0 = 0x00; P7M1 = 0x00;           //初�?�化P7口为准双向口模式

    PORT0_Init();                       //P0口初始化
    delay_ms(1);
    USBLIB_Init();                      //USB库初始化
    delay_ms(1);

    //<<AICUBE_USER_INITIAL_CODE_BEGIN>>
    // 锟节达拷锟斤拷锟斤拷锟矫伙拷锟斤拷�?�锟斤拷锟斤拷锟斤拷  
    //<<AICUBE_USER_INITIAL_CODE_END>>

    EnableGlobalInt();                  //使能全局�?�?
}

////////////////////////////////////////
// �?秒延时函�?
// 入口参数: us (设置延时的微秒�?)
// 函数返回: �?
////////////////////////////////////////
void delay_us(uint16_t us)
{
    do
    {
        NOP(34);                        //(MAIN_Fosc + 500000) / 1000000 - 6
    } while (--us);
}


////////////////////////////////////////
// �?秒延时函�?
// 入口参数: ms (设置延时的�??秒�?)
// 函数返回: �?
////////////////////////////////////////
void delay_ms(uint16_t ms)
{
    uint16_t i;

    do
    {
        i = MAIN_Fosc / 6000;
        while (--i);
    } while (--ms);
}


//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 锟斤拷锟斤拷应锟斤拷锟�??硷拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷应模锟介：
// - timer.c: TIMER0_Init() 锟斤�?? 锟叫�??达拷锟斤�?? TIMER0_ISR()
// - app/ws2812_display.c: 锟斤拷示锟斤拷色状态锟酵革拷锟铰猴拷锟斤拷
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


