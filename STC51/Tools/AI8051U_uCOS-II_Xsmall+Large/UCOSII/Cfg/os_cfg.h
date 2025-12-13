/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                 uC/OS-II Configuration File for V2.9x
*
* Filename : os_cfg.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef OS_CFG_H
#define OS_CFG_H


                                       /* ---------------------- MISCELLANEOUS ----------------------- */
#define OS_APP_HOOKS_EN           1u   /* Application-defined hooks are called from the uC/OS-II hooks */
#define OS_ARG_CHK_EN             1u   /* Enable (1) or Disable (0) argument checking                  */
#define OS_CPU_HOOKS_EN           1u   /* uC/OS-II hooks are found in the processor port files         */

#define OS_DEBUG_EN               0u   /* Enable(1) debug variables                                    */

#define OS_EVENT_MULTI_EN         0u   /* Include code for OSEventPendMulti()                          */
#define OS_EVENT_NAME_EN          0u   /* Enable names for Sem, Mutex, Mbox and Q                      */

#define OS_LOWEST_PRIO           15u   /* Defines the lowest priority that can be assigned ...         */
                                       /* ... MUST NEVER be higher than 254!                           */

#define OS_MAX_EVENTS             16u  /* Max. number of event control blocks in your application      */
#define OS_MAX_FLAGS              2u   /* Max. number of Event Flag Groups    in your application      */
#define OS_MAX_MEM_PART           2u   /* Max. number of memory partitions                             */
#define OS_MAX_QS                 2u   /* Max. number of queue control blocks in your application      */
#define OS_MAX_TASKS              8u   /* Max. number of tasks in your application, MUST be >= 2       */

#define OS_SCHED_LOCK_EN          1u   /* Include code for OSSchedLock() and OSSchedUnlock()           */

#define OS_TICK_STEP_EN           0u   /* Enable tick stepping feature for uC/OS-View                  */
#define OS_TICKS_PER_SEC        1000u   /* Set the number of ticks in one second                        */

#define OS_TLS_TBL_SIZE           0u   /* Size of Thread-Local Storage Table                           */


                                       /* --------------------- TASK STACK SIZE ---------------------- */
#define OS_TASK_TMR_STK_SIZE    128u   /* Timer      task stack size (# of OS_STK wide entries)        */
#define OS_TASK_STAT_STK_SIZE   128u   /* Statistics task stack size (# of OS_STK wide entries)        */
#define OS_TASK_IDLE_STK_SIZE   96u   /* Idle       task stack size (# of OS_STK wide entries)        */


                                       /* --------------------- TASK MANAGEMENT ---------------------- */
#define OS_TASK_CHANGE_PRIO_EN    1u   /*     Include code for OSTaskChangePrio()                      */
#define OS_TASK_CREATE_EN         1u   /*     Include code for OSTaskCreate()                          */
#define OS_TASK_CREATE_EXT_EN     1u   /*     Include code for OSTaskCreateExt()                       */
#define OS_TASK_DEL_EN            0u   /*     Include code for OSTaskDel()                             */
#define OS_TASK_NAME_EN           0u   /*     Enable task names                                        */
#define OS_TASK_PROFILE_EN        1u   /*     Include variables in OS_TCB for profiling                */
#define OS_TASK_QUERY_EN          1u   /*     Include code for OSTaskQuery()                           */
#define OS_TASK_REG_TBL_SIZE      0u   /*     Size of task variables array (#of INT32U entries)        */
#define OS_TASK_STAT_EN           1u   /*     Enable (1) or Disable(0) the statistics task             */
#define OS_TASK_STAT_STK_CHK_EN   1u   /*     Check task stacks from statistic task                    */
#define OS_TASK_SUSPEND_EN        1u   /*     Include code for OSTaskSuspend() and OSTaskResume()      */
#define OS_TASK_SW_HOOK_EN        0u   /*     Include code for OSTaskSwHook()                          */


                                       /* ----------------------- EVENT FLAGS ------------------------ */
#define OS_FLAG_EN                1u   /* Enable (1) or Disable (0) code generation for EVENT FLAGS    */
#define OS_FLAG_ACCEPT_EN         1u   /*     Include code for OSFlagAccept()                          */
#define OS_FLAG_DEL_EN            1u   /*     Include code for OSFlagDel()                             */
#define OS_FLAG_NAME_EN           1u   /*     Enable names for event flag group                        */
#define OS_FLAG_QUERY_EN          1u   /*     Include code for OSFlagQuery()                           */
#define OS_FLAG_WAIT_CLR_EN       1u   /* Include code for Wait on Clear EVENT FLAGS                   */
#define OS_FLAGS_NBITS            8u   /* Size in #bits of OS_FLAGS data type (8, 16 or 32)            */


                                       /* -------------------- MESSAGE MAILBOXES --------------------- */
