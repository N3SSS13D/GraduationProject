/*-----------------------------------------------------------*
os_cpu_a.c
*-----------------------------------------------------------*/

#include "config.h"


/*-----------------------------------------------------------*
变量声明
*-----------------------------------------------------------*/
#pragma asm
EXTRN       EDATA (OSTCBCur)
EXTRN       EDATA (OSTCBHighRdy)
EXTRN       DATA : BYTE (OSPrioCur)
EXTRN       DATA : BYTE (OSPrioHighRdy)
?STACK      SEGMENT   EDATA
#pragma endasm


#if OS_CRITICAL_METHOD == 2
INT8U data _uxNesting_ = 0;
bit _bEA_ = 0;
#endif

OsMStk_t xdata MStkState _at_ 0;

/*-----------------------------------------------------------*
PendSv中断初始化
使用Timer4, 如改其它中断号需替换初始化代码
*-----------------------------------------------------------*/
void prvPortPendSvInit(void)
{
    //设置PendSv为最低优先级
    //其它相关代码
    T4IF=0;     //清除中断标志
    ET4=1;      //使能中断
}

/*-----------------------------------------------------------*
滴答时钟初始化
*-----------------------------------------------------------*/
#define TIM0_DIV  ( MAIN_Fosc/OS_TICKS_PER_SEC/65536UL+1UL )                        //分频
#define TIM0_LOAD ( 65536UL - (MAIN_Fosc*2UL/TIM0_DIV/OS_TICKS_PER_SEC+1UL)/2UL )   //装载值
static void SetupTimerInterrupt( void )
{
    AUXR    |=  0x80;           //1T模式
    TMOD    &=  0XF0;           //模式0
    TM0PS   =   TIM0_DIV-1;     //分频系数
    TL0     =   TIM0_LOAD;      //装载值
    TH0     =   TIM0_LOAD>>8;   //装载值
    TF0     =   0;              //清除标志
    ET0     =   1;              //使能中断
    TR0     =   1;              //定时器开启
}


#if OS_USE_XDATA
#define WORD2_OSTCBCur  1
#else
#define WORD2_OSTCBCur  0
#endif

/*-----------------------------------------------------------*
第一次切换到任务里去
*-----------------------------------------------------------*/
void OSStartHighRdy(void)
{
    SetupTimerInterrupt();
    prvPortPendSvInit();
    
    EA=0;
    OSRunning = 1;
    
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
    OSTaskSwHook();
#endif
    
    
#pragma asm
    MOV WR6,    OSTCBCur
    MOV WR4,    #WORD2_OSTCBCur
    MOV WR2,    @DR4
    MOV DR60,   DR0
#pragma endasm
    
    portRESTORE_CONTEXT_SW();
    
#pragma asm
    POP   R4
    POP   R6
    POP   R5
    POP   PSW1
    PUSH  R5
    PUSH  R4
    PUSH  R6
    SETB  EA
    DB    0AAH
#pragma endasm
    
}

/*-----------------------------------------------------------*
PendSv 中断做任务切换
*-----------------------------------------------------------*/
void PendSvIsr( void )
{
__asm{ PendSvIsr_Entrance:  }

    portSAVE_CONTEXT_SW();
    
#pragma asm
    MOV DR0,    DR60
    MOV WR6,    OSTCBCur
    MOV WR4,    #WORD2_OSTCBCur
    MOV @DR4,   WR2
#pragma endasm
    
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
    OSTaskSwHook();
#endif
    
    PendSv_ClearFlag();
    
#pragma asm
    MOV OSPrioCur,OSPrioHighRdy
    MOV WR6,OSTCBHighRdy
    MOV OSTCBCur,WR6

;    MOV WR6,    OSTCBCur
    MOV WR4,    #WORD2_OSTCBCur
    MOV WR2,    @DR4
    MOV DR60,   DR0
#pragma endasm
    
    portRESTORE_CONTEXT_SW();
    
    __asm{  SETB  EA    }
    __asm{  RETI        }
}

/*-----------------------------------------------------------*
时钟滴答中断
*-----------------------------------------------------------*/
void Timer0_ISR_Handler_Hook(void)
{
    OSTimeTick();
}

