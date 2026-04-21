//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息
// 文件名称: uart.c
// 文件描述: UART2 communication support on P4.2/P4.3
// 文件版本: V1.0
// 修改记录:
//   1. (2026-04-20) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#include "config.h"


//<<AICUBE_USER_INCLUDE_BEGIN>>
// 在此添加用户头文件包含
//<<AICUBE_USER_INCLUDE_END>>


//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
// 在此添加用户全局变量定义、用户宏定义以及函数声明
#define UART2_BAUDRATE_VALUE            9600UL
#define UART2_PIN_MASK                  ((uint8_t)((1U << 2) | (1U << 3)))
#define UART2_BT_AT_PIN_MASK            PIN_1
#define UART2_RX_RING_SIZE              192U
#define UART2_RX_DEBUG_SIZE             48U
#define UART2_DMA_RX_BUFFER_SIZE        192U
#define UART2_DMA_TX_BUFFER_SIZE        128U
#define UART2_DMA_RX_IDLE_REARM_TICKS   2U
#define UART2_TX_READY_TIMEOUT          60000U
#define UART2_S2CFG_INIT_VALUE          0x01U
#define UART2_S2CON_INIT_VALUE          0x50U
#define UART2_BAUDRATE_MIN              1200UL
#define UART2_BAUDRATE_MAX              115200UL
#define UART2_DMA_PRIORITY              2U

static uint8_t xdata g_uart2RxRing[UART2_RX_RING_SIZE];
static uint8_t xdata g_uart2RxDebug[UART2_RX_DEBUG_SIZE];
static uint8_t xdata g_uart2RxDmaBuffer[UART2_DMA_RX_BUFFER_SIZE];
static uint8_t xdata g_uart2TxDmaBuffer[UART2_DMA_TX_BUFFER_SIZE];
static volatile uint8_t g_uart2RxWriteIndex = 0U;
static volatile uint8_t g_uart2RxReadIndex = 0U;
static volatile uint8_t g_uart2RxDebugLength = 0U;
static volatile uint8_t g_uart2RxDebugPending = 0U;
static volatile uint8_t g_uart2RxOverflow = 0U;
static volatile uint8_t g_uart2BtAtMode = 0U;
static volatile uint16_t g_uart2RxTotalCount = 0U;
static volatile uint16_t g_uart2RxDmaHandledCount = 0U;
static volatile uint8_t g_uart2RxDmaIdleTicks = 0U;
static volatile uint8_t g_uart2LastRxByte = 0U;
static volatile uint8_t g_uart2LastRxValid = 0U;
static uint32_t xdata g_uart2Baudrate = UART2_BAUDRATE_VALUE;

static uint16_t UART2_GetTimer2Reload(uint32_t baudrate);
static void UART2_ConfigurePins(void);
static uint8_t UART2_ApplyBaudrate(uint32_t baudrate);
static void UART2_ResetRxDmaEngine(void);
static uint16_t UART2_ReadRxDoneStable(void);
static uint16_t UART2_ReadTxDoneStable(void);
static void UART2_PushRxByte(uint8_t rxByte);
static void UART2_ServiceRxDmaInternal(void);
//<<AICUBE_USER_GLOBAL_DEFINE_END>>


static uint16_t UART2_GetTimer2Reload(uint32_t baudrate)
{
    uint32_t divider;

    divider = (MAIN_Fosc / baudrate + 2UL) / 4UL;
    return (uint16_t)(65536UL - divider);
}

static void UART2_ConfigurePins(void)
{
    SetP4nInitLevelHigh(UART2_PIN_MASK);
    SetP4nQuasiMode(UART2_PIN_MASK);
    DisableP4nPullUp(UART2_PIN_MASK);
    DisableP4nPullDown(UART2_PIN_MASK);
    DisableP4nSchmitt(UART2_PIN_MASK);
    SetP4nSlewRateNormal(UART2_PIN_MASK);
    SetP4nDrivingNormal(UART2_PIN_MASK);
    SetP4nDigitalInput(UART2_PIN_MASK);
    SetP4nInitLevelLow(UART2_BT_AT_PIN_MASK);
    SetP4nPushPullMode(UART2_BT_AT_PIN_MASK);
    DisableP4nPullUp(UART2_BT_AT_PIN_MASK);
    DisableP4nPullDown(UART2_BT_AT_PIN_MASK);
    DisableP4nSchmitt(UART2_BT_AT_PIN_MASK);
    SetP4nSlewRateNormal(UART2_BT_AT_PIN_MASK);
    SetP4nDrivingNormal(UART2_BT_AT_PIN_MASK);
    UART2_SwitchP4243();
}

