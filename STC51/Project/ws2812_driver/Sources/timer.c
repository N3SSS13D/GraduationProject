//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: timer.c
// 文件描述: 定时器驱动，包含定时器0初始化和中断处理
// 文件版本: V1.0
// 修改记录:
//   1. (2025-12-28) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"


//<<AICUBE_USER_INCLUDE_BEGIN>>
// 在此添加用户头文件包含  
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// 定时器中断计数
static int timer0_count = 0;
//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// 定时器0初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void TIMER0_Init(void)
{
#define T0_PSCR                 (51)
#define T0_RELOAD               (65536 - (float)SYSCLK / 12 / (T0_PSCR + 1) * 1 / 1) //定时周期1秒

    TIMER0_TimerMode();                 //设置定时器0为定时模式
    TIMER0_12TMode();                   //设置定时器0为12T模式
    TIMER0_Mode0();                     //设置定时器0为模式0 (16位自动重载模式)
    TIMER0_DisableGateINT0();           //禁止定时器0门控
    TIMER0_SetIntPriority(0);           //设置中断为最低优先级
    TIMER0_EnableInt();                 //使能定时器0中断
    TIMER0_SetPrescale(T0_PSCR);        //设置定时器0的8位预分频
    TIMER0_SetReload16(T0_RELOAD);      //设置定时器0的16位重载值
    TIMER0_Run();                       //定时器0开始运行

    //<<AICUBE_USER_TIMER0_INITIAL_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_TIMER0_INITIAL_END>>
}


////////////////////////////////////////
// 定时器0中断服务程序
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void TIMER0_ISR(void) interrupt TMR0_VECTOR
{
    //<<AICUBE_USER_TIMER0_ISR_CODE1_BEGIN>>
    // 在此添加中断函数用户代码  
    //<<AICUBE_USER_TIMER0_ISR_CODE1_END>>
}


//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码  
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


