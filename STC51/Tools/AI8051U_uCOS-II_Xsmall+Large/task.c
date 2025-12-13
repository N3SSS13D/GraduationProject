/*------------------------------------------------------------------------------
STARTUP_TASK.C
------------------------------------------------------------------------------*/
#include "config.H"
#include "UART.h"

void isp_monitoring(u8 *buf, u16 len);

/*------------------------------------------------------------------------------
静态任务堆栈
------------------------------------------------------------------------------*/
OS_STK STARTUP_TASK_STK[APP_CFG_STARTUP_TASK_STK_SIZE];     //任务栈
OS_STK TASK_B_STK[APP_CFG_TASK_B_STK_SIZE];                 //任务栈
OS_STK TASK_C_STK[APP_CFG_TASK_C_STK_SIZE];                 //任务栈


/*------------------------------------------------------------------------------
全局变量
------------------------------------------------------------------------------*/
OS_EVENT    *Uart1Mutex;        //控制UART1互斥访问

OS_EVENT    *testSem;        
OS_EVENT    *testMbox;        
OS_FLAG_GRP *testFlag;
void * xdata MessageStorage[8]; //队列 消息指针数组
OS_EVENT    *testQ;
OS_TMR      *testTmr;
u32 xdata MEM_32X4[32] __attribute__((aligned (4)));    //内存管理
OS_MEM * testMem32;                                     


/*-----------------------------------------------------------*
TMR 回调
*-----------------------------------------------------------*/
void MyCallback (void *ptmr, void *p_arg)
{
    P24=~P24;
    if(ptmr);
    if(p_arg);
}

/*------------------------------------------------------------------------------
AppPrepare
------------------------------------------------------------------------------*/
void AppPrepare(void)
{
    u8 perr;
    
#if OS_TASK_STAT_EN > 0u
    OSStatInit();   //统计任务初始化. 为了避免中断对统计任务初值造成影响, 应在启用外设前, 初始化统计任务.
#endif
    
    UART1_Init(115200,UART1_BRT_Timer2,UART1_SW_P30_P31);

    OSTimeDly(500);
    
    testMem32 = OSMemCreate (MEM_32X4,4,32,&perr);
    if(perr){  printf("OSMemCreate Error; %u\r\n", (u16)perr);   while(1);}
    
    testSem = OSSemCreate (0);
    if(testSem==NULL){  printf("OSSemCreate Error; %u\r\n", (u16)perr);   while(1);}
    
    testMbox = OSMboxCreate(NULL);
    if(testMbox==NULL){  printf("OSMboxCreate Error; %u\r\n", (u16)perr);   while(1);}
    
    testFlag = OSFlagCreate(0,&perr);
    if(perr){  printf("OSFlagCreate Error; %u\r\n", (u16)perr);   while(1);}
    
    testQ = OSQCreate(MessageStorage,8);
    if(testQ==NULL){  printf("OSQCreate Error; %u\r\n", (u16)perr);   while(1);}
    
    testTmr  = OSTmrCreate(0,25,OS_TMR_OPT_PERIODIC,MyCallback,NULL,NULL,&perr);
    if(perr){  printf("OSTmrCreate Error; %u\r\n", (u16)perr);   while(1);}
    
    OSTmrStart(testTmr,&perr);
    if(perr){  printf("OSTmrStart Error; %u\r\n", (u16)perr);   while(1);}
    
    Uart1Mutex=OSMutexCreate(APP_CFG_UART1_MUTEX_PRIO,&perr);
    if(perr){  printf("OSMutexCreate Error; %u\r\n", (u16)perr);   while(1);}
    

    OSTaskCreateExt(    TASK_B,                                         //任务名             
                        (void*)0,                                       //传递参数
                        TASK_B_STK,                                     //栈顶
                        APP_CFG_TASK_B_PRIO,                            //优先级
                        APP_CFG_TASK_B_PRIO,                            //任务ID
                        &TASK_B_STK[APP_CFG_TASK_B_STK_SIZE-1],         //栈底
                        APP_CFG_TASK_B_STK_SIZE,                        //堆栈大小
                        (void*)0,                                       //扩展 备用指针
                        OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR         //允许堆栈检查，初始化堆栈清零   
                    ); 
    
    OSTaskCreateExt(    TASK_C,                                         //任务名             
                        (void*)0,                                       //传递参数
                        TASK_C_STK,                                     //栈顶
                        APP_CFG_TASK_C_PRIO,                            //优先级
                        APP_CFG_TASK_C_PRIO,                            //任务ID
                        &TASK_C_STK[APP_CFG_TASK_C_STK_SIZE-1],         //栈底
                        APP_CFG_TASK_C_STK_SIZE,                        //堆栈大小
                        (void*)0,                                       //扩展 备用指针
                        OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR         //允许堆栈检查，初始化堆栈清零   
                    ); 
    
}

/*------------------------------------------------------------------------------
任务STARTUP
------------------------------------------------------------------------------*/
void STARTUP_TASK(void *ppdata)
{
    AppPrepare();

    while(1)
    {
        
        u8 uart_dat[16],len,perr;
        
        OSMutexPend (Uart1Mutex,0,&perr);
        do{
            len = UART1_Receive(uart_dat,16);   //读串口数据
            UART1_Send(uart_dat,len);           //原路返回
#if USE_STCISP_MOMITORING
            isp_monitoring(uart_dat,len);
#endif
        }while(len==16);
        OSMutexPost(Uart1Mutex);
       

        P27=!P27;
        OSTimeDly(50);
    }
    if(ppdata);
}



/*------------------------------------------------------------------------------
TASK_B
------------------------------------------------------------------------------*/
void TASK_B(void *ppdata)
{
    u8 perr;
    
    while(1){
        
        OSSchedLock();
        OSSchedUnlock();
        
        OSSemPost(testSem);
        OSMboxPost(testMbox,testMbox);
        OSFlagPost(testFlag,0x01,OS_FLAG_SET,&perr);
        OSQPost(testQ,testQ);
        OSTaskSuspend(OS_PRIO_SELF);
        
        P23=~P23;
        
    }
    if(ppdata);
}


/*------------------------------------------------------------------------------
TASK_C
------------------------------------------------------------------------------*/
void TASK_C(void *ppdata)
{
    u8 perr;
    
    while(1)
    {
        
        OSSemPend (testSem,0,&perr);
        OSMboxPend(testMbox,0,&perr);
        OSQPend(testQ,0,&perr);
        OSFlagPend (testFlag,0x01,OS_FLAG_WAIT_SET_ANY+OS_FLAG_CONSUME,0,&perr);

        OSTimeDly(500);
        P22=~P22;

        OSTaskResume(APP_CFG_TASK_B_PRIO);

    }
    if(ppdata);
}



/*------------------------------------------------------------------------------
VisualizationHook,  大约每秒会被调用一次
------------------------------------------------------------------------------*/
void VisualizationHook(void)
{
    u8 perr,*testPt;
    testPt = OSMemGet (testMem32,&perr);
    if(perr){
        printf("OSMemGet Error; %u\r\n", (u16)perr);   
        while(1);
    }else{
        printf("MemAddr;    0X%lX\r\n", (u32)testPt );
        OSMemPut(testMem32,testPt);
    }
    
    P20=~P20;
}






