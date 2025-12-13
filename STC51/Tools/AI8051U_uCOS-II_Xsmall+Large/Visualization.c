/*------------------------------------------------------------------------------
Visualization.c
------------------------------------------------------------------------------*/

#include "config.h"


#if OS_TASK_STAT_EN > 0u

void VisualizationHook(void);

/*------------------------------------------------------------------------------
printf OS运行信息
------------------------------------------------------------------------------*/
void printfVisualization(u16 usage)
{
    printf("CPUUsage:   %u%%\r\n",usage);
    printf("MSTK_Used:  %u/%u\r\n", MStkState.Used,MStkState.Size);

    #if (OS_TASK_STAT_STK_CHK_EN > 0u) && (OS_TASK_CREATE_EXT_EN > 0u)
    {
        OS_TCB      *ptcb;
        INT8U        prio;
        for (prio = 0u; prio <= OS_TASK_IDLE_PRIO; prio++) {
            ptcb = OSTCBPrioTbl[prio];
            if ( (ptcb != (OS_TCB *)0) && (ptcb != OS_TCB_RESERVED) && ((ptcb->OSTCBOpt & OS_TASK_OPT_STK_CHK) != 0u) ){
                printf("TASK%.2u:     %u/%u\r\n", (u16)prio,  (u16)OSTCBPrioTbl[prio]->OSTCBStkUsed, (u16)OSTCBPrioTbl[prio]->OSTCBStkSize);
            }
        }
    }
    #endif
    
    VisualizationHook();
    
    printf("OSTime;     %lu\r\n", (u32)OSTimeGet() );
}


/*------------------------------------------------------------------------------
User_TaskStatHook,大约100毫秒被统计任务调用一次
------------------------------------------------------------------------------*/
void User_TaskStatHook(void)
{
    #define CPUUSAGE_AV_CNT  10      //10次, 即1秒左右, printf一次运行信息
    static u16    cumulative=0;
    static u8   number=0;
    cumulative+=OSCPUUsage;
    if(++number>=CPUUSAGE_AV_CNT){
        u8 perr;
        OsMStkStkChk();
        OSMutexPend (Uart1Mutex,0,&perr);
        printfVisualization(cumulative/CPUUSAGE_AV_CNT);
        OSMutexPost(Uart1Mutex);
        cumulative = number = 0;
    }
}

#endif
