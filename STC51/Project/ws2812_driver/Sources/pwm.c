//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: pwm.c
// 文件描述: 
// 文件版本: V1.0
// 修改记录:
//   1. (2026-01-25) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"


//<<AICUBE_USER_INCLUDE_BEGIN>>
// 在此添加用户头文件包含  
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// 在此添加用户全局变量定义、用户宏定义以及函数声明  
//<<AICUBE_USER_GLOBAL_DEFINE_END>>


uint8_t xdata pu8PWMADMATxBuffer[100];  //串口ADMA发送缓冲区数组

////////////////////////////////////////
// PWMA初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void PWMA_Init(void)
{
    PWMA_C1SwitchP1011();               //设置PWMA通道1数据端口: PWM1P (P1.0), PWM1N (P1.1)

    HSPWMA_Disable();                   //关闭高速模式，使用同步方式进行初始化
    PWMA_InternalClockMode();           //选择内部时钟模式
    PWMA_SetClockDivider(0);            //设置16位预分频
    PWMA_SetReload16(999);              //设置16位重载值
    PWMA_BufferARR();                   //使能重载值寄存器预装载功能
    PWMA_SetCounter(0);                 //初始化计数值

    PWMA_CC1PDisable();                 //关闭通道
    PWMA_CC1Output();                   //使能通道输出功能
    PWMA_CC1PEnable();                  //打开通道
    PWM_UpdateDuty(PWMA_CH1, 333);      //设置通道的PWM占空比
    PWMA_OC1REFPWMMode1();              //设置通道输出参考信号为PWM模式1信号
    PWMA_BufferCCR1();                  //使能CCRn预装载功能
    PWMA_CC1PNonInverted();             //设置正极通道输出高电平有效
    PWMA_EnablePWM1POutput();           //使能正极通道输出
    PWMA_DisablePWM1NOutput();          //关闭负极通道输出

    PWMA_EnableMainOutput();            //使能PWM主输出

    PWMA_UpdateIntAsDMAEvent();         //设置PWMA更新中断作为DMA事件
    PWMA_SetDMABaseAddress(13);         //设置PWMA DMA的寄存器基地址
    PWMA_SetDMABurstCount(0);           //设置PWMA DMA突发传输次数
    PWMA_SetDMABurst1Byte();            //设置PWMA DMA突发传输字节数
    PWMA_DMASkipSpace();                //设置PWMA DMA时跳过保留寄存器（空白字节）
    PWMA_SetDMAOutput();                //设置PWMA DMA方向为输出（DMA输出PWM波形）

    DMA_PWMA_SetTxAmount(99);           //设置PWMA发送DMA总字节数
    DMA_PWMA_SetTxAddress(pu8PWMADMATxBuffer); //设置PWMA发送DMA缓冲区地址
    DMA_PWMA_SetInterval(0);            //设置PWMA发送DMA字节间隔时间（系统时钟）
    DMA_PWMA_ClearTxFlag();             //清除PWMA发送DMA中断标志
    DMA_PWMA_SetTxBusPriority(0);       //设置总线访问为最低优先级
    DMA_PWMA_EnableTx();                //使能PWMA发送DMA功能
    PWMA_EnableDMA();                   //设置PWMA DMA方向为输出（DMA输出PWM波形）

    PWMA_Run();                         //PWMA开始运行
//  DMA_PWMA_TriggerTx();               //触发PWMA发送DMA

    //<<AICUBE_USER_PWM0_INITIAL_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_PWM0_INITIAL_END>>
}

////////////////////////////////////////
// 设置PWM通道输出占空比
// 入口参数: PWMx: (目标PWM组和通道索引)
//           nCompare: (PWM占空比值)
//           iMode: (PWM输出模式：PWM_MODE1/PWM_MODE2)
// 函数返回: 无
////////////////////////////////////////
void PWM_UpdateDuty(uint8_t PWMx, uint16_t nCompare)
{
    switch (PWMx)
    {
    case PWMA_CH1:
        PWMA_SetCCR1Value(nCompare);    //设置通道比较值
        break;
    case PWMA_CH2:
        PWMA_SetCCR2Value(nCompare);    //设置通道比较值
        break;
    case PWMA_CH3:
        PWMA_SetCCR3Value(nCompare);    //设置通道比较值
        break;
    case PWMA_CH4:
        PWMA_SetCCR4Value(nCompare);    //设置通道比较值
        break;
    case PWMB_CH5:
        PWMB_SetCCR5Value(nCompare);    //设置通道比较值
        break;
    case PWMB_CH6:
        PWMB_SetCCR6Value(nCompare);    //设置通道比较值
        break;
    case PWMB_CH7:
        PWMB_SetCCR7Value(nCompare);    //设置通道比较值
        break;
    case PWMB_CH8:
        PWMB_SetCCR8Value(nCompare);    //设置通道比较值
        break;
    }
}