#define OS_MBOX_EN                1u   /* Enable (1) or Disable (0) code generation for MAILBOXES      */
#define OS_MBOX_ACCEPT_EN         1u   /*     Include code for OSMboxAccept()                          */
#define OS_MBOX_DEL_EN            1u   /*     Include code for OSMboxDel()                             */
#define OS_MBOX_PEND_ABORT_EN     1u   /*     Include code for OSMboxPendAbort()                       */
#define OS_MBOX_POST_EN           1u   /*     Include code for OSMboxPost()                            */
#define OS_MBOX_POST_OPT_EN       1u   /*     Include code for OSMboxPostOpt()                         */
#define OS_MBOX_QUERY_EN          1u   /*     Include code for OSMboxQuery()                           */


                                       /* --------------------- MEMORY MANAGEMENT -------------------- */
#define OS_MEM_EN                 1u   /* Enable (1) or Disable (0) code generation for MEMORY MANAGER */
#define OS_MEM_NAME_EN            1u   /*     Enable memory partition names                            */
#define OS_MEM_QUERY_EN           1u   /*     Include code for OSMemQuery()                            */


                                       /* ---------------- MUTUAL EXCLUSION SEMAPHORES --------------- */
#define OS_MUTEX_EN               1u   /* Enable (1) or Disable (0) code generation for MUTEX          */
#define OS_MUTEX_ACCEPT_EN        1u   /*     Include code for OSMutexAccept()                         */
#define OS_MUTEX_DEL_EN           1u   /*     Include code for OSMutexDel()                            */
#define OS_MUTEX_QUERY_EN         1u   /*     Include code for OSMutexQuery()                          */


                                       /* ---------------------- MESSAGE QUEUES ---------------------- */
#define OS_Q_EN                   1u   /* Enable (1) or Disable (0) code generation for QUEUES         */
#define OS_Q_ACCEPT_EN            1u   /*     Include code for OSQAccept()                             */
#define OS_Q_DEL_EN               1u   /*     Include code for OSQDel()                                */
#define OS_Q_FLUSH_EN             1u   /*     Include code for OSQFlush()                              */
#define OS_Q_PEND_ABORT_EN        1u   /*     Include code for OSQPendAbort()                          */
#define OS_Q_POST_EN              1u   /*     Include code for OSQPost()                               */
#define OS_Q_POST_FRONT_EN        1u   /*     Include code for OSQPostFront()                          */
#define OS_Q_POST_OPT_EN          1u   /*     Include code for OSQPostOpt()                            */
#define OS_Q_QUERY_EN             1u   /*     Include code for OSQQuery()                              */


                                       /* ------------------------ SEMAPHORES ------------------------ */
#define OS_SEM_EN                 1u   /* Enable (1) or Disable (0) code generation for SEMAPHORES     */
#define OS_SEM_ACCEPT_EN          1u   /*    Include code for OSSemAccept()                            */
#define OS_SEM_DEL_EN             1u   /*    Include code for OSSemDel()                               */
#define OS_SEM_PEND_ABORT_EN      1u   /*    Include code for OSSemPendAbort()                         */
#define OS_SEM_QUERY_EN           1u   /*    Include code for OSSemQuery()                             */
#define OS_SEM_SET_EN             1u   /*    Include code for OSSemSet()                               */


                                       /* --------------------- TIME MANAGEMENT ---------------------- */
#define OS_TIME_DLY_HMSM_EN       1u   /*     Include code for OSTimeDlyHMSM()                         */
#define OS_TIME_DLY_RESUME_EN     1u   /*     Include code for OSTimeDlyResume()                       */
#define OS_TIME_GET_SET_EN        1u   /*     Include code for OSTimeGet() and OSTimeSet()             */
#define OS_TIME_TICK_HOOK_EN      1u   /*     Include code for OSTimeTickHook()                        */


                                       /* --------------------- TIMER MANAGEMENT --------------------- */
