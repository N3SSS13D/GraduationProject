//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: spi.c
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


BOOL fSPITransBusy;                     //SPI读写忙标志
uint8_t xdata pu8SPIDMATxBuffer[192];   //SPI DMA发送缓冲区数组

////////////////////////////////////////
// SPI初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void SPI_Init(void)
{
    SPI_SwitchP4n();                    //选择SPI数据口: SS(P4.0), MOSI(P4.1), MISO(P4.2), SCLK(P4.3)

    SPI_MasterMode();                   //设置SPI为主机模式
    SPI_IgnoreSS();                     //忽略SS脚
    SPI_DataMSB();                      //设置SPI数据顺序为MSB (高位在前)
    SPI_SetMode0();                     //设置SPI工作模式0 (CPOL=0, CPHA=0)
    SPI_SetClockDivider4();             //设置SPI时钟分频

    HSSPI_Disable();                    //关闭SPI高速模式

    SPI_SetIntPriority(0);              //设置中断为最低优先级
    SPI_EnableInt();                    //使能SPI中断
    fSPITransBusy = 0;                  //清除SPI忙标志位

    SPI_Enable();                       //使能SPI功能

    DMA_SPI_ManualSS();                 //关闭SPI DMA自动控制SS功能
    DMA_SPI_SetAmount(191);             //设置SPI DMA发送/接收字节数
    DMA_SPI_SetTxAddress(pu8SPIDMATxBuffer); //设置SPI DMA发送缓冲区地址
    DMA_SPI_SetInterval(0);             //设置SPI DMA发送/接收字节间隔时间（系统时钟）
    DMA_SPI_ClearFIFO();                //清空SPI DMA FIFO缓冲区
    DMA_SPI_ClearFlag();                //清除SPI DMA中断标志
    DMA_SPI_EnableTx();                 //使能发送数据
    DMA_SPI_DisableRx();                //禁止接收数据
    DMA_SPI_SetBusPriority(0);          //设置总线访问为最低优先级
    DMA_SPI_SetIntPriority(0);          //设置中断为最低优先级
    DMA_SPI_EnableInt();                //使能SPI DMA中断
    DMA_SPI_Enable();                   //使能SPI DMA功能
//  DMA_SPI_MasterTrigger();            //触发SPI主机DMA

    //<<AICUBE_USER_SPI_INITIAL_BEGIN>>
    // 在此添加用户初始化代码  
    //<<AICUBE_USER_SPI_INITIAL_END>>
}

////////////////////////////////////////
// SPI主机模式发送字节函数
// 入口参数: dat (待发送的字节数据)
// 函数返回: 无
////////////////////////////////////////
void SPI_WriteByte(uint8_t dat)
{
    fSPITransBusy = 1;                  //设置发送忙标志
    SPI_SendData(dat);                  //触发主机发送数据
    while (fSPITransBusy);              //等待发送完成
}

////////////////////////////////////////
// SPI主机模式读取字节函数
// 入口参数: 无
// 函数返回: 读取的字节数据
////////////////////////////////////////
uint8_t SPI_ReadByte(void)
{
    fSPITransBusy = 1;                  //设置读取忙标志
    SPI_SendData(0xff);                 //触发主机读取数据（主机发送时钟信号）
    while (fSPITransBusy);              //等待读取完成
    return SPI_ReadData();
}


////////////////////////////////////////
// SPI中断服务程序
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void SPI_ISR(void) interrupt SPI_VECTOR
{
    //<<AICUBE_USER_SPI_ISR_CODE1_BEGIN>>
    // 在此添加中断函数用户代码  
    if (SPI_CheckFlag())                //判断SPI中断
    {
        SPI_ClearFlag();                //清除SPI中断标志
        fSPITransBusy = 0;              //清除SPI忙标志位
    }
    //<<AICUBE_USER_SPI_ISR_CODE1_END>>
}

////////////////////////////////////////
// SPI DMA中断服务程序
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void DMA_SPI_ISR(void) interrupt DMA_SPI_VECTOR
{
    //<<AICUBE_USER_SPI_ISR_CODE2_BEGIN>>
    // 在此添加中断函数用户代码  
    if (DMA_SPI_CheckFlag())            //判断SPI DMA中断
    {
        DMA_SPI_ClearFlag();            //清除SPI DMA中断标志
    }
    //<<AICUBE_USER_SPI_ISR_CODE2_END>>
}


//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码  
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


