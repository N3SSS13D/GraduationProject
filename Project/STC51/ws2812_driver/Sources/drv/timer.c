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
#define TIMER0_NULL_HOOK_1MS            ((Timer0Hook1ms_t)0)
#define TIMER0_NULL_HOOK_US             ((Timer0HookUs_t)0)
static Timer0Hook1ms_t g_timer0Hook = TIMER0_NULL_HOOK_1MS;
static Timer0HookUs_t g_timer0UsHook = TIMER0_NULL_HOOK_US;
static volatile uint16_t g_timer0RemainMs = 0;
static volatile bit g_timer0IsRunning = 0;

#define TIMER0_US_MIN                   1UL
#define TIMER0_1MS_TICKS               ((uint16_t)((MAIN_Fosc + 500UL) / 1000UL))
#define TIMER0_1MS_RELOAD              ((uint16_t)(65536UL - TIMER0_1MS_TICKS))
//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// 定时器0初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void TIMER0_Init(void)
{
    TIMER0_TimerMode();                 //设置定时器0为定时模式
    TIMER0_1TMode();                    //设置定时器0为1T模式
    TIMER0_Mode0();                     //设置定时器0为模式0 (16位自动重载模式)
    TIMER0_DisableGateINT0();           //禁止定时器0门控
    TIMER0_SetPrescale(0U);
    TIMER0_SetReload16(TIMER0_1MS_RELOAD);
    TIMER0_ClearFlag();
    TIMER0_EnableInt();
    TIMER0_Stop();

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

void TIMER0_RegisterUsHook(Timer0HookUs_t hook)
{
    g_timer0UsHook = hook;
}

void TIMER0_StartOneShotUs(uint32_t delayUs)
{
    uint32_t delayMs;

    if (delayUs < TIMER0_US_MIN)
    {
        delayUs = TIMER0_US_MIN;
    }

    delayMs = (delayUs + 999UL) / 1000UL;
    if (delayMs == 0UL)
    {
        delayMs = 1UL;
    }
    if (delayMs > 65535UL)
    {
        delayMs = 65535UL;
    }

    TIMER0_Stop();
    TIMER0_SetReload16(TIMER0_1MS_RELOAD);
    g_timer0RemainMs = (uint16_t)delayMs;
    g_timer0IsRunning = 1;
    TIMER0_ClearFlag();
    TIMER0_Run();
}

void TIMER0_StopOneShot(void)
{
    TIMER0_Stop();
    g_timer0RemainMs = 0U;
    g_timer0IsRunning = 0;
}

void TIMER0_ISR(void) interrupt 1
{
    TIMER0_ClearFlag();
    TIMER0_Stop();
    g_timerTickMs++;

    if ((g_timer0IsRunning != 0) && (g_timer0RemainMs > 1U))
    {
        g_timer0RemainMs--;
        TIMER0_SetReload16(TIMER0_1MS_RELOAD);
        TIMER0_ClearFlag();
        TIMER0_Run();

        return;
    }

    g_timer0IsRunning = 0;
    g_timer0RemainMs = 0U;

    if (g_timer0UsHook != NULL)
    {
        g_timer0UsHook();
    }

    if (g_timer0Hook != NULL)
    {
        g_timer0Hook();
    }
}
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