#define OS_TMR_EN                 1u   /* Enable (1) or Disable (0) code generation for TIMERS         */
#define OS_TMR_CFG_MAX            4u   /*     Maximum number of timers                                 */
#define OS_TMR_CFG_NAME_EN        1u   /*     Determine timer names                                    */
#define OS_TMR_CFG_WHEEL_SIZE     8u   /*     Size of timer wheel (#Spokes)                            */
#define OS_TMR_CFG_TICKS_PER_SEC  50u  /*     Rate at which timer management task runs (Hz)            */


                                       /* ---------------------- TRACE RECORDER ---------------------- */
#define OS_TRACE_EN               0u   /* Enable (1) or Disable (0) uC/OS-II Trace instrumentation     */
#define OS_TRACE_API_ENTER_EN     0u   /* Enable (1) or Disable (0) uC/OS-II Trace API enter instrum.  */
#define OS_TRACE_API_EXIT_EN      0u   /* Enable (1) or Disable (0) uC/OS-II Trace API exit  instrum.  */



//全局变量存放
#define OS_USE_XDATA              1u   //0:OS定义的全局变量存放在 edata, 1:OS定义的全局变量存放在 xdata


//模拟PendSv中断行为的相关配制
#define  PendSv_InterruptNumber   20      //PendSv使用的中断编号 (TIMTE4)  (使用纯数字,不要带修饰,例如　20u 编译通不过)
#define  PendSv_SetFlag()         T4IF=1  //设置PendSv标志对应的指令或函数
#define  PendSv_ClearFlag()       T4IF=0  //清除PendSv标志对应的指令或函数
#define  PendSv_GetFlag()         T4IF    //返回PendSv标志的表达式或函数


/*------------------------------------------------------------------------------------
                    配制中断是否被OS管理

使用说明： 指定每个中断是否被 OS 管理  (中断内有函数调用时推荐选择1)

* 设置1, 对应的中断,编译器自动创建中断汇编封皮函数,用户不需要再用interrupt关键字再创建一次
* 被OS管理的中断使用独立于任务的中断专用堆栈．
* OS通过勾子来调用用户的中断代码．
* 用户勾子函数中不需要处理"OSIntNesting++"和"OSIntExit()"．因为封皮函数已经处理过了．

例子:
#define OS_ISR19_USE_MSP_EN      1u
void Timer3_ISR_Handler_Hook(void)  //勾子函数已预定义,见下文
{
    xxApp();    //有函数调用时,强烈推荐用方法1, 即中断使用MSP. 可大幅减少每任务栈的使用量.
}


* 设置0, 对应的中断将不受OS管理．
* 用户需要自己用interrupt关键字创建中断函数
* 不受OS管理的中断,不可以调用OS服务,包括"OSIntNesting++"和"OSIntExit()".
* 不受OS管理的中断,不使用中断专用栈, 而是沿用中断触发时当前的任务堆栈.任务栈可控性不友好.
* 中断函数内只有本地代码(没有函数调用时),选择0可提高中断出入效率．(无函数调用推荐0)

例子:
#define OS_ISR19_USE_MSP_EN      0u
void Timer3_ISR_Handler(void) interrupt 19
{
    P2=~P2;     //没有函数调用时,推荐用方法0,直接定义中断函数可提高出入效率.
}


* 无论用哪一种方法,都不用处理 "OSIntNesting++"和"OSIntExit()"．

------------------------------------------------------------------------------------*/