static uint16_t UART2_ReadRxDoneStable(void)
{
    uint8_t highByteA;
    uint8_t lowByte;
    uint8_t highByteB;

    DisableGlobalInt();
    do
    {
        highByteA = DMA_UR2R_DONEH;
        lowByte = DMA_UR2R_DONE;
        highByteB = DMA_UR2R_DONEH;
    }
    while (highByteA != highByteB);
    EnableGlobalInt();

    return (uint16_t)(((uint16_t)highByteB << 8) | (uint16_t)lowByte);
}

static uint16_t UART2_ReadTxDoneStable(void)
{
    uint8_t highByteA;
    uint8_t lowByte;
    uint8_t highByteB;

    DisableGlobalInt();
    do
    {
        highByteA = DMA_UR2T_DONEH;
        lowByte = DMA_UR2T_DONE;
        highByteB = DMA_UR2T_DONEH;
    }
    while (highByteA != highByteB);
    EnableGlobalInt();

    return (uint16_t)(((uint16_t)highByteB << 8) | (uint16_t)lowByte);
}

static void UART2_ResetRxDmaEngine(void)
{
    DisableGlobalInt();
    DMA_UART2_DisableRxInt();
    DMA_UART2_DisableRx();
    DMA_UART2_ClearRxFlag();
    DMA_UART2_ClearRxLossFlag();
    DMA_UART2_ClearFIFO();
    DMA_UART2_SetRxBusPriority(UART2_DMA_PRIORITY);
    DMA_UART2_SetRxIntPriority(UART2_DMA_PRIORITY);
    DMA_UART2_SetRxAddress((uint16_t)g_uart2RxDmaBuffer);
    DMA_UART2_SetRxAmount((uint16_t)(UART2_DMA_RX_BUFFER_SIZE - 1U));
    DMA_UART2_EnableRx();
    DMA_UART2_TriggerRx();
    g_uart2RxDmaHandledCount = 0U;
    g_uart2RxDmaIdleTicks = 0U;
    EnableGlobalInt();
}

static void UART2_PushRxByte(uint8_t rxByte)
{
    uint8_t nextWriteIndex;

    g_uart2LastRxByte = rxByte;
    g_uart2LastRxValid = 1U;
    g_uart2RxTotalCount++;
    if (g_uart2RxDebugLength < UART2_RX_DEBUG_SIZE)
    {
        g_uart2RxDebug[g_uart2RxDebugLength] = rxByte;
        g_uart2RxDebugLength++;
    }
    g_uart2RxDebugPending = 1U;

    nextWriteIndex = (uint8_t)((g_uart2RxWriteIndex + 1U) % UART2_RX_RING_SIZE);
    if (nextWriteIndex == g_uart2RxReadIndex)
    {
        g_uart2RxOverflow = 1U;
        return;
    }

    g_uart2RxRing[g_uart2RxWriteIndex] = rxByte;
    g_uart2RxWriteIndex = nextWriteIndex;
}

static void UART2_ServiceRxDmaInternal(void)
{
    uint16_t currentDone;
    uint16_t copyIndex;

    if (DMA_UART2_CheckRxLossFlag() != 0U)
    {
        g_uart2RxOverflow = 1U;
        DMA_UART2_ClearRxLossFlag();
    }

    currentDone = UART2_ReadRxDoneStable();
    if (currentDone > UART2_DMA_RX_BUFFER_SIZE)
    {
        currentDone = UART2_DMA_RX_BUFFER_SIZE;
    }

    if (currentDone < g_uart2RxDmaHandledCount)
    {
        UART2_ResetRxDmaEngine();
        return;
    }

    if (currentDone > g_uart2RxDmaHandledCount)
    {
        for (copyIndex = g_uart2RxDmaHandledCount; copyIndex < currentDone; ++copyIndex)
        {
            UART2_PushRxByte(g_uart2RxDmaBuffer[copyIndex]);
        }
        g_uart2RxDmaHandledCount = currentDone;
        g_uart2RxDmaIdleTicks = 0U;
    }
    else if (g_uart2RxDmaHandledCount != 0U)
    {
        g_uart2RxDmaIdleTicks++;
        if (g_uart2RxDmaIdleTicks >= UART2_DMA_RX_IDLE_REARM_TICKS)
        {
            UART2_ResetRxDmaEngine();
        }
    }

    if (g_uart2RxDmaHandledCount >= (UART2_DMA_RX_BUFFER_SIZE - 1U))
    {
        UART2_ResetRxDmaEngine();
    }
}

