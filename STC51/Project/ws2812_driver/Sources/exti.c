//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: exti.c
// 文件描述: 
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
// 在此添加用户全局变量定义、用户宏定义以及函数声明  
//<<AICUBE_USER_GLOBAL_DEFINE_END>>



////////////////////////////////////////
// 外部中断INT0初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void EXTI0_Init(void)
{
    INT0_FallingInt();                  //设置外部中断为下降沿中断
    INT0_SetIntPriority(0);             //设置中断为最低优先级
    INT0_EnableInt();                   //使能外部中断

    //<<AICUBE_USER_EXTI0_INITIAL_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_EXTI0_INITIAL_END>>
}

////////////////////////////////////////
// 外部中断INT1初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void EXTI1_Init(void)
{
    INT1_FallingInt();                  //设置外部中断为下降沿中断
    INT1_SetIntPriority(0);             //设置中断为最低优先级
    INT1_EnableInt();                   //使能外部中断

    //<<AICUBE_USER_EXTI1_INITIAL_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_EXTI1_INITIAL_END>>
}


////////////////////////////////////////
// 外部中断INT0中断服务程序
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void EXTI0_ISR(void) interrupt INT0_VECTOR
{
    //<<AICUBE_USER_EXTI0_ISR_CODE1_BEGIN>>
    // 在此添加中断函数用户代码  
    //<<AICUBE_USER_EXTI0_ISR_CODE1_END>>
}

////////////////////////////////////////
// 外部中断INT1中断服务程序
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void EXTI1_ISR(void) interrupt INT1_VECTOR
{
    //<<AICUBE_USER_EXTI1_ISR_CODE1_BEGIN>>
    // 在此添加中断函数用户代码  
    //<<AICUBE_USER_EXTI1_ISR_CODE1_END>>
}


//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码  
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