#define OS_ISR0_USE_MSP_EN       0u   /*中断号0*/
#define OS_ISR1_USE_MSP_EN       1u   /*中断号1*/
#define OS_ISR2_USE_MSP_EN       0u   /*中断号2*/
#define OS_ISR3_USE_MSP_EN       0u   /*中断号3*/
#define OS_ISR4_USE_MSP_EN       0u   /*中断号4*/
#define OS_ISR5_USE_MSP_EN       0u   /*中断号5*/
#define OS_ISR6_USE_MSP_EN       0u   /*中断号6*/
#define OS_ISR7_USE_MSP_EN       0u   /*中断号7*/
#define OS_ISR8_USE_MSP_EN       0u   /*中断号8*/
#define OS_ISR9_USE_MSP_EN       0u   /*中断号9*/
#define OS_ISR10_USE_MSP_EN      0u   /*中断号10*/
#define OS_ISR11_USE_MSP_EN      0u   /*中断号11*/
#define OS_ISR12_USE_MSP_EN      0u   /*中断号12*/
#define OS_ISR13_USE_MSP_EN      0u   /*中断号13*/
#define OS_ISR14_USE_MSP_EN      0u   /*中断号14*/
#define OS_ISR15_USE_MSP_EN      0u   /*中断号15*/
#define OS_ISR16_USE_MSP_EN      0u   /*中断号16*/
#define OS_ISR17_USE_MSP_EN      0u   /*中断号17*/
#define OS_ISR18_USE_MSP_EN      0u   /*中断号18*/
#define OS_ISR19_USE_MSP_EN      0u   /*中断号19*/
#define OS_ISR20_USE_MSP_EN      0u   /*中断号20*/
#define OS_ISR21_USE_MSP_EN      0u   /*中断号21*/
#define OS_ISR22_USE_MSP_EN      0u   /*中断号22*/
#define OS_ISR23_USE_MSP_EN      0u   /*中断号23*/
#define OS_ISR24_USE_MSP_EN      0u   /*中断号24*/
#define OS_ISR25_USE_MSP_EN      0u   /*中断号25*/
#define OS_ISR26_USE_MSP_EN      0u   /*中断号26*/
#define OS_ISR27_USE_MSP_EN      0u   /*中断号27*/
#define OS_ISR28_USE_MSP_EN      0u   /*中断号28*/
#define OS_ISR29_USE_MSP_EN      0u   /*中断号29*/
#define OS_ISR30_USE_MSP_EN      0u   /*中断号30*/
#define OS_ISR31_USE_MSP_EN      0u   /*中断号31*/
#define OS_ISR32_USE_MSP_EN      0u   /*中断号32*/
#define OS_ISR33_USE_MSP_EN      0u   /*中断号33*/
#define OS_ISR34_USE_MSP_EN      0u   /*中断号34*/
#define OS_ISR35_USE_MSP_EN      0u   /*中断号35*/
#define OS_ISR36_USE_MSP_EN      0u   /*中断号36*/
#define OS_ISR37_USE_MSP_EN      0u   /*中断号37*/
#define OS_ISR38_USE_MSP_EN      0u   /*中断号38*/
#define OS_ISR39_USE_MSP_EN      0u   /*中断号39*/
#define OS_ISR40_USE_MSP_EN      0u   /*中断号40*/
#define OS_ISR41_USE_MSP_EN      0u   /*中断号41*/
#define OS_ISR42_USE_MSP_EN      0u   /*中断号42*/
#define OS_ISR43_USE_MSP_EN      0u   /*中断号43*/
#define OS_ISR44_USE_MSP_EN      0u   /*中断号44*/
#define OS_ISR45_USE_MSP_EN      0u   /*中断号45*/
#define OS_ISR46_USE_MSP_EN      0u   /*中断号46*/
#define OS_ISR47_USE_MSP_EN      0u   /*中断号47*/
#define OS_ISR48_USE_MSP_EN      0u   /*中断号48*/
#define OS_ISR49_USE_MSP_EN      0u   /*中断号49*/
#define OS_ISR50_USE_MSP_EN      0u   /*中断号50*/
#define OS_ISR51_USE_MSP_EN      0u   /*中断号51*/
#define OS_ISR52_USE_MSP_EN      0u   /*中断号52*/
#define OS_ISR53_USE_MSP_EN      0u   /*中断号53*/
#define OS_ISR54_USE_MSP_EN      0u   /*中断号54*/
#define OS_ISR55_USE_MSP_EN      0u   /*中断号55*/
#define OS_ISR56_USE_MSP_EN      0u   /*中断号56*/
#define OS_ISR57_USE_MSP_EN      0u   /*中断号57*/
#define OS_ISR58_USE_MSP_EN      0u   /*中断号58*/
#define OS_ISR59_USE_MSP_EN      0u   /*中断号59*/
#define OS_ISR60_USE_MSP_EN      0u   /*中断号60*/
#define OS_ISR61_USE_MSP_EN      0u   /*中断号61*/
#define OS_ISR62_USE_MSP_EN      0u   /*中断号62*/
#define OS_ISR63_USE_MSP_EN      0u   /*中断号63*/
#define OS_ISR64_USE_MSP_EN      0u   /*中断号64*/



//OS中断勾子函数声明 (中断号排序)
void INT0_ISR_Handler_Hook (void);      // 0        
void Timer0_ISR_Handler_Hook(void);     // 1
void INT1_ISR_Handler_Hook (void);      // 2        
void Timer1_ISR_Handler_Hook (void);    // 3        
void UART1_ISR_Handler_Hook (void);     // 4
void ADC_ISR_Handler_Hook (void);       // 5
void LVD_ISR_Handler_Hook (void);       // 6

