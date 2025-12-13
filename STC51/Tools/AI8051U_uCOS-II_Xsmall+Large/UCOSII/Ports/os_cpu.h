/*---------------------------------------------------------------------
os_cpu.h
---------------------------------------------------------------------*/


#ifndef    __OS_CPU_H__
#define    __OS_CPU_H__

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

#include    "config.h"



/******************************************************************************
*                    定义与编译器无关的数据类型
******************************************************************************/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;       /* Unsigned  8 bit quantity       */
typedef signed   char  INT8S;       /* Signed    8 bit quantity       */
typedef unsigned int   INT16U;      /* Unsigned 16 bit quantity       */
typedef signed   int   INT16S;      /* Signed   16 bit quantity       */
typedef unsigned long  INT32U;      /* Unsigned 32 bit quantity       */
typedef signed   long  INT32S;      /* Signed   32 bit quantity       */
typedef float          FP32;        /* Single precision floating point */
typedef double         FP64;        /* Double precision floating point */

typedef INT8U edata    OS_STK;      /* Each stack entry is 32-bit wide */
typedef unsigned char  OS_CPU_SR;   /* Define size of CPU status register */


#define  OS_STK_GROWTH          0   //定义栈的增长方向
#define  OS_CRITICAL_METHOD     2   //进入临界段的方法, 可选2或3, 不支持1


#if OS_CRITICAL_METHOD == 2
extern INT8U data _uxNesting_;
extern bit _bEA_;
#define  OS_ENTER_CRITICAL()    do{ if(!_testbit_(EA)) { if(_uxNesting_==0)_bEA_=0;} else { if(_uxNesting_ == 0)_bEA_=1;}_uxNesting_++; }while(0)
#define  OS_EXIT_CRITICAL()     do{ _uxNesting_--; if(_uxNesting_==0)EA=_bEA_; }while(0)
#endif


#if OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()    do{cpu_sr=((!_testbit_(EA))?0X00:0X80);}while(0)
#define  OS_EXIT_CRITICAL()     do{IE|=cpu_sr;}while(0)
#endif


#define  OS_TASK_SW()   do{PendSv_SetFlag();OS_EXIT_CRITICAL();while(PendSv_GetFlag());return;}while(0)
#define  OSIntCtxSw()   do{PendSv_SetFlag();}while(0)

typedef struct {
    INT16U  Start;
    INT16U  Size;
    INT16U  Used;
    INT16U  Free;
}OsMStk_t;

void OSStartHighRdy(void);
void OsMStkStkChk(void);
extern OsMStk_t xdata MStkState;



/*切换任务入栈*/
#define portSAVE_CONTEXT_SW()               \
{                                           \
    __asm   { PUSH  DR28    }               \
    __asm   { PUSH  DR24    }               \
    __asm   { PUSH  DR20    }               \
    __asm   { PUSH  DR16    }               \
    __asm   { PUSH  DR12    }               \
    __asm   { PUSH  DR8     }               \
    __asm   { PUSH  DR4     }               \
    __asm   { PUSH  DR0     }               \
    __asm   { PUSH  PSW1    }               \
    __asm   { PUSH  PSW     }               \
    __asm   { PUSH  DPH     }               \
    __asm   { PUSH  DPL     }               \
}


/*切换任务出栈*/
#define portRESTORE_CONTEXT_SW()            \
{                                           \
    __asm   { POP   DPL     }               \
    __asm   { POP   DPH     }               \
    __asm   { POP   PSW     }               \
    __asm   { POP   PSW1    }               \
    __asm   { POP   DR0     }               \
    __asm   { POP   DR4     }               \
    __asm   { POP   DR8     }               \
    __asm   { POP   DR12    }               \
    __asm   { POP   DR16    }               \
    __asm   { POP   DR20    }               \
    __asm   { POP   DR24    }               \
    __asm   { POP   DR28    }               \
}


/*中断入栈*/
#define portSAVE_CONTEXT()                  \
{                                           \
    __asm   { PUSH  DR28    }               \
    __asm   { PUSH  DR24    }               \
    __asm   { PUSH  DR20    }               \
    __asm   { PUSH  DR16    }               \
    __asm   { PUSH  DR8     }               \
    __asm   { PUSH  DR4     }               \
    __asm   { PUSH  DR0     }               \
    __asm   { PUSH  PSW1    }               \
    __asm   { PUSH  PSW     }               \
    __asm   { PUSH  DPH     }               \
    __asm   { PUSH  DPL     }               \
}


/*中断出栈*/
#define portRESTORE_CONTEXT()               \
{                                           \
    __asm   { POP   DPL     }               \
    __asm   { POP   DPH     }               \
    __asm   { POP   PSW     }               \
    __asm   { POP   PSW1    }               \
    __asm   { POP   DR0     }               \
    __asm   { POP   DR4     }               \
    __asm   { POP   DR8     }               \
    __asm   { POP   DR16    }               \
    __asm   { POP   DR20    }               \
    __asm   { POP   DR24    }               \
    __asm   { POP   DR28    }               \
}


#endif

