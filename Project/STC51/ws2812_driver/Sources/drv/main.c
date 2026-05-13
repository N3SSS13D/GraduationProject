//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: main.c
// 文件描述: 
// 文件版本: V1.0
// 修改记录:
//   1. (2026-03-15) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"                     //默认已包含stdio.h、intrins.h等头文件


//<<AICUBE_USER_INCLUDE_BEGIN>>
// 在此添加用户头文件包含  
#include "hc595_drv.h"
#include "app.h"
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// 在此添加用户全局变量定义、用户宏定义以及函数声明  
#define MAIN_STARTUP_DELAY_MS           2000

static void SYS_ClockInit(void);
//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// 项目主函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void main(void)
{
    //<<AICUBE_USER_MAIN_INITIAL_BEGIN>>
    // 仅保留驱动测试路径：74HC595选通第一行 + APP例程。
    SYS_Init();
    HC595_Init();
    HC595_SelectRows(0, 0xFF);
    //<<AICUBE_USER_MAIN_INITIAL_END>>

    //<<AICUBE_USER_MAIN_CODE_BEGIN>>
    APP_Init();
    //<<AICUBE_USER_MAIN_CODE_END>>

    while (1)
    {
        //<<AICUBE_USER_MAIN_LOOP_BEGIN>>
        APP_TaskLoop();
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
    SYS_ClockInit();                    //设置系统主时钟为33.1776MHz
    IAP_SetTimeBase();                  //设置IAP等待参数,产生1us时基

    //<<AICUBE_USER_PREINITIAL_CODE_BEGIN>>
    // 在此添加用户预初始化代码  
    //<<AICUBE_USER_PREINITIAL_CODE_END>>

    PORT2_Init();                       //P2口初始化
    PORT0_Init();                       //P0口初始化
    PORT1_Init();                       //P1口初始化
    PORT3_Init();                       //P3口初始化
    PORT4_Init();                       //P4口初始化
    EXTI0_Init();                       //P3.2按键中断初始化（低电平按键按下触发）
    EXTI1_Init();                      //P3.3按键中断初始化（低电平按键按下触发）
    TIMER0_Init();                      //定时器0初始化
    // PWMA_Init();                        //高级PWMA初始化
    delay_ms(1);
    USBLIB_Init();                      //USB库初始化
    delay_ms(1);

    //<<AICUBE_USER_INITIAL_CODE_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_INITIAL_CODE_END>>

    EnableGlobalInt();                  //使能全局中断
}

static void SYS_ClockInit(void)
{
    CLK_HSIOCK_Divider(1);
    CLK_SPICLK_Divider(1);
    CLK_I2SCLK_Divider(1);
    CLK_PWMACLK_Divider(1);
    CLK_PWMBCLK_Divider(1);
    CLK_TFPUCLK_Divider(1);

    CLK_IRC48M_Enable();
    CLK_IRC48M_WaitStable();

    CLK_SYSCLK_Divider(10);
    HIRC_33M1776();
    CLK_MCLK_HIRC();
    CLK_MCLK2_BYPASS();
    CLK_SYSCLK_Divider(1);
    CLK_HSIOCK_MCLK();
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
        NOP(27);                        //(MAIN_Fosc + 500000) / 1000000 - 6
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
// 在此添加用户函数实现代码  
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


