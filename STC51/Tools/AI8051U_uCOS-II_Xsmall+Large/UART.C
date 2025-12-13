/*---------------------------------------------------------
UART.C
----------------------------------------------------------*/

/*---------------------------------------------------------
文件包含
----------------------------------------------------------*/
#include "UART.H"


#define BTL_LOAD   (65536UL-(MAIN_Fosc/BRT+2UL)/4UL)


///////////////////////////UART1//////////////////////////
#if UART1_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART1_TX_BUF[UART1_TX_BUF_SIZE];
static u16          UART1_TX_W_ADDR=0;
static u16          UART1_TX_R_ADDR=0;
#if UART1_USE_DMA_EN == 1    
static u16          UART1_TX_DMA_SIZE =0;
#endif
static bit          UART1_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART1_RX_BUF[UART1_RX_BUF_SIZE];
#if UART1_USE_DMA_EN != 1    
static u16          UART1_RX_W_ADDR=0;
#endif
static u16          UART1_RX_R_ADDR=0;


#if UART1_USE_DMA_EN == 1    
/*---------------------------------------------------------
DMA_UART1TX中断
----------------------------------------------------------*/
void DMA_UART1TX_ISR_Handler (void) interrupt DMA_UR1T_VECTOR
{
    DMA_UR1T_STA = 0;           //清除中断标志
    UART1_TX_R_ADDR+=UART1_TX_DMA_SIZE;     //发送指针递加
    if(UART1_TX_R_ADDR>=UART1_TX_BUF_SIZE)UART1_TX_R_ADDR=0;    //指针循环调整
    if(UART1_TX_W_ADDR!=UART1_TX_R_ADDR)    //如果有数据要发出
    {
        UART1_TX_DMA_SIZE = (UART1_TX_W_ADDR>UART1_TX_R_ADDR)? UART1_TX_W_ADDR-UART1_TX_R_ADDR : UART1_TX_BUF_SIZE-UART1_TX_R_ADDR;
        DMA_UR1T_AMTH = (UART1_TX_DMA_SIZE-1)>>8;             //字节数
        DMA_UR1T_AMT  = (UART1_TX_DMA_SIZE-1);                //字节数
        DMA_UR1T_TXAH = (u16)(&UART1_TX_BUF[UART1_TX_R_ADDR])>>8;   //地址
        DMA_UR1T_TXAL = (u16)(&UART1_TX_BUF[UART1_TX_R_ADDR]);      //地址
        DMA_UR1T_CR   = 0XC0;                           //启动DMA
    }else{
        UART1_TX_IDLE = 1;
        UART1_set485RxMode();
    }
}

/*---------------------------------------------------------
DMA_UART1RX中断
----------------------------------------------------------*/
void DMA_UART1RX_ISR_Handler (void) interrupt DMA_UR1R_VECTOR
{
    DMA_UR1R_STA = 0x00;    //清除中断标志
    DMA_UR1R_CR  = 0XA1;    //开DMA接收
}

/*---------------------------------------------------------
DMA_UART1_Init
----------------------------------------------------------*/
static void DMA_UART1_Init(void)            
{
    DMA_UR1T_STA  = 0x00;                           //清除标志
    DMA_UR1T_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    
    DMA_UR1R_STA  = 0x00;                           //清除标志
    DMA_UR1R_AMTH = (UART1_RX_BUF_SIZE-1)>>8;       //字节数
    DMA_UR1R_AMT  = (UART1_RX_BUF_SIZE-1);          //字节数        
    DMA_UR1R_RXAH = (u16)UART1_RX_BUF>>8;           //地址
    DMA_UR1R_RXAL = (u16)UART1_RX_BUF;              //地址
    DMA_UR1R_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    DMA_UR1R_CR   = 0XA1;                           //开DMA接收
}

/*---------------------------------------------------------
返回DMA_UR1R_DONE
----------------------------------------------------------*/
static u16 getUr1rDone(void)
{
    u8 a,b,c;
    OS_ENTER_CRITICAL();
    do{
        a = DMA_UR1R_DONEH;
        b = DMA_UR1R_DONE;
        c = DMA_UR1R_DONEH;
    }while(a!=c);
    OS_EXIT_CRITICAL();
    return ((u16)c<<8)+(u16)b;
}

#else