/*-----------------------------------------------------------*
中断栈(MSTK)检查,结果在全局变量中
请使用START251.A51, 并正确设置第109行
START251.A51针对OS移植有增设语句,请使用本项目中的START251.A51范例,
*-----------------------------------------------------------*/
void OsMStkStkChk(void)
{
    INT16U cnt, Size=MStkState.Size;
    OS_STK * pt = (OS_STK *)MStkState.Start+Size-1;
    if((Size==0)||(MStkState.Start==0))return;  //检测到未正确设置 START251.A51 
    for(cnt=0;cnt<Size;cnt++){
        if(0!=*pt--)break;
    }
    OS_ENTER_CRITICAL();
    MStkState.Used=Size-cnt;
    MStkState.Free=cnt;
    OS_EXIT_CRITICAL();
}


/*切换到MSP */
#define portSW_TO_MSP()                                     \
{                                                           \
    __asm{  MOV     DR0,    DR60                }           \
    __asm{  MOV     DR60,   #WORD0 (?STACK-1)   }           \
    __asm{  PUSH    DR0                         }           \
}


/*切换到PSP */
#define portSW_TO_PSP()                                     \
{                                                           \
/*    __asm{  POP     DR0                 }*/                   \
/*    __asm{  XRL     WR0,    WR0         }*/                   \
/*    __asm{  MOV     DR60,   DR0         }*/                   \
    __asm{  POP     DR60                }                   \
}


/*中断函数宏模板*/
#define _ISR_Package_CODE(n)                                \
void ISR_Handler_##n (void)                                 \
{                                                           \
    __asm{ISR_Handler##n:}                                  \
    portSAVE_CONTEXT();                                     \
    if(OSIntNesting==0){ portSW_TO_MSP() }                  \
    OSIntNesting++;                                         \
    __asm   { SETB  EA      }                               \
    ISR_Handler_Hook##n();                                  \
    __asm   { CLR  EA      }                                \
    OSIntExit();                                            \
    if(OSIntNesting==0){ portSW_TO_PSP() }                  \
    portRESTORE_CONTEXT();                                  \
    __asm   { SETB  EA      }                               \
    __asm   { RETI          }                               \
}


/*-----------------------------------------------------------*
中断封装函数
*-----------------------------------------------------------*/
#if OS_ISR0_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 0u
    _ISR_Package_CODE(0)
#endif
#if OS_ISR1_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 1u
    _ISR_Package_CODE(1)
#endif
#if OS_ISR2_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 2u
    _ISR_Package_CODE(2)
#endif  
#if OS_ISR3_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 3u
    _ISR_Package_CODE(3)
#endif  
#if OS_ISR4_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 4u
    _ISR_Package_CODE(4)
#endif  
#if OS_ISR5_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 5u
    _ISR_Package_CODE(5)
#endif  
#if OS_ISR6_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 6u
    _ISR_Package_CODE(6)
#endif 
#if OS_ISR7_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 7u
    _ISR_Package_CODE(7)
#endif 
#if OS_ISR8_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 8u
    _ISR_Package_CODE(8)
#endif 
#if OS_ISR9_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 9u
    _ISR_Package_CODE(9)
#endif
#if OS_ISR10_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 10u
    _ISR_Package_CODE(10)
#endif 
#if OS_ISR11_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 11u
    _ISR_Package_CODE(11)
#endif 
#if OS_ISR12_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 12u
    _ISR_Package_CODE(12)
#endif 
#if OS_ISR13_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 13u
    _ISR_Package_CODE(13)
#endif  
#if OS_ISR14_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 14u
    _ISR_Package_CODE(14)
#endif 
#if OS_ISR15_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 15u
    _ISR_Package_CODE(15)
#endif 
#if OS_ISR16_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 16u
    _ISR_Package_CODE(16)
#endif 
#if OS_ISR17_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 17u
    _ISR_Package_CODE(17)
#endif  
#if OS_ISR18_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 18u
    _ISR_Package_CODE(18)
#endif 
#if OS_ISR19_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 19u
    _ISR_Package_CODE(19)
#endif
#if OS_ISR20_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 20u
    _ISR_Package_CODE(20)
#endif 
#if OS_ISR21_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 21u
    _ISR_Package_CODE(21)
#endif 
#if OS_ISR22_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 22u
    _ISR_Package_CODE(22)
#endif
#if OS_ISR23_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 23u
    _ISR_Package_CODE(23)
#endif
#if OS_ISR24_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 24u
    _ISR_Package_CODE(24)
#endif 
#if OS_ISR25_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 25u
    _ISR_Package_CODE(25)
#endif
#if OS_ISR26_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 26u
    _ISR_Package_CODE(26)
#endif 
#if OS_ISR27_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 27u
    _ISR_Package_CODE(27)
#endif 
#if OS_ISR28_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 28u
    _ISR_Package_CODE(28)
#endif  
#if OS_ISR29_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 29u
    _ISR_Package_CODE(29)
#endif
#if OS_ISR30_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 30u
    _ISR_Package_CODE(30)
#endif 
#if OS_ISR31_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 31u
    _ISR_Package_CODE(31)
#endif 
#if OS_ISR32_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 32u
    _ISR_Package_CODE(32)
#endif
#if OS_ISR33_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 33u
    _ISR_Package_CODE(33)
#endif 
#if OS_ISR34_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 34u
    _ISR_Package_CODE(34)
#endif 
#if OS_ISR35_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 35u
    _ISR_Package_CODE(35)
#endif
#if OS_ISR36_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 36u
    _ISR_Package_CODE(36)
#endif 
#if OS_ISR37_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 37u
    _ISR_Package_CODE(37)
#endif
#if OS_ISR38_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 38u
    _ISR_Package_CODE(38)
#endif 
#if OS_ISR39_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 39u
    _ISR_Package_CODE(39)
#endif 
#if OS_ISR40_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 40u
    _ISR_Package_CODE(40)
#endif  
#if OS_ISR41_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 41u
    _ISR_Package_CODE(41)
#endif  
#if OS_ISR42_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 42u
    _ISR_Package_CODE(42)
#endif 
#if OS_ISR43_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 43u
    _ISR_Package_CODE(43)
#endif
#if OS_ISR44_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 44u
    _ISR_Package_CODE(44)
#endif
#if OS_ISR45_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 45u
    _ISR_Package_CODE(45)
#endif 
#if OS_ISR46_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 46u
    _ISR_Package_CODE(46)
#endif
#if OS_ISR47_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 47u
    _ISR_Package_CODE(47)
#endif 
#if OS_ISR48_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 48u
    _ISR_Package_CODE(48)
#endif
#if OS_ISR49_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 49u
    _ISR_Package_CODE(49)
#endif 
#if OS_ISR50_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 50u
    _ISR_Package_CODE(50)
#endif
#if OS_ISR51_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 51u
    _ISR_Package_CODE(51)
#endif 
#if OS_ISR52_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 52u
    _ISR_Package_CODE(52)
#endif
#if OS_ISR53_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 53u
    _ISR_Package_CODE(53)
#endif
#if OS_ISR54_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 54u
    _ISR_Package_CODE(54)
#endif 
#if OS_ISR55_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 55u
    _ISR_Package_CODE(55)
#endif 
#if OS_ISR56_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 56u
    _ISR_Package_CODE(56)
#endif
#if OS_ISR57_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 57u
    _ISR_Package_CODE(57)
#endif 
#if OS_ISR58_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 58u
    _ISR_Package_CODE(58)
#endif 
#if OS_ISR59_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 59u
    _ISR_Package_CODE(59)
#endif 
#if OS_ISR60_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 60u
    _ISR_Package_CODE(60)
#endif
#if OS_ISR61_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 61u
    _ISR_Package_CODE(61)
#endif 
#if OS_ISR62_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 62u
    _ISR_Package_CODE(62)
#endif 
#if OS_ISR63_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 63u
    _ISR_Package_CODE(63)
#endif 
#if OS_ISR64_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 64u
    _ISR_Package_CODE(64)
#endif



/*-----------------------------------------------------------*
PendSv  向量入口引导
*-----------------------------------------------------------*/
__asm{ 
    CSEG    AT  PendSv_InterruptNumber*8+3
    CLR     EA
    JMP     PendSvIsr_Entrance
}

/*-----------------------------------------------------------*
中断入口
*-----------------------------------------------------------*/
#if OS_ISR0_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 0u
__asm{ 
    CSEG    AT  0003H           /**/
    CLR     EA
    JMP     ISR_Handler0        /**/
}
#endif  
#if OS_ISR1_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 1u
__asm{ 
    CSEG    AT  000BH           /**/
    CLR     EA
    JMP     ISR_Handler1        /**/
}
#endif  
#if OS_ISR2_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 2u
__asm{ 
    CSEG    AT  0013H           /**/
    CLR     EA
    JMP     ISR_Handler2        /**/
}
#endif  
#if OS_ISR3_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 3u
__asm{ 
    CSEG    AT  001BH           /**/
    CLR     EA
    JMP     ISR_Handler3        /**/
}
#endif  
#if OS_ISR4_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 4u
__asm{ 
    CSEG    AT  0023H           /**/
    CLR     EA
    JMP     ISR_Handler4        /**/
}
#endif  
#if OS_ISR5_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 5u
__asm{ 
    CSEG    AT  002BH           /**/
    CLR     EA
    JMP     ISR_Handler5        /**/
}
#endif  
#if OS_ISR6_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 6u
__asm{ 
    CSEG    AT  0033H           /**/
    CLR     EA
    JMP     ISR_Handler6        /**/
}
#endif  
#if OS_ISR7_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 7u
__asm{ 
    CSEG    AT  003BH           /**/
    CLR     EA
    JMP     ISR_Handler7        /**/
}
#endif  
#if OS_ISR8_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 8u
__asm{ 
    CSEG    AT  0043H           /**/
    CLR     EA
    JMP     ISR_Handler8        /**/
}
#endif  
#if OS_ISR9_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 9u
__asm{ 
    CSEG    AT  004BH           /**/
    CLR     EA
    JMP     ISR_Handler9        /**/
}
#endif  
#if OS_ISR10_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 10u
__asm{ 
    CSEG    AT  0053H           /**/
    CLR     EA
    JMP     ISR_Handler10        /**/
}
#endif  
#if OS_ISR11_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 11u
__asm{ 
    CSEG    AT  005BH           /**/
    CLR     EA
    JMP     ISR_Handler11        /**/
}
#endif  
#if OS_ISR12_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 12u
__asm{ 
    CSEG    AT  0063H           /**/
    CLR     EA
    JMP     ISR_Handler12        /**/
}
#endif  
#if OS_ISR13_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 13u
__asm{ 
    CSEG    AT  006BH           /**/
    CLR     EA
    JMP     ISR_Handler13        /**/
}
#endif  
#if OS_ISR14_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 14u
__asm{ 
    CSEG    AT  0073H           /**/
    CLR     EA
    JMP     ISR_Handler14        /**/
}
#endif  
#if OS_ISR15_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 15u
__asm{ 
    CSEG    AT  007BH           /**/
    CLR     EA
    JMP     ISR_Handler15        /**/
}
#endif  
#if OS_ISR16_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 16u
__asm{ 
    CSEG    AT  0083H           /**/
    CLR     EA
    JMP     ISR_Handler16        /**/
}
#endif  
#if OS_ISR17_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 17u
__asm{ 
    CSEG    AT  008BH           /**/
    CLR     EA
    JMP     ISR_Handler17        /**/
}
#endif  
#if OS_ISR18_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 18u
__asm{ 
    CSEG    AT  0093H           /**/
    CLR     EA
    JMP     ISR_Handler18        /**/
}
#endif  
#if OS_ISR19_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 19u  
__asm{ 
    CSEG    AT  009BH           /**/
    CLR     EA
    JMP     ISR_Handler19        /**/
}
#endif  
#if OS_ISR20_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 20u
__asm{ 
    CSEG    AT  00A3H           /**/
    CLR     EA
    JMP     ISR_Handler20        /**/
}
#endif  
#if OS_ISR21_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 21u
__asm{ 
    CSEG    AT  00ABH           /**/
    CLR     EA
    JMP     ISR_Handler21        /**/
}
#endif  
#if OS_ISR22_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 22u
__asm{ 
    CSEG    AT  00B3H           /**/
    CLR     EA
    JMP     ISR_Handler22        /**/
}
#endif  
#if OS_ISR23_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 23u
__asm{ 
    CSEG    AT  00BBH           /**/
    CLR     EA
    JMP     ISR_Handler23        /**/
}
#endif  
#if OS_ISR24_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 24u
__asm{ 
    CSEG    AT  00C3H           /**/
    CLR     EA
    JMP     ISR_Handler24        /**/
}
#endif  
#if OS_ISR25_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 25u
__asm{ 
    CSEG    AT  00CBH           /**/
    CLR     EA
    JMP     ISR_Handler25        /**/
}
#endif  
#if OS_ISR26_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 26u
__asm{ 
    CSEG    AT  00D3H           /**/
    CLR     EA
    JMP     ISR_Handler26        /**/
}
#endif  
#if OS_ISR27_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 27u
__asm{ 
    CSEG    AT  00DBH           /**/
    CLR     EA
    JMP     ISR_Handler27        /**/
}
#endif  
#if OS_ISR28_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 28u
__asm{ 
    CSEG    AT  00E3H           /**/
    CLR     EA
    JMP     ISR_Handler28        /**/
}
#endif  
#if OS_ISR29_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 29u
__asm{ 
    CSEG    AT  00EBH           /**/
    CLR     EA
    JMP     ISR_Handler29        /**/
}
#endif  
#if OS_ISR30_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 30u
__asm{ 
    CSEG    AT  00F3H           /**/
    CLR     EA
    JMP     ISR_Handler30        /**/
}
#endif  
#if OS_ISR31_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 31u
__asm{ 
    CSEG    AT  00FBH           /**/
    CLR     EA
    JMP     ISR_Handler31        /**/
}
#endif  
#if OS_ISR32_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 32u
__asm{ 
    CSEG    AT  0103H           /**/
    CLR     EA
    JMP     ISR_Handler32        /**/
}
#endif  
#if OS_ISR33_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 33u
__asm{ 
    CSEG    AT  010BH           /**/
    CLR     EA
    JMP     ISR_Handler33        /**/
}
#endif  
#if OS_ISR34_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 34u
__asm{ 
    CSEG    AT  0113H           /**/
    CLR     EA
    JMP     ISR_Handler34        /**/
}
#endif  
#if OS_ISR35_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 35u
__asm{ 
    CSEG    AT  011BH           /**/
    CLR     EA
    JMP     ISR_Handler35        /**/
}
#endif  
#if OS_ISR36_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 36u
__asm{ 
    CSEG    AT  0123H           /**/
    CLR     EA
    JMP     ISR_Handler36        /**/
}
#endif  
#if OS_ISR37_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 37u
__asm{ 
    CSEG    AT  012BH           /**/
    CLR     EA
    JMP     ISR_Handler37        /**/
}
#endif  
#if OS_ISR38_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 38u
__asm{ 
    CSEG    AT  0133H           /**/
    CLR     EA
    JMP     ISR_Handler38        /**/
}
#endif  
#if OS_ISR39_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 39u
__asm{ 
    CSEG    AT  013BH           /**/
    CLR     EA
    JMP     ISR_Handler39        /**/
}
#endif  
#if OS_ISR40_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 40u
__asm{ 
    CSEG    AT  0143H           /**/
    CLR     EA
    JMP     ISR_Handler40        /**/
}
#endif  
#if OS_ISR41_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 41u
__asm{ 
    CSEG    AT  014BH           /**/
    CLR     EA
    JMP     ISR_Handler41        /**/
}
#endif  
#if OS_ISR42_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 42u
__asm{ 
    CSEG    AT  0153H           /**/
    CLR     EA
    JMP     ISR_Handler42        /**/
}
#endif  
#if OS_ISR43_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 43u
__asm{ 
    CSEG    AT  015BH           /**/
    CLR     EA
    JMP     ISR_Handler43        /**/
}
#endif  
#if OS_ISR44_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 44u
__asm{ 
    CSEG    AT  0163H           /**/
    CLR     EA
    JMP     ISR_Handler44        /**/
}
#endif  
#if OS_ISR45_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 45u
__asm{ 
    CSEG    AT  016BH           /**/
    CLR     EA
    JMP     ISR_Handler45        /**/
}
#endif  
#if OS_ISR46_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 46u
__asm{ 
    CSEG    AT  0173H           /**/
    CLR     EA
    JMP     ISR_Handler46        /**/
}
#endif  
#if OS_ISR47_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 47u
__asm{ 
    CSEG    AT  017BH           /**/
    CLR     EA
    JMP     ISR_Handler47        /**/
}
#endif  
#if OS_ISR48_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 48u
__asm{ 
    CSEG    AT  0183H           /**/
    CLR     EA
    JMP     ISR_Handler48        /**/
}
#endif  
#if OS_ISR49_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 49u
__asm{ 
    CSEG    AT  018BH           /**/
    CLR     EA
    JMP     ISR_Handler49        /**/
}
#endif  
#if OS_ISR50_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 50u
__asm{ 
    CSEG    AT  0193H           /**/
    CLR     EA
    JMP     ISR_Handler50        /**/
}
#endif  
#if OS_ISR51_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 51u
__asm{ 
    CSEG    AT  019BH           /**/
    CLR     EA
    JMP     ISR_Handler51        /**/
}
#endif  
#if OS_ISR52_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 52u
__asm{ 
    CSEG    AT  01A3H           /**/
    CLR     EA
    JMP     ISR_Handler52        /**/
}
#endif  
#if OS_ISR53_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 53u
__asm{ 
    CSEG    AT  01ABH           /**/
    CLR     EA
    JMP     ISR_Handler53        /**/
}
#endif  
#if OS_ISR54_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 54u
__asm{ 
    CSEG    AT  01B3H           /**/
    CLR     EA
    JMP     ISR_Handler54        /**/
}
#endif  
#if OS_ISR55_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 55u
__asm{ 
    CSEG    AT  01BBH           /**/
    CLR     EA
    JMP     ISR_Handler55        /**/
}
#endif  
#if OS_ISR56_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 56u
__asm{ 
    CSEG    AT  01C3H           /**/
    CLR     EA
    JMP     ISR_Handler56        /**/
}
#endif  
#if OS_ISR57_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 57u
__asm{ 
    CSEG    AT  01CBH           /**/
    CLR     EA
    JMP     ISR_Handler57        /**/
}
#endif  
#if OS_ISR58_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 58u
__asm{ 
    CSEG    AT  01D3H           /**/
    CLR     EA
    JMP     ISR_Handler58        /**/
}
#endif  
#if OS_ISR59_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 59u
__asm{ 
    CSEG    AT  01DBH           /**/
    CLR     EA
    JMP     ISR_Handler59        /**/
}
#endif  
#if OS_ISR60_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 60u
__asm{ 
    CSEG    AT  01E3H           /**/
    CLR     EA
    JMP     ISR_Handler60        /**/
}
#endif  
#if OS_ISR61_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 61u
__asm{ 
    CSEG    AT  01EBH           /**/
    CLR     EA
    JMP     ISR_Handler61        /**/
}
#endif  
#if OS_ISR62_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 62u
__asm{ 
    CSEG    AT  01F3H           /**/
    CLR     EA
    JMP     ISR_Handler62        /**/
}
#endif  
#if OS_ISR63_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 63u
__asm{ 
    CSEG    AT  01FBH           /**/
    CLR     EA
    JMP     ISR_Handler63        /**/
}
#endif
#if OS_ISR64_USE_MSP_EN > 0u  &&  PendSv_InterruptNumber != 64u
__asm{ 
    CSEG    AT  0203H           /**/
    CLR     EA
    JMP     ISR_Handler64       /**/
}
#endif