////////////////////////////////////////
// 读取PWM通道捕获值
// 入口参数: PWMx: (目标PWM组和通道索引)
// 函数返回: 捕获值
////////////////////////////////////////
uint16_t PWM_ReadCapture(uint8_t PWMx)
{
    uint16_t cap;

    switch (PWMx)
    {
    case PWMA_CH1:  cap = PWMA_ReadCCR1Value(); break;
    case PWMA_CH2:  cap = PWMA_ReadCCR2Value(); break;
    case PWMA_CH3:  cap = PWMA_ReadCCR3Value(); break;
    case PWMA_CH4:  cap = PWMA_ReadCCR4Value(); break;
    case PWMB_CH5:  cap = PWMB_ReadCCR5Value(); break;
    case PWMB_CH6:  cap = PWMB_ReadCCR6Value(); break;
    case PWMB_CH7:  cap = PWMB_ReadCCR7Value(); break;
    case PWMB_CH8:  cap = PWMB_ReadCCR8Value(); break;
    default: cap = 0;
    }

    return cap;
}

////////////////////////////////////////
// 设置高速模式PWM通道输出占空比
// 入口参数: PWMx: (目标PWM组和通道索引)
//           nCompare: (PWM占空比值)
// 函数返回: 无
////////////////////////////////////////
void HSPWM_UpdateDuty(uint8_t PWMx, uint16_t nCompare)
{
    switch (PWMx)
    {
    case PWMA_CH1:
        HSPWMA_SetCCR1Value(nCompare);  //设置通道比较值
        break;
    case PWMA_CH2:
        HSPWMA_SetCCR2Value(nCompare);  //设置通道比较值
        break;
    case PWMA_CH3:
        HSPWMA_SetCCR3Value(nCompare);  //设置通道比较值
        break;
    case PWMA_CH4:
        HSPWMA_SetCCR4Value(nCompare);  //设置通道比较值
        break;
    case PWMB_CH5:
        HSPWMB_SetCCR5Value(nCompare);  //设置通道比较值
        break;
    case PWMB_CH6:
        HSPWMB_SetCCR6Value(nCompare);  //设置通道比较值
        break;
    case PWMB_CH7:
        HSPWMB_SetCCR7Value(nCompare);  //设置通道比较值
        break;
    case PWMB_CH8:
        HSPWMB_SetCCR8Value(nCompare);  //设置通道比较值
        break;
    }
}

////////////////////////////////////////
// 读取高速模式PWM通道捕获值
// 入口参数: PWMx: (目标PWM组和通道索引)
// 函数返回: 捕获值
////////////////////////////////////////
uint16_t HSPWM_ReadCapture(uint8_t PWMx)
{
    uint16_t cap;

    switch (PWMx)
    {
    case PWMA_CH1:  cap = HSPWMA_ReadCCR1Value();   break;
    case PWMA_CH2:  cap = HSPWMA_ReadCCR2Value();   break;
    case PWMA_CH3:  cap = HSPWMA_ReadCCR3Value();   break;
    case PWMA_CH4:  cap = HSPWMA_ReadCCR4Value();   break;
    case PWMB_CH5:  cap = HSPWMB_ReadCCR5Value();   break;
    case PWMB_CH6:  cap = HSPWMB_ReadCCR6Value();   break;
    case PWMB_CH7:  cap = HSPWMB_ReadCCR7Value();   break;
    case PWMB_CH8:  cap = HSPWMB_ReadCCR8Value();   break;
    default: cap = 0;
    }

    return cap;
}

////////////////////////////////////////
// 异步方式读取PWMA特殊功能寄存器
// 入口参数: addr: (PWMA特殊功能寄存器的低7位)
// 函数返回: 寄存器的值
////////////////////////////////////////;
uint8_t HSPWMA_ReadReg(uint8_t addr)
{
    uint8_t dat;

    while (HSPWMA_CheckBusy());         //等待前一个异步读写完成
    HSPWMA_AsyncRead(addr, dat);        //触发异步方式读取寄存器

    return dat;
}

////////////////////////////////////////
// 异步方式写PWMA特殊功能寄存器
// 入口参数: addr: (PWMA特殊功能寄存器的低7位)
//           dat: (待写入的数据)
// 函数返回: 无
////////////////////////////////////////;
void HSPWMA_WriteReg(uint8_t addr, uint8_t dat)
{
    while (HSPWMA_CheckBusy());         //等待前一个异步读写完成
    HSPWMA_AsyncWrite(addr, dat);       //触发异步方式写寄存器
}

////////////////////////////////////////
// 异步方式读取PWMB特殊功能寄存器
// 入口参数: addr: (PWMB特殊功能寄存器的低7位)
// 函数返回: 寄存器的值
////////////////////////////////////////;
uint8_t HSPWMB_ReadReg(uint8_t addr)
{
    uint8_t dat;

    while (HSPWMB_CheckBusy());         //等待前一个异步读写完成
    HSPWMB_AsyncRead(addr, dat);        //触发异步方式读取寄存器

    return dat;
}

////////////////////////////////////////
// 异步方式写PWMB特殊功能寄存器
// 入口参数: addr: (PWMB特殊功能寄存器的低7位)
//           dat: (待写入的数据)
// 函数返回: 无
////////////////////////////////////////;
void HSPWMB_WriteReg(uint8_t addr, uint8_t dat)
{
    while (HSPWMB_CheckBusy());         //等待前一个异步读写完成
    HSPWMB_AsyncWrite(addr, dat);       //触发异步方式写寄存器
}



//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码  
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


