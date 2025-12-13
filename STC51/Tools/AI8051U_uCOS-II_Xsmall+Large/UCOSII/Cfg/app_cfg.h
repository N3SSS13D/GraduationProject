/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                            EXAMPLE CODE
*
* Filename : app_cfg.h
*********************************************************************************************************
*/

#ifndef  _APP_CFG_H_
#define  _APP_CFG_H_


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <config.h>


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           TASK PRIORITIES
*********************************************************************************************************
*/
#define  APP_CFG_UART1_MUTEX_PRIO       4u
#define  OS_TASK_TMR_PRIO               5u
#define  APP_CFG_STARTUP_TASK_PRIO      10u
#define  APP_CFG_TASK_B_PRIO            11u
#define  APP_CFG_TASK_C_PRIO            12u


/*
*********************************************************************************************************
*                                          TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/
#define  APP_CFG_STARTUP_TASK_STK_SIZE  128u
#define  APP_CFG_TASK_B_STK_SIZE        128u
#define  APP_CFG_TASK_C_STK_SIZE        128u



/*------------------------------------------------------------------------------
信号量声明, 任务声明
------------------------------------------------------------------------------*/
extern OS_EVENT    *Uart1Mutex;

extern  OS_STK STARTUP_TASK_STK[APP_CFG_STARTUP_TASK_STK_SIZE];     ///任务栈
void STARTUP_TASK(void *ppdata);                                    //任务函数    

extern  OS_STK TASK_B_STK[APP_CFG_TASK_B_STK_SIZE];                 //任务栈
void TASK_B(void *ppdata);                                          //任务函数

extern  OS_STK TASK_C_STK[APP_CFG_TASK_C_STK_SIZE];                 //任务栈
void TASK_C(void *ppdata);                                          //任务函数


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                    0u
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                   1u
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                    2u
#endif

#define  APP_TRACE_LEVEL                   TRACE_LEVEL_OFF
#define  APP_TRACE                         printf

#define  APP_TRACE_INFO(x)    ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)     ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_TRACE x) : (void)0)


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of module include.              */