static uint8_t UART2_ApplyBaudrate(uint32_t baudrate)
{
    uint16_t timer2Reload;

    if ((baudrate < UART2_BAUDRATE_MIN) || (baudrate > UART2_BAUDRATE_MAX))
    {
        return 0U;
    }

    timer2Reload = UART2_GetTimer2Reload(baudrate);

    TIMER2_Stop();
    TIMER2_DisableInt();
    S2CFG = UART2_S2CFG_INIT_VALUE;
    S2CON = UART2_S2CON_INIT_VALUE;
    TIMER2_TimerMode();
    TIMER2_1TMode();
    TIMER2_SetPrescale(0);
    TIMER2_SetReload16(timer2Reload);
    TIMER2_ClearFlag();
    UART2_Timer2BRT();
    TIMER2_Run();

    UART2_NoneParity();
    UART2_DisableInt();
    UART2_ClearRxFlag();
    UART2_ClearTxFlag();

    DMA_UART2_DisableTxInt();
    DMA_UART2_DisableTx();
    DMA_UART2_ClearTxFlag();
    DMA_UART2_ClearOverWriteFlag();
    DMA_UART2_SetTxBusPriority(UART2_DMA_PRIORITY);
    DMA_UART2_SetTxIntPriority(UART2_DMA_PRIORITY);

    g_uart2Baudrate = baudrate;
    UART2_ResetRxDmaEngine();

    return 1U;
}

////////////////////////////////////////
// 串口2初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void UART2_Init(void)
{
    UART2_ConfigurePins();

    DisableGlobalInt();
    g_uart2RxWriteIndex = 0U;
    g_uart2RxReadIndex = 0U;
    g_uart2RxDebugLength = 0U;
    g_uart2RxDebugPending = 0U;
    g_uart2RxOverflow = 0U;
    g_uart2BtAtMode = 0U;
    g_uart2RxTotalCount = 0U;
    g_uart2RxDmaHandledCount = 0U;
    g_uart2RxDmaIdleTicks = 0U;
    g_uart2LastRxByte = 0U;
    g_uart2LastRxValid = 0U;
    EnableGlobalInt();
    (void)UART2_ApplyBaudrate(UART2_BAUDRATE_VALUE);
    UART2_SetBtAtMode(0U);

    //<<AICUBE_USER_UART2_INITIAL_BEGIN>>
    // 在此添加用户初始化代码
    //<<AICUBE_USER_UART2_INITIAL_END>>
}

uint32_t UART2_GetBaudrate(void)
{
    return g_uart2Baudrate;
}

uint8_t UART2_SetBaudrate(uint32_t baudrate)
{
    return UART2_ApplyBaudrate(baudrate);
}

void UART2_SetBtAtMode(uint8_t enable)
{
    if (enable != 0U)
    {
        SET_REG_BIT(P4, UART2_BT_AT_PIN_MASK);
        g_uart2BtAtMode = 1U;
    }
    else
    {
        CLR_REG_BIT(P4, UART2_BT_AT_PIN_MASK);
        g_uart2BtAtMode = 0U;
    }
}

uint8_t UART2_GetBtAtMode(void)
{
    return g_uart2BtAtMode;
}

void UART2_ServiceRx(void)
{
    UART2_ServiceRxDmaInternal();
}

void UART2_ResetRxRing(void)
{
    DisableGlobalInt();
    g_uart2RxWriteIndex = 0U;
    g_uart2RxReadIndex = 0U;
    g_uart2RxDebugLength = 0U;
    g_uart2RxDebugPending = 0U;
    g_uart2RxOverflow = 0U;
    g_uart2LastRxValid = 0U;
    EnableGlobalInt();

    UART2_ResetRxDmaEngine();
}

uint16_t UART2_GetRxTotalCount(void)
{
    UART2_ServiceRxDmaInternal();
    return g_uart2RxTotalCount;
}

uint8_t UART2_GetLastRxByte(uint8_t *rxByte)
{
    UART2_ServiceRxDmaInternal();
    if ((rxByte == 0) || (g_uart2LastRxValid == 0U))
    {
        return 0U;
    }

    *rxByte = g_uart2LastRxByte;
    return 1U;
}

