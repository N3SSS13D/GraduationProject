//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: timer.c
// 文件描述: 
// 文件版本: V1.0
// 修改记录:
//   1. (2026-03-16) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"


//<<AICUBE_USER_INCLUDE_BEGIN>>
// 在此添加用户头文件包含  
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// 在此添加用户全局变量定义、用户宏定义以及函数声明  
static volatile uint32_t g_timerTickMs = 0;
static Timer0Hook1ms_t g_timer0Hook = NULL;
//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// 定时器0初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void TIMER0_Init(void)
{
#define T0_PSCR                 (1)
#define T0_RELOAD               (65536 - (float)SYSCLK / (T0_PSCR + 1) * 1 / 1000) //定时周期1毫秒

    TIMER0_TimerMode();                 //设置定时器0为定时模式
    TIMER0_1TMode();                    //设置定时器0为1T模式
    TIMER0_Mode0();                     //设置定时器0为模式0 (16位自动重载模式)
    TIMER0_DisableGateINT0();           //禁止定时器0门控
    TIMER0_SetPrescale(T0_PSCR);        //设置定时器0的8位预分频
    TIMER0_SetReload16(T0_RELOAD);      //设置定时器0的16位重载值
    TIMER0_ClearFlag();
    TIMER0_EnableInt();
    TIMER0_Run();                       //定时器0开始运行

    //<<AICUBE_USER_TIMER0_INITIAL_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_TIMER0_INITIAL_END>>
}



//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码  
uint32_t TIMER0_GetTickMs(void)
{
    uint32_t tick;

    DisableGlobalInt();
    tick = g_timerTickMs;
    EnableGlobalInt();

    return tick;
}

void TIMER0_Register1msHook(Timer0Hook1ms_t hook)
{
    g_timer0Hook = hook;
}

void TIMER0_ISR(void) interrupt 1
{
    TIMER0_ClearFlag();
    g_timerTickMs++;

    if (g_timer0Hook != NULL)
    {
        g_timer0Hook();
    }
}
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