void UART2_ISR_Handler_Hook (void);     // 8
void SPI_ISR_Handler_Hook (void);       // 9
void INT2_ISR_Handler_Hook (void);      // 10        
void INT3_ISR_Handler_Hook (void);      // 11        
void Timer2_ISR_Handler_Hook (void);    // 12
void DMA_ISR_Handler_Hook (void);       // 13

void INT4_ISR_Handler_Hook (void);      // 16        
void UART3_ISR_Handler_Hook (void);     // 17
void UART4_ISR_Handler_Hook (void);     // 18
void Timer3_ISR_Handler_Hook (void);    // 19        
void Timer4_ISR_Handler_Hook (void);    // 20        
void CMP_ISR_Handler_Hook (void);       // 21

void I2C_ISR_Handler_Hook (void);       // 24
void USB_ISR_Handler_Hook (void);       // 25
void PWMA_ISR_Handler_Hook (void);      // 26
void PWMB_ISR_Handler_Hook (void);      // 27
void CAN1_ISR_Handler_Hook (void);      // 28
void CAN2_ISR_Handler_Hook (void);      // 29
void LIN_ISR_Handler_Hook (void);       // 30

void RTC_ISR_Handler_Hook (void);       // 36
void P0_ISR_Handler_Hook (void);        // 37
void P1_ISR_Handler_Hook (void);        // 38
void P2_ISR_Handler_Hook (void);        // 39
void P3_ISR_Handler_Hook (void);        // 40
void P4_ISR_Handler_Hook (void);        // 41
void P5_ISR_Handler_Hook (void);        // 42
void P6_ISR_Handler_Hook (void);        // 43
void P7_ISR_Handler_Hook (void);        // 44

void DMA_M2M_ISR_Handler_Hook (void);       // 47
void DMA_ADC_ISR_Handler_Hook (void);       // 48
void DMA_SPI_ISR_Handler_Hook (void);       // 49
void DMA_UART1TX_ISR_Handler_Hook (void);   // 50
void DMA_UART1RX_ISR_Handler_Hook (void);   // 51
void DMA_UART2TX_ISR_Handler_Hook (void);   // 52
void DMA_UART2RX_ISR_Handler_Hook (void);   // 53
void DMA_UART3TX_ISR_Handler_Hook (void);   // 54
void DMA_UART3RX_ISR_Handler_Hook (void);   // 55
void DMA_UART4TX_ISR_Handler_Hook (void);   // 56
void DMA_UART4RX_ISR_Handler_Hook (void);   // 57
void DMA_LCM_ISR_Handler_Hook (void);       // 58
void LCM_ISR_Handler_Hook (void);           // 59
void DMA_I2CT_ISR_Handler_Hook (void);      // 60
void DMA_I2CR_ISR_Handler_Hook (void);      // 61
void I2S_I2CR_ISR_Handler_Hook (void);      // 62
void DMS_I2STX_ISR_Handler_Hook (void);     // 63
void DMA_I2SRX_ISR_Handler_Hook (void);     // 64




//中断号宏调用,映射对应的勾子函数, 中断号映射硬件没有变化, 就不需要修改
#define ISR_Handler_Hook0()     INT0_ISR_Handler_Hook ()      // 0        
#define ISR_Handler_Hook1()     Timer0_ISR_Handler_Hook()     // 1
#define ISR_Handler_Hook2()     INT1_ISR_Handler_Hook ()      // 2        
#define ISR_Handler_Hook3()     Timer1_ISR_Handler_Hook ()    // 3        
#define ISR_Handler_Hook4()     UART1_ISR_Handler_Hook ()     // 4
#define ISR_Handler_Hook5()     ADC_ISR_Handler_Hook ()       // 5
#define ISR_Handler_Hook6()     LVD_ISR_Handler_Hook ()       // 6

#define ISR_Handler_Hook7()     

#define ISR_Handler_Hook8()     UART2_ISR_Handler_Hook ()     // 8
#define ISR_Handler_Hook9()     SPI_ISR_Handler_Hook ()       // 9
#define ISR_Handler_Hook10()    INT2_ISR_Handler_Hook ()      // 10        
#define ISR_Handler_Hook11()    INT3_ISR_Handler_Hook ()      // 11        
#define ISR_Handler_Hook12()    Timer2_ISR_Handler_Hook ()    // 12

#define ISR_Handler_Hook13()    
#define ISR_Handler_Hook14()    
#define ISR_Handler_Hook15()    

