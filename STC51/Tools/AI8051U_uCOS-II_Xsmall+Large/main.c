/*---------------------------------------------------------
MAIN.C
----------------------------------------------------------*/

#include "config.h"
#include "STC32G_GPIO.h"

void XOSCClkConfig(u8 div);


/*---------------------------------------------------------
GPIO_config
----------------------------------------------------------*/
void GPIO_config(void)
{
    P0_MODE_IO_PU(GPIO_Pin_All);        //P0 设置为准双向口
    P1_MODE_IO_PU(GPIO_Pin_All);        //P1 设置为准双向口
    P2_MODE_IO_PU(GPIO_Pin_All);        //P2 设置为准双向口
    P3_MODE_IO_PU(GPIO_Pin_All);        //P3 设置为准双向口
    P4_MODE_IO_PU(GPIO_Pin_All);        //P4 设置为准双向口
    P5_MODE_IO_PU(GPIO_Pin_All);        //P5 设置为准双向口
    P6_MODE_IO_PU(GPIO_Pin_All);        //P6 设置为准双向口
    P7_MODE_IO_PU(GPIO_Pin_All);        //P7 设置为准双向口
    P7_MODE_IO_PU(GPIO_Pin_All);        //P7 设置为准双向口
}


/*---------------------------------------------------------
main
----------------------------------------------------------*/
void main(void)
{
    WTST = 0;       //设置程序指令延时参数，赋值为0可将CPU执行指令的速度设置为最快
    EAXSFR();       //扩展SFR(XFR)访问使能 
    CKCON = 0;      //提高访问XRAM速度
    
    GPIO_config();  //GPIO 初始化
    
    #if USE_Extern_Fosc
        XOSCClkConfig(1);       //切换时钟
    #endif
    
    OSInit();        //OS初始化    
    
    //创建起始任务
     OSTaskCreateExt(    STARTUP_TASK,                                      //任务名             
                        (void*)0,                                           //传递参数
                        STARTUP_TASK_STK,                                   //栈顶
                        APP_CFG_STARTUP_TASK_PRIO,                          //优先级
                        APP_CFG_STARTUP_TASK_PRIO,                          //任务ID
                        &STARTUP_TASK_STK[APP_CFG_STARTUP_TASK_STK_SIZE-1], //栈底
                        APP_CFG_STARTUP_TASK_STK_SIZE,                      //堆栈大小
                        (void*)0,                                           //扩展 备用指针
                        OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR             //允许堆栈检查，初始化堆栈清零   
                   );
    
    OSStart();        //开始任务    
    
    while(1);
}


/*---------------------------------------------------------
* STCISP监测, 只要监测到连续重复的数据到一定数量就复位
* 不检查命令字
* 调试完成后需要屏蔽本函数, 以避免意外重启
----------------------------------------------------------*/
#if USE_STCISP_MOMITORING
void isp_monitoring(u8 *buf, u16 len)
{
    static u8 xdata nu=0, key=0;
    while(len--){
        if(*buf==key){
            nu++;
        }else{
            nu=0;
            key=*buf;
        }
        buf++;
        if(nu>=STCISP_KEYWORDS_COUNT) IAP_CONTR=0X60;
    }
}
#endif




