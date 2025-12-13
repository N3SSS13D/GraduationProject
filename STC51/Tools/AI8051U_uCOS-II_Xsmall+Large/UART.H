/*---------------------------------------------------------
UART.H
----------------------------------------------------------*/

#ifndef __UART_H_
#define __UART_H_

#include "config.H"
#include "STC32G_Switch.H"
#include "STC32G_GPIO.h"
#include "STC32G_NVIC.h"


/*---------------------------------------------------------
设置
----------------------------------------------------------*/
#define	printf_UART	            1       //printf使用哪个串口 1/2/3/4

#define	UART1_EN 	            1       //使用哪些串口就开对应的定义1
#define	UART2_EN 	            0
#define	UART3_EN 	            0
#define	UART4_EN 	            0


#define UART1_TX_BUF_SIZE       256     //设置数据池大小
#define UART1_RX_BUF_SIZE       128     //设置数据池大小
#define UART1_USE_DMA_EN        1       //0:中断模式, 1:DMA模式
#define	UART1_set485RxMode()
#define	UART1_set485TxMode()

#define UART2_TX_BUF_SIZE       256     //设置数据池大小
#define UART2_RX_BUF_SIZE       128     //设置数据池大小
#define UART2_USE_DMA_EN        1       //0:中断模式, 1:DMA模式
#define	UART2_set485RxMode()
#define	UART2_set485TxMode()

#define UART3_TX_BUF_SIZE       256     //设置数据池大小
#define UART3_RX_BUF_SIZE       128     //设置数据池大小
#define UART3_USE_DMA_EN        1       //0:中断模式, 1:DMA模式
#define	UART3_set485RxMode()
#define	UART3_set485TxMode()

#define UART4_TX_BUF_SIZE       256     //设置数据池大小
#define UART4_RX_BUF_SIZE       128     //设置数据池大小
#define UART4_USE_DMA_EN        1       //0:中断模式, 1:DMA模式
#define	UART4_set485RxMode()
#define	UART4_set485TxMode()


/*---------------------------------------------------------
常量
----------------------------------------------------------*/
#define	UART1_BRT_Timer1    1
#define	UART1_BRT_Timer2    2
#define	UART2_BRT_Timer2    2
#define	UART3_BRT_Timer2    2
#define	UART3_BRT_Timer3    3
#define	UART4_BRT_Timer2    2
#define	UART4_BRT_Timer4    4


/*---------------------------------------------------------
函数
----------------------------------------------------------*/
#if UART1_EN  == 1
void UART1_Init(u32 BRT, u8 brt_Timer, u8 sw);
void UART1_setBRT( u32 BRT, u8 brt_Timer );
u8   UART1_Send(void *pt, u16 Size);
u16  UART1_Receive(u8 *buf, u16 Size);
u16  UART1_GetCapacity( void );
#endif

#if UART2_EN  == 1
void UART2_Init(u32 BRT, u8 sw);
void UART2_setBRT( u32 BRT );
u8   UART2_Send(void *pt, u16 Size);
u16  UART2_Receive(u8 *buf, u16 Size);
u16  UART2_GetCapacity( void );
#endif

#if UART3_EN  == 1
void UART3_Init(u32 BRT, u8 brt_Timer, u8 sw);
void UART3_setBRT( u32 BRT, u8 brt_Timer );
u8   UART3_Send(void *pt, u16 Size);
u16  UART3_Receive(u8 *buf, u16 Size);
u16  UART3_GetCapacity( void );
#endif

#if UART4_EN  == 1
void UART4_Init(u32 BRT, u8 brt_Timer, u8 sw);
void UART4_setBRT( u32 BRT, u8 brt_Timer );
u8   UART4_Send(void *pt, u16 Size);
u16  UART4_Receive(u8 *buf, u16 Size);
u16  UART4_GetCapacity( void );
#endif


#endif