#define ISR_Handler_Hook16()    INT4_ISR_Handler_Hook ()      // 16        
#define ISR_Handler_Hook17()    UART3_ISR_Handler_Hook ()     // 17
#define ISR_Handler_Hook18()    UART4_ISR_Handler_Hook ()     // 18
#define ISR_Handler_Hook19()    Timer3_ISR_Handler_Hook ()    // 19        
#define ISR_Handler_Hook20()    Timer4_ISR_Handler_Hook ()    // 20        
#define ISR_Handler_Hook21()    CMP_ISR_Handler_Hook ()       // 21

#define ISR_Handler_Hook22()    
#define ISR_Handler_Hook23()    

#define ISR_Handler_Hook24()    I2C_ISR_Handler_Hook ()       // 24
#define ISR_Handler_Hook25()    USB_ISR_Handler_Hook ()       // 25
#define ISR_Handler_Hook26()    PWMA_ISR_Handler_Hook ()      // 26
#define ISR_Handler_Hook27()    PWMB_ISR_Handler_Hook ()      // 27
#define ISR_Handler_Hook28()    CAN1_ISR_Handler_Hook ()      // 28
#define ISR_Handler_Hook29()    CAN2_ISR_Handler_Hook ()      // 29
#define ISR_Handler_Hook30()    LIN_ISR_Handler_Hook ()       // 30

#define ISR_Handler_Hook31()    
#define ISR_Handler_Hook32()    
#define ISR_Handler_Hook33()    
#define ISR_Handler_Hook34()    
#define ISR_Handler_Hook35()    

#define ISR_Handler_Hook36()    RTC_ISR_Handler_Hook ()       // 36
#define ISR_Handler_Hook37()    P0_ISR_Handler_Hook ()        // 37
#define ISR_Handler_Hook38()    P1_ISR_Handler_Hook ()        // 38
#define ISR_Handler_Hook39()    P2_ISR_Handler_Hook ()        // 39
#define ISR_Handler_Hook40()    P3_ISR_Handler_Hook ()        // 40
#define ISR_Handler_Hook41()    P4_ISR_Handler_Hook ()        // 41
#define ISR_Handler_Hook42()    P5_ISR_Handler_Hook ()        // 42
#define ISR_Handler_Hook43()    P6_ISR_Handler_Hook ()        // 43
#define ISR_Handler_Hook44()    P7_ISR_Handler_Hook ()        // 44

#define ISR_Handler_Hook45()    
#define ISR_Handler_Hook46()    

#define ISR_Handler_Hook47()    DMA_M2M_ISR_Handler_Hook ()       // 47
#define ISR_Handler_Hook48()    DMA_ADC_ISR_Handler_Hook ()       // 48
#define ISR_Handler_Hook49()    DMA_SPI_ISR_Handler_Hook ()       // 49
#define ISR_Handler_Hook50()    DMA_UART1TX_ISR_Handler_Hook ()   // 50
#define ISR_Handler_Hook51()    DMA_UART1RX_ISR_Handler_Hook ()   // 51
#define ISR_Handler_Hook52()    DMA_UART2TX_ISR_Handler_Hook ()   // 52
#define ISR_Handler_Hook53()    DMA_UART2RX_ISR_Handler_Hook ()   // 53
#define ISR_Handler_Hook54()    DMA_UART3TX_ISR_Handler_Hook ()   // 54
#define ISR_Handler_Hook55()    DMA_UART3RX_ISR_Handler_Hook ()   // 55
#define ISR_Handler_Hook56()    DMA_UART4TX_ISR_Handler_Hook ()   // 56
#define ISR_Handler_Hook57()    DMA_UART4RX_ISR_Handler_Hook ()   // 57
#define ISR_Handler_Hook58()    DMA_LCM_ISR_Handler_Hook ()       // 58
#define ISR_Handler_Hook59()    LCM_ISR_Handler_Hook ()           // 59
#define ISR_Handler_Hook60()    DMA_I2CT_ISR_Handler_Hook ()      // 60
#define ISR_Handler_Hook61()    DMA_I2CR_ISR_Handler_Hook ()      // 61
#define ISR_Handler_Hook62()    I2S_I2CR_ISR_Handler_Hook ()      // 62
#define ISR_Handler_Hook63()    DMS_I2STX_ISR_Handler_Hook()      // 63
#define ISR_Handler_Hook64()    DMA_I2SRX_ISR_Handler_Hook()      // 64



#endif