/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART1_ISR_Handler (void) interrupt UART1_VECTOR
{
    if(TI){
        TI=0;
        if(UART1_TX_W_ADDR!=UART1_TX_R_ADDR){
            SBUF=UART1_TX_BUF[UART1_TX_R_ADDR++];
            if(UART1_TX_R_ADDR>=UART1_TX_BUF_SIZE)UART1_TX_R_ADDR=0;
        }else{
            UART1_TX_IDLE=1; 
            UART1_set485RxMode();
        }
    }
    if(RI){
        RI=0;
        UART1_RX_BUF[UART1_RX_W_ADDR++]=SBUF;
        if(UART1_RX_W_ADDR>=UART1_RX_BUF_SIZE)UART1_RX_W_ADDR=0;
    }
}

#endif

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART1_GetCapacity( void )
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART1_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART1_TX_W_ADDR>=tx_r_addr)? UART1_TX_BUF_SIZE-(UART1_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART1_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART1_Send( void *pt, u16 Size)
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART1_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART1_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART1_TX_BUF_SIZE){
        memcpy(&UART1_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART1_TX_BUF_SIZE-tx_w_addr;
        memcpy(&UART1_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcpy(&UART1_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART1_TX_W_ADDR = tx_w_addr;
    if(UART1_TX_IDLE){
        if(UART1_TX_W_ADDR!=UART1_TX_R_ADDR){
            UART1_set485TxMode();
            UART1_TX_IDLE=0;
#if UART1_USE_DMA_EN == 1    
            UART1_TX_DMA_SIZE = (UART1_TX_W_ADDR>=UART1_TX_R_ADDR)? UART1_TX_W_ADDR-UART1_TX_R_ADDR : UART1_TX_BUF_SIZE-UART1_TX_R_ADDR;
            DMA_UR1T_AMTH = (UART1_TX_DMA_SIZE-1)>>8;
            DMA_UR1T_AMT  = (UART1_TX_DMA_SIZE-1);
            DMA_UR1T_TXAH = (u16)(&UART1_TX_BUF[UART1_TX_R_ADDR])>>8;
            DMA_UR1T_TXAL = (u16)(&UART1_TX_BUF[UART1_TX_R_ADDR]);
            DMA_UR1T_CR   = 0XC0;
#else
            SBUF=UART1_TX_BUF[UART1_TX_R_ADDR++];
            if(UART1_TX_R_ADDR>=UART1_TX_BUF_SIZE)UART1_TX_R_ADDR=0;
#endif
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART1_Receive(u8 *buf, u16 Size)
{
    u16 rx_w_addr,nu=0;
#if UART1_USE_DMA_EN == 1    
    rx_w_addr=getUr1rDone();
#else
    OS_ENTER_CRITICAL();
    rx_w_addr=UART1_RX_W_ADDR;
    OS_EXIT_CRITICAL();
#endif
    nu = (rx_w_addr>=UART1_RX_R_ADDR)? rx_w_addr-UART1_RX_R_ADDR : rx_w_addr+UART1_RX_BUF_SIZE-UART1_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART1_RX_R_ADDR+nu)<UART1_RX_BUF_SIZE){
        memcpy(buf,&UART1_RX_BUF[UART1_RX_R_ADDR],nu);
        UART1_RX_R_ADDR+=nu;
    }else{
        u16 len = UART1_RX_BUF_SIZE-UART1_RX_R_ADDR;
        memcpy(buf,&UART1_RX_BUF[UART1_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcpy(buf,&UART1_RX_BUF[0],len);
        UART1_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART1_setBRT
----------------------------------------------------------*/
void UART1_setBRT( u32 BRT, u8 brt_Timer )
{
    if(brt_Timer==UART1_BRT_Timer1){
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
    }else{
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
    }
}

/*---------------------------------------------------------
UART1_Init
----------------------------------------------------------*/
void UART1_Init(u32 BRT, u8 brt_Timer, u8 sw)
{
    UART1_SW(sw);
    SCON = 0x50;
    if(brt_Timer==UART1_BRT_Timer1){
        AUXR |= 0x40;
        AUXR &= 0xFE;
        TMOD &= 0x0F;
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
        TR1 = 1;
    }else{
        AUXR |= 0x01;
        AUXR |= 0x04;
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
        AUXR |= 0x10;
    }
#if UART1_USE_DMA_EN == 1    
    UART1_Interrupt(0);
    DMA_UART1_Init();
#else
    UART1_Priority(Priority_2);
    UART1_Interrupt(1);
#endif
}
#endif


///////////////////////////UART2//////////////////////////
#if UART2_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART2_TX_BUF[UART2_TX_BUF_SIZE];
static u16          UART2_TX_W_ADDR=0;
static u16          UART2_TX_R_ADDR=0;
#if UART2_USE_DMA_EN == 1    
static u16          UART2_TX_DMA_SIZE =0;
#endif
static bit          UART2_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART2_RX_BUF[UART2_RX_BUF_SIZE];
#if UART2_USE_DMA_EN != 1    
static u16          UART2_RX_W_ADDR=0;
#endif
static u16          UART2_RX_R_ADDR=0;


#if UART2_USE_DMA_EN == 1    
/*---------------------------------------------------------
DMA_UART2TX中断
----------------------------------------------------------*/
void DMA_UART2TX_ISR_Handler (void) interrupt DMA_UR2T_VECTOR
{
    DMA_UR2T_STA = 0;           //清除中断标志
    UART2_TX_R_ADDR+=UART2_TX_DMA_SIZE;     //发送指针递加
    if(UART2_TX_R_ADDR>=UART2_TX_BUF_SIZE)UART2_TX_R_ADDR=0;    //指针循环调整
    if(UART2_TX_W_ADDR!=UART2_TX_R_ADDR)    //如果有数据要发出
    {
        UART2_TX_DMA_SIZE = (UART2_TX_W_ADDR>UART2_TX_R_ADDR)? UART2_TX_W_ADDR-UART2_TX_R_ADDR : UART2_TX_BUF_SIZE-UART2_TX_R_ADDR;
        DMA_UR2T_AMTH = (UART2_TX_DMA_SIZE-1)>>8;             //字节数
        DMA_UR2T_AMT  = (UART2_TX_DMA_SIZE-1);                //字节数
        DMA_UR2T_TXAH = (u16)(&UART2_TX_BUF[UART2_TX_R_ADDR])>>8;   //地址
        DMA_UR2T_TXAL = (u16)(&UART2_TX_BUF[UART2_TX_R_ADDR]);      //地址
        DMA_UR2T_CR   = 0XC0;                           //启动DMA
    }else{
        UART2_TX_IDLE = 1;
        UART2_set485RxMode();
    }
}

/*---------------------------------------------------------
DMA_UART2RX中断
----------------------------------------------------------*/
void DMA_UART2RX_ISR_Handler (void) interrupt DMA_UR2R_VECTOR
{
    DMA_UR2R_STA = 0x00;    //清除中断标志
    DMA_UR2R_CR  = 0XA1;    //开DMA接收
}

/*---------------------------------------------------------
DMA_UART2_Init
----------------------------------------------------------*/
static void DMA_UART2_Init(void)            
{
    DMA_UR2T_STA  = 0x00;                           //清除标志
    DMA_UR2T_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    
    DMA_UR2R_STA  = 0x00;                           //清除标志
    DMA_UR2R_AMTH = (UART2_RX_BUF_SIZE-1)>>8;       //字节数
    DMA_UR2R_AMT  = (UART2_RX_BUF_SIZE-1);          //字节数        
    DMA_UR2R_RXAH = (u16)UART2_RX_BUF>>8;           //地址
    DMA_UR2R_RXAL = (u16)UART2_RX_BUF;              //地址
    DMA_UR2R_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    DMA_UR2R_CR   = 0XA1;                           //开DMA接收
}

/*---------------------------------------------------------
返回DMA_UR2R_DONE
----------------------------------------------------------*/
static u16 getUr2rDone(void)
{
    u8 a,b,c;
    OS_ENTER_CRITICAL();
    do{
        a = DMA_UR2R_DONEH;
        b = DMA_UR2R_DONE;
        c = DMA_UR2R_DONEH;
    }while(a!=c);
    OS_EXIT_CRITICAL();
    return ((u16)c<<8)+(u16)b;
}

#else

/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART2_ISR_Handler (void) interrupt UART2_VECTOR
{
    if(S2TI){
        S2TI=0;
        if(UART2_TX_W_ADDR!=UART2_TX_R_ADDR){
            S2BUF=UART2_TX_BUF[UART2_TX_R_ADDR++];
            if(UART2_TX_R_ADDR>=UART2_TX_BUF_SIZE)UART2_TX_R_ADDR=0;
        }else{
            UART2_TX_IDLE=1; 
            UART2_set485RxMode();
        }
    }
    if(S2RI){
        S2RI=0;
        UART2_RX_BUF[UART2_RX_W_ADDR++]=S2BUF;
        if(UART2_RX_W_ADDR>=UART2_RX_BUF_SIZE)UART2_RX_W_ADDR=0;
    }
}

#endif

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART2_GetCapacity( void )
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART2_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART2_TX_W_ADDR>=tx_r_addr)? UART2_TX_BUF_SIZE-(UART2_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART2_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART2_Send( void *pt, u16 Size)
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART2_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART2_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART2_TX_BUF_SIZE){
        memcpy(&UART2_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART2_TX_BUF_SIZE-tx_w_addr;
        memcpy(&UART2_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcpy(&UART2_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART2_TX_W_ADDR = tx_w_addr;
    if(UART2_TX_IDLE){
        if(UART2_TX_W_ADDR!=UART2_TX_R_ADDR){
            UART2_set485TxMode();
            UART2_TX_IDLE=0;
#if UART2_USE_DMA_EN == 1    
            UART2_TX_DMA_SIZE = (UART2_TX_W_ADDR>=UART2_TX_R_ADDR)? UART2_TX_W_ADDR-UART2_TX_R_ADDR : UART2_TX_BUF_SIZE-UART2_TX_R_ADDR;
            DMA_UR2T_AMTH = (UART2_TX_DMA_SIZE-1)>>8;
            DMA_UR2T_AMT  = (UART2_TX_DMA_SIZE-1);
            DMA_UR2T_TXAH = (u16)(&UART2_TX_BUF[UART2_TX_R_ADDR])>>8;
            DMA_UR2T_TXAL = (u16)(&UART2_TX_BUF[UART2_TX_R_ADDR]);
            DMA_UR2T_CR   = 0XC0;
#else
            S2BUF=UART2_TX_BUF[UART2_TX_R_ADDR++];
            if(UART2_TX_R_ADDR>=UART2_TX_BUF_SIZE)UART2_TX_R_ADDR=0;
#endif
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART2_Receive(u8 *buf, u16 Size)
{
    u16 rx_w_addr,nu=0;
#if UART2_USE_DMA_EN == 1    
    rx_w_addr=getUr2rDone();
#else
    OS_ENTER_CRITICAL();
    rx_w_addr=UART2_RX_W_ADDR;
    OS_EXIT_CRITICAL();
#endif
    nu = (rx_w_addr>=UART2_RX_R_ADDR)? rx_w_addr-UART2_RX_R_ADDR : rx_w_addr+UART2_RX_BUF_SIZE-UART2_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART2_RX_R_ADDR+nu)<UART2_RX_BUF_SIZE){
        memcpy(buf,&UART2_RX_BUF[UART2_RX_R_ADDR],nu);
        UART2_RX_R_ADDR+=nu;
    }else{
        u16 len = UART2_RX_BUF_SIZE-UART2_RX_R_ADDR;
        memcpy(buf,&UART2_RX_BUF[UART2_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcpy(buf,&UART2_RX_BUF[0],len);
        UART2_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART2_setBRT
----------------------------------------------------------*/
void UART2_setBRT( u32 BRT )
{
    T2L = BTL_LOAD;
    T2H = BTL_LOAD>>8;
}

/*---------------------------------------------------------
UART2_Init
----------------------------------------------------------*/
void UART2_Init(u32 BRT, u8 sw)
{
    UART2_SW(sw);
    
    S2CFG |= 0x01;  //使用串口2时，W1位必需设置为1，否则可能会产生不可预期的错误
    S2CON = 0x50;
    AUXR |= 0x04;
    T2L = BTL_LOAD;
    T2H = BTL_LOAD>>8;
    AUXR |= 0x10;
    
#if UART2_USE_DMA_EN == 1    
    UART2_Interrupt(0);
    DMA_UART2_Init();
#else
    UART2_Priority(Priority_2);
    UART2_Interrupt(1);
#endif
}
#endif


///////////////////////////UART3//////////////////////////
#if UART3_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART3_TX_BUF[UART3_TX_BUF_SIZE];
static u16          UART3_TX_W_ADDR=0;
static u16          UART3_TX_R_ADDR=0;
#if UART3_USE_DMA_EN == 1    
static u16          UART3_TX_DMA_SIZE =0;
#endif
static bit          UART3_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART3_RX_BUF[UART3_RX_BUF_SIZE];
#if UART3_USE_DMA_EN != 1    
static u16          UART3_RX_W_ADDR=0;
#endif
static u16          UART3_RX_R_ADDR=0;


#if UART3_USE_DMA_EN == 1    
/*---------------------------------------------------------
DMA_UART3TX中断
----------------------------------------------------------*/
void DMA_UART3TX_ISR_Handler (void) interrupt DMA_UR3T_VECTOR
{
    DMA_UR3T_STA = 0;           //清除中断标志
    UART3_TX_R_ADDR+=UART3_TX_DMA_SIZE;     //发送指针递加
    if(UART3_TX_R_ADDR>=UART3_TX_BUF_SIZE)UART3_TX_R_ADDR=0;    //指针循环调整
    if(UART3_TX_W_ADDR!=UART3_TX_R_ADDR)    //如果有数据要发出
    {
        UART3_TX_DMA_SIZE = (UART3_TX_W_ADDR>UART3_TX_R_ADDR)? UART3_TX_W_ADDR-UART3_TX_R_ADDR : UART3_TX_BUF_SIZE-UART3_TX_R_ADDR;
        DMA_UR3T_AMTH = (UART3_TX_DMA_SIZE-1)>>8;             //字节数
        DMA_UR3T_AMT  = (UART3_TX_DMA_SIZE-1);                //字节数
        DMA_UR3T_TXAH = (u16)(&UART3_TX_BUF[UART3_TX_R_ADDR])>>8;   //地址
        DMA_UR3T_TXAL = (u16)(&UART3_TX_BUF[UART3_TX_R_ADDR]);      //地址
        DMA_UR3T_CR   = 0XC0;                           //启动DMA
    }else{
        UART3_TX_IDLE = 1;
        UART3_set485RxMode();
    }
}

/*---------------------------------------------------------
DMA_UART3RX中断
----------------------------------------------------------*/
void DMA_UART3RX_ISR_Handler (void) interrupt DMA_UR3R_VECTOR
{
    DMA_UR3R_STA = 0x00;    //清除中断标志
    DMA_UR3R_CR  = 0XA1;    //开DMA接收
}

/*---------------------------------------------------------
DMA_UART3_Init
----------------------------------------------------------*/
static void DMA_UART3_Init(void)            
{
    DMA_UR3T_STA  = 0x00;                           //清除标志
    DMA_UR3T_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    
    DMA_UR3R_STA  = 0x00;                           //清除标志
    DMA_UR3R_AMTH = (UART3_RX_BUF_SIZE-1)>>8;       //字节数
    DMA_UR3R_AMT  = (UART3_RX_BUF_SIZE-1);          //字节数        
    DMA_UR3R_RXAH = (u16)UART3_RX_BUF>>8;           //地址
    DMA_UR3R_RXAL = (u16)UART3_RX_BUF;              //地址
    DMA_UR3R_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    DMA_UR3R_CR   = 0XA1;                           //开DMA接收
}

/*---------------------------------------------------------
返回DMA_UR3R_DONE
----------------------------------------------------------*/
static u16 getUr3rDone(void)
{
    u8 a,b,c;
    OS_ENTER_CRITICAL();
    do{
        a = DMA_UR3R_DONEH;
        b = DMA_UR3R_DONE;
        c = DMA_UR3R_DONEH;
    }while(a!=c);
    OS_EXIT_CRITICAL();
    return ((u16)c<<8)+(u16)b;
}

#else

/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART3_ISR_Handler (void) interrupt UART3_VECTOR
{
    if(S3TI){
        S3TI=0;
        if(UART3_TX_W_ADDR!=UART3_TX_R_ADDR){
            S3BUF=UART3_TX_BUF[UART3_TX_R_ADDR++];
            if(UART3_TX_R_ADDR>=UART3_TX_BUF_SIZE)UART3_TX_R_ADDR=0;
        }else{
            UART3_TX_IDLE=1; 
            UART3_set485RxMode();
        }
    }
    if(S3RI){
        S3RI=0;
        UART3_RX_BUF[UART3_RX_W_ADDR++]=S3BUF;
        if(UART3_RX_W_ADDR>=UART3_RX_BUF_SIZE)UART3_RX_W_ADDR=0;
    }
}

#endif

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART3_GetCapacity( void )
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART3_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART3_TX_W_ADDR>=tx_r_addr)? UART3_TX_BUF_SIZE-(UART3_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART3_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART3_Send( void *pt, u16 Size)
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART3_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART3_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART3_TX_BUF_SIZE){
        memcpy(&UART3_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART3_TX_BUF_SIZE-tx_w_addr;
        memcpy(&UART3_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcpy(&UART3_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART3_TX_W_ADDR = tx_w_addr;
    if(UART3_TX_IDLE){
        if(UART3_TX_W_ADDR!=UART3_TX_R_ADDR){
            UART3_set485TxMode();
            UART3_TX_IDLE=0;
#if UART3_USE_DMA_EN == 1    
            UART3_TX_DMA_SIZE = (UART3_TX_W_ADDR>=UART3_TX_R_ADDR)? UART3_TX_W_ADDR-UART3_TX_R_ADDR : UART3_TX_BUF_SIZE-UART3_TX_R_ADDR;
            DMA_UR3T_AMTH = (UART3_TX_DMA_SIZE-1)>>8;
            DMA_UR3T_AMT  = (UART3_TX_DMA_SIZE-1);
            DMA_UR3T_TXAH = (u16)(&UART3_TX_BUF[UART3_TX_R_ADDR])>>8;
            DMA_UR3T_TXAL = (u16)(&UART3_TX_BUF[UART3_TX_R_ADDR]);
            DMA_UR3T_CR   = 0XC0;
#else
            S3BUF=UART3_TX_BUF[UART3_TX_R_ADDR++];
            if(UART3_TX_R_ADDR>=UART3_TX_BUF_SIZE)UART3_TX_R_ADDR=0;
#endif
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART3_Receive(u8 *buf, u16 Size)
{
    u16 rx_w_addr,nu=0;
#if UART3_USE_DMA_EN == 1    
    rx_w_addr=getUr3rDone();
#else
    OS_ENTER_CRITICAL();
    rx_w_addr=UART3_RX_W_ADDR;
    OS_EXIT_CRITICAL();
#endif
    nu = (rx_w_addr>=UART3_RX_R_ADDR)? rx_w_addr-UART3_RX_R_ADDR : rx_w_addr+UART3_RX_BUF_SIZE-UART3_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART3_RX_R_ADDR+nu)<UART3_RX_BUF_SIZE){
        memcpy(buf,&UART3_RX_BUF[UART3_RX_R_ADDR],nu);
        UART3_RX_R_ADDR+=nu;
    }else{
        u16 len = UART3_RX_BUF_SIZE-UART3_RX_R_ADDR;
        memcpy(buf,&UART3_RX_BUF[UART3_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcpy(buf,&UART3_RX_BUF[0],len);
        UART3_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART3_setBRT
----------------------------------------------------------*/
void UART3_setBRT( u32 BRT, u8 brt_Timer )
{
    if(brt_Timer==UART3_BRT_Timer3){
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
    }else{
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
    }
}

/*---------------------------------------------------------
UART3_Init
----------------------------------------------------------*/
void UART3_Init(u32 BRT, u8 brt_Timer, u8 sw)
{
    UART3_SW(sw);
    S3CON = 0x10;
    if(brt_Timer==UART3_BRT_Timer3){
        S3CON |= 0x40;
        T4T3M |= 0x02;
        T3L = BTL_LOAD;
        T3H = BTL_LOAD>>8;
        T4T3M |= 0x08;
    }else{
        S3CON &= 0xBF;
        AUXR |= 0x04;
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
        AUXR |= 0x10;
    }
#if UART3_USE_DMA_EN == 1    
    UART3_Interrupt(0);
    DMA_UART3_Init();
#else
    UART3_Priority(Priority_2);
    UART3_Interrupt(1);
#endif
}
#endif


///////////////////////////UART4//////////////////////////
#if UART4_EN  == 1
/*---------------------------------------------------------
发送部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART4_TX_BUF[UART4_TX_BUF_SIZE];
static u16          UART4_TX_W_ADDR=0;
static u16          UART4_TX_R_ADDR=0;
#if UART4_USE_DMA_EN == 1    
static u16          UART4_TX_DMA_SIZE =0;
#endif
static bit          UART4_TX_IDLE=1;

/*---------------------------------------------------------
接收部分定义及变量
----------------------------------------------------------*/
static u8 xdata     UART4_RX_BUF[UART4_RX_BUF_SIZE];
#if UART4_USE_DMA_EN != 1    
static u16          UART4_RX_W_ADDR=0;
#endif
static u16          UART4_RX_R_ADDR=0;


#if UART4_USE_DMA_EN == 1    
/*---------------------------------------------------------
DMA_UART4TX中断
----------------------------------------------------------*/
void DMA_UART4TX_ISR_Handler (void) interrupt DMA_UR4T_VECTOR
{
    DMA_UR4T_STA = 0;           //清除中断标志
    UART4_TX_R_ADDR+=UART4_TX_DMA_SIZE;     //发送指针递加
    if(UART4_TX_R_ADDR>=UART4_TX_BUF_SIZE)UART4_TX_R_ADDR=0;    //指针循环调整
    if(UART4_TX_W_ADDR!=UART4_TX_R_ADDR)    //如果有数据要发出
    {
        UART4_TX_DMA_SIZE = (UART4_TX_W_ADDR>UART4_TX_R_ADDR)? UART4_TX_W_ADDR-UART4_TX_R_ADDR : UART4_TX_BUF_SIZE-UART4_TX_R_ADDR;
        DMA_UR4T_AMTH = (UART4_TX_DMA_SIZE-1)>>8;             //字节数
        DMA_UR4T_AMT  = (UART4_TX_DMA_SIZE-1);                //字节数
        DMA_UR4T_TXAH = (u16)(&UART4_TX_BUF[UART4_TX_R_ADDR])>>8;   //地址
        DMA_UR4T_TXAL = (u16)(&UART4_TX_BUF[UART4_TX_R_ADDR]);      //地址
        DMA_UR4T_CR   = 0XC0;                           //启动DMA
    }else{
        UART4_TX_IDLE = 1;
        UART4_set485RxMode();
    }
}

/*---------------------------------------------------------
DMA_UART4RX中断
----------------------------------------------------------*/
void DMA_UART4RX_ISR_Handler (void) interrupt DMA_UR4R_VECTOR
{
    DMA_UR4R_STA = 0x00;    //清除中断标志
    DMA_UR4R_CR  = 0XA1;    //开DMA接收
}

/*---------------------------------------------------------
DMA_UART4_Init
----------------------------------------------------------*/
static void DMA_UART4_Init(void)            
{
    DMA_UR4T_STA  = 0x00;                           //清除标志
    DMA_UR4T_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    
    DMA_UR4R_STA  = 0x00;                           //清除标志
    DMA_UR4R_AMTH = (UART4_RX_BUF_SIZE-1)>>8;       //字节数
    DMA_UR4R_AMT  = (UART4_RX_BUF_SIZE-1);          //字节数        
    DMA_UR4R_RXAH = (u16)UART4_RX_BUF>>8;           //地址
    DMA_UR4R_RXAL = (u16)UART4_RX_BUF;              //地址
    DMA_UR4R_CFG  = 0x8A;                           //允许DMA中断, 中断Priority_2优先级,传输Priority_2优先级
    DMA_UR4R_CR   = 0XA1;                           //开DMA接收
}

/*---------------------------------------------------------
返回DMA_UR4R_DONE
----------------------------------------------------------*/
static u16 getUR4rDone(void)
{
    u8 a,b,c;
    OS_ENTER_CRITICAL();
    do{
        a = DMA_UR4R_DONEH;
        b = DMA_UR4R_DONE;
        c = DMA_UR4R_DONEH;
    }while(a!=c);
    OS_EXIT_CRITICAL();
    return ((u16)c<<8)+(u16)b;
}

#else

/*---------------------------------------------------------
串口中断函数
----------------------------------------------------------*/
void UART4_ISR_Handler (void) interrupt UART4_VECTOR
{
    if(S4TI){
        S4TI=0;
        if(UART4_TX_W_ADDR!=UART4_TX_R_ADDR){
            S4BUF=UART4_TX_BUF[UART4_TX_R_ADDR++];
            if(UART4_TX_R_ADDR>=UART4_TX_BUF_SIZE)UART4_TX_R_ADDR=0;
        }else{
            UART4_TX_IDLE=1; 
            UART4_set485RxMode();
        }
    }
    if(S4RI){
        S4RI=0;
        UART4_RX_BUF[UART4_RX_W_ADDR++]=S4BUF;
        if(UART4_RX_W_ADDR>=UART4_RX_BUF_SIZE)UART4_RX_W_ADDR=0;
    }
}

#endif

/*---------------------------------------------------------
返回发送区容量
----------------------------------------------------------*/
u16 UART4_GetCapacity( void )
{
    u16 tx_r_addr;
    OS_ENTER_CRITICAL();
    tx_r_addr = UART4_TX_R_ADDR;
    OS_EXIT_CRITICAL();
    return ( (UART4_TX_W_ADDR>=tx_r_addr)? UART4_TX_BUF_SIZE-(UART4_TX_W_ADDR-tx_r_addr)-1 : tx_r_addr-UART4_TX_W_ADDR-1);
}

/*---------------------------------------------------------
串口发送数据,缓冲区容量不足返回1
----------------------------------------------------------*/
u8 UART4_Send( void *pt, u16 Size)
{    
    u8 *buf = pt;
    u16 tx_w_addr;
    
    if(UART4_GetCapacity()<Size) return 1;
    
    tx_w_addr = UART4_TX_W_ADDR;
    
    if((tx_w_addr+Size)<UART4_TX_BUF_SIZE){
        memcpy(&UART4_TX_BUF[tx_w_addr],buf,Size);
        tx_w_addr+=Size;
    }else{
        u16 len = UART4_TX_BUF_SIZE-tx_w_addr;
        memcpy(&UART4_TX_BUF[tx_w_addr],buf,len);
        buf+=len;
        len=Size-len;
        memcpy(&UART4_TX_BUF[0],buf,len);
        tx_w_addr=len;
    }
    
    OS_ENTER_CRITICAL();
    UART4_TX_W_ADDR = tx_w_addr;
    if(UART4_TX_IDLE){
        if(UART4_TX_W_ADDR!=UART4_TX_R_ADDR){
            UART4_set485TxMode();
            UART4_TX_IDLE=0;
#if UART4_USE_DMA_EN == 1    
            UART4_TX_DMA_SIZE = (UART4_TX_W_ADDR>=UART4_TX_R_ADDR)? UART4_TX_W_ADDR-UART4_TX_R_ADDR : UART4_TX_BUF_SIZE-UART4_TX_R_ADDR;
            DMA_UR4T_AMTH = (UART4_TX_DMA_SIZE-1)>>8;
            DMA_UR4T_AMT  = (UART4_TX_DMA_SIZE-1);
            DMA_UR4T_TXAH = (u16)(&UART4_TX_BUF[UART4_TX_R_ADDR])>>8;
            DMA_UR4T_TXAL = (u16)(&UART4_TX_BUF[UART4_TX_R_ADDR]);
            DMA_UR4T_CR   = 0XC0;
#else
            S4BUF=UART4_TX_BUF[UART4_TX_R_ADDR++];
            if(UART4_TX_R_ADDR>=UART4_TX_BUF_SIZE)UART4_TX_R_ADDR=0;
#endif
        }
    }
    OS_EXIT_CRITICAL();
        
    return 0;
}

/*---------------------------------------------------------
读串口数据
----------------------------------------------------------*/
u16 UART4_Receive(u8 *buf, u16 Size)
{
    u16 rx_w_addr,nu=0;
#if UART4_USE_DMA_EN == 1    
    rx_w_addr=getUR4rDone();
#else
    OS_ENTER_CRITICAL();
    rx_w_addr=UART4_RX_W_ADDR;
    OS_EXIT_CRITICAL();
#endif
    nu = (rx_w_addr>=UART4_RX_R_ADDR)? rx_w_addr-UART4_RX_R_ADDR : rx_w_addr+UART4_RX_BUF_SIZE-UART4_RX_R_ADDR;
    nu = (nu<Size)? nu:Size;
    if((UART4_RX_R_ADDR+nu)<UART4_RX_BUF_SIZE){
        memcpy(buf,&UART4_RX_BUF[UART4_RX_R_ADDR],nu);
        UART4_RX_R_ADDR+=nu;
    }else{
        u16 len = UART4_RX_BUF_SIZE-UART4_RX_R_ADDR;
        memcpy(buf,&UART4_RX_BUF[UART4_RX_R_ADDR],len);
        buf+=len;
        len=nu-len;
        memcpy(buf,&UART4_RX_BUF[0],len);
        UART4_RX_R_ADDR=len;
    }
    return nu;
}

/*---------------------------------------------------------
UART4_setBRT
----------------------------------------------------------*/
void UART4_setBRT( u32 BRT, u8 brt_Timer )
{
    if(brt_Timer==UART4_BRT_Timer4){
        TL1 = BTL_LOAD;
        TH1 = BTL_LOAD>>8;
    }else{
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
    }
}

/*---------------------------------------------------------
UART4_Init
----------------------------------------------------------*/
void UART4_Init(u32 BRT, u8 brt_Timer, u8 sw)
{
    UART4_SW(sw);
    S4CON = 0x10;
    if(brt_Timer==UART4_BRT_Timer4){
        S4CON |= 0x40;
        T4T3M |= 0x20;
        T4L = BTL_LOAD;
        T4H = BTL_LOAD>>8;
        T4T3M |= 0x80;
    }else{
        S4CON &= 0xBF;
        AUXR |= 0x04;
        T2L = BTL_LOAD;
        T2H = BTL_LOAD>>8;
        AUXR |= 0x10;
    }
#if UART4_USE_DMA_EN == 1    
    UART4_Interrupt(0);
    DMA_UART4_Init();
#else
    UART4_Priority(Priority_2);
    UART4_Interrupt(1);
#endif
}
#endif


/*---------------------------------------------------------
putchar
----------------------------------------------------------*/
#pragma functions (static)
#if printf_UART == 1
char putchar (char c)  {
    while(UART1_Send(&c,1));
    return c;
}
#endif

#if printf_UART == 2
char putchar (char c)  {
    while(UART2_Send(&c,1));
    return c;
}
#endif

#if printf_UART == 3
char putchar (char c)  {
    while(UART3_Send(&c,1));
    return c;
}
#endif

#if printf_UART == 4
char putchar (char c)  {
    while(UART4_Send(&c,1));
    return c;
}
#endif


/*---------------------------------------------------------
end
----------------------------------------------------------*/