uint8_t UART2_DebugCopyRecentRx(uint8_t *buffer, uint8_t maxLength, uint8_t clearAfterCopy)
{
    uint8_t copyLength;
    uint8_t copyIndex;

    UART2_ServiceRxDmaInternal();

    if ((buffer == 0) || (maxLength == 0U))
    {
        return 0U;
    }

    DisableGlobalInt();
    copyLength = g_uart2RxDebugLength;
    if (copyLength > maxLength)
    {
        copyLength = maxLength;
    }

    for (copyIndex = 0U; copyIndex < copyLength; ++copyIndex)
    {
        buffer[copyIndex] = g_uart2RxDebug[copyIndex];
    }

    if (clearAfterCopy != 0U)
    {
        g_uart2RxDebugLength = 0U;
        g_uart2RxDebugPending = 0U;
    }
    EnableGlobalInt();

    return copyLength;
}

void UART2_DebugResetRecentRx(void)
{
    DisableGlobalInt();
    g_uart2RxDebugLength = 0U;
    g_uart2RxDebugPending = 0U;
    EnableGlobalInt();
}

uint8_t UART2_DebugHasRecentRx(void)
{
    UART2_ServiceRxDmaInternal();
    return g_uart2RxDebugPending;
}

uint8_t UART2_GetRxOverflow(void)
{
    UART2_ServiceRxDmaInternal();
    return g_uart2RxOverflow;
}

uint8_t UART2_TakeRxOverflow(void)
{
    uint8_t overflow;

    UART2_ServiceRxDmaInternal();

    DisableGlobalInt();
    overflow = g_uart2RxOverflow;
    g_uart2RxOverflow = 0U;
    EnableGlobalInt();
    return overflow;
}

uint8_t UART2_TryPopByte(uint8_t *rxByte)
{
    UART2_ServiceRxDmaInternal();

    if ((rxByte == 0) || (g_uart2RxReadIndex == g_uart2RxWriteIndex))
    {
        return 0U;
    }

    DisableGlobalInt();
    *rxByte = g_uart2RxRing[g_uart2RxReadIndex];
    g_uart2RxReadIndex = (uint8_t)((g_uart2RxReadIndex + 1U) % UART2_RX_RING_SIZE);
    EnableGlobalInt();
    return 1U;
}

uint8_t UART2_SendBuffer(const uint8_t *buffer, uint8_t length)
{
    uint8_t txIndex;
    uint16_t timeoutCount;

    if ((buffer == 0) || (length == 0U) || (length > UART2_DMA_TX_BUFFER_SIZE))
    {
        return 0U;
    }

    for (txIndex = 0U; txIndex < length; ++txIndex)
    {
        g_uart2TxDmaBuffer[txIndex] = buffer[txIndex];
    }

    DisableGlobalInt();
    DMA_UART2_DisableTxInt();
    DMA_UART2_DisableTx();
    DMA_UART2_ClearTxFlag();
    DMA_UART2_ClearOverWriteFlag();
    DMA_UART2_SetTxAddress((uint16_t)g_uart2TxDmaBuffer);
    DMA_UART2_SetTxAmount((uint16_t)(length - 1U));
    DMA_UART2_EnableTx();
    DMA_UART2_TriggerTx();
    EnableGlobalInt();

    timeoutCount = 0U;
    while (DMA_UART2_CheckTxFlag() == 0U)
    {
        timeoutCount++;
        if ((timeoutCount >= UART2_TX_READY_TIMEOUT)
            || (UART2_ReadTxDoneStable() >= length))
        {
            break;
        }
    }

    if ((DMA_UART2_CheckTxFlag() == 0U) && (UART2_ReadTxDoneStable() < length))
    {
        DMA_UART2_DisableTx();
        return 0U;
    }

    DMA_UART2_ClearTxFlag();
    if (DMA_UART2_CheckOverWriteFlag() != 0U)
    {
        DMA_UART2_ClearOverWriteFlag();
        DMA_UART2_DisableTx();
        return 0U;
    }

    DMA_UART2_DisableTx();

    return 1U;
}

uint8_t UART2_SendText(const char *text)
{
    uint8_t textLength;

    if (text == 0)
    {
        return 0U;
    }

    textLength = 0U;
    while (text[textLength] != '\0')
    {
        textLength++;
    }

    if (textLength == 0U)
    {
        return 0U;
    }

    return UART2_SendBuffer((const uint8_t *)text, textLength);
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>