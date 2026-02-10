//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: main.c
// 文件描述: 
// 文件版本: V1.0
// 修改记录:
//   1. (2025-12-13) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"                     //默认已包含stdio.h、intrins.h等头文件


//<<AICUBE_USER_INCLUDE_BEGIN>>
#include "fml/ws2812_driver.h"
#include "app/ws2812_display.h"
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// WS2812 相关全局变量已移至 app/ws2812_display.c（static）
// 由定时器中断和应用接口进行控制

//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// 项目主函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void main(void)
{
    //<<AICUBE_USER_MAIN_INITIAL_BEGIN>>
    // 在此添加用户主函数初始化代码  
    //<<AICUBE_USER_MAIN_INITIAL_END>>

    SYS_Init();

    //<<AICUBE_USER_MAIN_CODE_BEGIN>>
     
    //<<AICUBE_USER_MAIN_CODE_END>>

    while (1)
    {
        //<<AICUBE_USER_MAIN_LOOP_BEGIN>>
        ws2812_display_update();
        delay_ms(100);
//		ws2812_send_grb(255, 0, 0); 
//		delay_ms(1000);
//		ws2812_send_grb(0, 255, 0); 
//		delay_ms(1000);
//		ws2812_send_grb(0, 0, 255); 
//		delay_ms(1000);
        //<<AICUBE_USER_MAIN_LOOP_END>>
    }
}

////////////////////////////////////////
// 系统初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void SYS_Init(void)
{
    EnableAccessXFR();                  //使能访问扩展XFR
    AccessCodeFastest();                //设置最快速度访问程序代码
    AccessIXramFastest();               //设置最快速度访问内部XDATA
    IAP_SetTimeBase();                  //设置IAP等待参数,产生1us时基

    //<<AICUBE_USER_PREINITIAL_CODE_BEGIN>>
    // 在此添加用户预初始化代码  
    //<<AICUBE_USER_PREINITIAL_CODE_END>>

    P0M0 = 0x00; P0M1 = 0x00;           //初始化P0口为准双向口模式
    P1M0 = 0x00; P1M1 = 0x00;           //初始化P1口为准双向口模式
    P2M0 = 0x00; P2M1 = 0x00;           //初始化P2口为准双向口模式
    P3M0 = 0x00; P3M1 = 0x00;           //初始化P3口为准双向口模式
    P4M0 = 0x00; P4M1 = 0x00;           //初始化P4口为准双向口模式
    P5M0 = 0x00; P5M1 = 0x00;           //初始化P5口为准双向口模式
    P6M0 = 0x00; P6M1 = 0x00;           //初始化P6口为准双向口模式
    P7M0 = 0x00; P7M1 = 0x00;           //初始化P7口为准双向口模式

    TIMER0_Init();                      //定时器0初始化
    EXTI0_Init();                       //INT0初始化
    EXTI1_Init();                       //INT1初始化
//    PWMA_Init();                        //高级PWMA初始化
    delay_ms(1);
    USBLIB_Init();                      //USB库初始化
    delay_ms(1);

    //<<AICUBE_USER_INITIAL_CODE_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_INITIAL_CODE_END>>

    EnableGlobalInt();                  //使能全局中断
}

////////////////////////////////////////
// 微秒延时函数
// 入口参数: us (设置延时的微秒值)
// 函数返回: 无
////////////////////////////////////////
void delay_us(uint16_t us)
{
    do
    {
        NOP(34);                        //(MAIN_Fosc + 500000) / 1000000 - 6
    } while (--us);
}


////////////////////////////////////////
// 毫秒延时函数
// 入口参数: ms (设置延时的毫秒值)
// 函数返回: 无
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
// 所有应用逻辑已整理到对应模块：
// - timer.c: TIMER0_Init() 和 中断处理 TIMER0_ISR()
// - app/ws2812_display.c: 显示颜色状态和更新函数
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


