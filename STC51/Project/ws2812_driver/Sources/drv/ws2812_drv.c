#include "config.h"
#include "ws2812_drv.h"
#include "hc595_drv.h"

/* PWMAT-DMA control bits. */
#define WS2812DRV_DMA_BASE_CCR1H            0x0D
#define WS2812DRV_DMA_ENPWMAT               (1 << 7)
#define WS2812DRV_PWMAT_TRIG                (1 << 6)
#define WS2812DRV_DMA_PWMATIE               (1 << 7)
#define WS2812DRV_DMA_PWMATIP               (0 << 2)
#define WS2812DRV_DMA_PWMATPTY              2
#define WS2812DRV_DMA_WAIT_LOOP_MAX         60000U
#define WS2812DRV_DMA_ENM2M                 (1 << 7)
#define WS2812DRV_M2M_TRIG                  (1 << 6)
#define WS2812DRV_DMA_M2M_WAIT_LOOP_MAX     20000U
#define WS2812DRV_BUF_ACTIVE                0U
#define WS2812DRV_BUF_BACK                  1U
#define WS2812DRV_LINE_DISCHARGE_US         1U
#define WS2812DRV_M2M_ZERO_BLOCK_BYTES      64U

static uint8_t xdata g_ws2812ImageBuf[2][WS2812DRV_ROW_NUM][WS2812DRV_COL_NUM_MAX][WS2812DRV_PIXEL_CHANNELS];
static uint8_t xdata g_ws2812RowPwmBuf[2][WS2812DRV_ROW_NUM][WS2812DRV_PWM_NUM_MAX];
static uint8_t xdata g_ws2812DualRowPwmBufRaw[WS2812DRV_PWM_NUM_DUAL_MAX + WS2812DRV_DMA_TAIL_GUARD_BYTES + 1];
static uint8_t xdata g_ws2812BitExpandLut[256][8];
static uint8_t xdata g_ws2812M2MZeroBlock[WS2812DRV_M2M_ZERO_BLOCK_BYTES];
static uint8_t xdata *g_ws2812DualRowPwmBuf = 0;
static bit g_ws2812DmaBusy = 0;
static bit g_ws2812ImageDirty = 0;
static bit g_ws2812PwmSwapPending = 0;
static bit g_ws2812FrameFastWrite = 0;
static uint8_t g_ws2812ActivePwmBufIdx = WS2812DRV_BUF_ACTIVE;
static uint8_t g_ws2812PendingPwmBufIdx = WS2812DRV_BUF_BACK;
static uint8_t g_ws2812ScanRowIdx = 0;
static WS2812DRV_DisplayMode_t g_ws2812DisplayMode = WS2812DRV_MODE_16X8;
static WS2812DRV_ScanMode_t g_ws2812ScanMode = WS2812DRV_SCAN_NORMAL_PAIR;
static uint8_t g_ws2812ActiveCols = WS2812DRV_COL_NUM_8;
static uint8_t g_ws2812LastScanRowA = 0U;
static uint8_t g_ws2812LastScanRowB = 1U;
static uint16_t g_ws2812LastScanTxLen = 0U;

extern void Test_DebugMarkRowSwitchStart(void);
extern void Test_DebugMarkPwmSendDone(void);

static bit WS2812DRV_M2MCopy(uint8_t xdata *dst, const uint8_t xdata *src, uint16_t len)
{
	uint16_t waitCnt;
	uint16_t dstAddr;
	uint16_t srcAddr;

	if ((dst == 0) || (src == 0) || (len == 0U))
	{
		return 0;
	}

	DisableGlobalInt();

	DMA_M2M_CR = 0x00;
	DMA_M2M_CFG = 0x00;
	DMA_M2M_STA = 0x00;

	srcAddr = (uint16_t)src;
	dstAddr = (uint16_t)dst;
	DMA_M2M_TXAH = (uint8_t)(srcAddr >> 8);
	DMA_M2M_TXAL = (uint8_t)srcAddr;
	DMA_M2M_RXAH = (uint8_t)(dstAddr >> 8);
	DMA_M2M_RXAL = (uint8_t)dstAddr;
	DMA_M2M_AMTH = (uint8_t)((len - 1U) / 256U);
	DMA_M2M_AMT = (uint8_t)((len - 1U) % 256U);

	DMA_M2M_CR = WS2812DRV_DMA_ENM2M | WS2812DRV_M2M_TRIG;

	EnableGlobalInt();

	waitCnt = 0U;
	while (DMA_M2M_CheckFlag() == 0)
	{
		waitCnt++;
		if (waitCnt >= WS2812DRV_DMA_M2M_WAIT_LOOP_MAX)
		{
			DMA_M2M_CR = 0x00;
			DMA_M2M_STA = 0x00;
			return 0;
		}
	}

	DMA_M2M_STA = 0x00;
	DMA_M2M_CR = 0x00;

	return 1;
}

static bit WS2812DRV_M2MFillZero(uint8_t xdata *dst, uint16_t len)
{
	uint16_t chunk;

	while (len > 0U)
	{
		chunk = len;
		if (chunk > WS2812DRV_M2M_ZERO_BLOCK_BYTES)
		{
			chunk = WS2812DRV_M2M_ZERO_BLOCK_BYTES;
		}

		if (WS2812DRV_M2MCopy(dst, g_ws2812M2MZeroBlock, chunk) == 0)
		{
			return 0;
		}

		dst += chunk;
		len = (uint16_t)(len - chunk);
	}

	return 1;
}

static void WS2812DRV_InitBitExpandLut(void)
{
	uint16_t dat;
	uint8_t bitIdx;

	for (dat = 0U; dat < 256U; dat++)
	{
		for (bitIdx = 0U; bitIdx < 8U; bitIdx++)
		{
			if (((uint8_t)dat & (uint8_t)(0x80U >> bitIdx)) != 0U)
			{
				g_ws2812BitExpandLut[dat][bitIdx] = WS2812DRV_PWM_DUTY_BIT1;
			}
			else
			{
				g_ws2812BitExpandLut[dat][bitIdx] = WS2812DRV_PWM_DUTY_BIT0;
			}
		}
	}
}

static uint16_t WS2812DRV_GetActivePwmNum(void)
{
	return (uint16_t)(WS2812DRV_ROW_RESET_PREFIX_SLOTS + (uint16_t)g_ws2812ActiveCols * 24U + 2U);
}

static void WS2812DRV_BlankOutputs(void)
{
	/* Force both PWM data lines low and close all row switches. */
	WS2812DRV_StopPwmDualChannels();
	HC595_AllOff();
}

static void WS2812DRV_ResetDmaPwmat(void)
{
	DMA_PWMAT_CR = 0x00;
	DMA_PWMAT_CFG = 0x00;
	DMA_PWMAT_STA = 0x00;
	PWMA_DER = 0x00;
	PWMA_DMACR = 0x00;
}

static void WS2812DRV_InitDualRowDmaBuffer(void)
{
	uint16_t addr;

	addr = (uint16_t)g_ws2812DualRowPwmBufRaw;
	if ((addr & 0x0001U) == 0U)
	{
		g_ws2812DualRowPwmBuf = g_ws2812DualRowPwmBufRaw;
	}
	else
	{
		g_ws2812DualRowPwmBuf = g_ws2812DualRowPwmBufRaw + 1;
	}
}

static void WS2812DRV_PWMAConfig(void)
{
	uint8_t ccer1;
	uint8_t ccer2;
	uint8_t ps;
	uint8_t eno;

	PWMA_ENO = 0;
	PWMA_IER = 0;
	PWMA_SR1 = 0;
	PWMA_SR2 = 0;
	ccer1 = 0;
	ccer2 = 0;
	ps = 0;
	eno = 0;

	PWMA_PSCRH = 0;
	PWMA_PSCRL = 0;
	PWMA_DTR = 24;
	PWMA_ARRH = 0;
	PWMA_ARRL = (48 - 1);

	PWMA_CCMR1 = 0x60;
	PWMA_CCR1H = 0;
	PWMA_CCR1L = 0;
	PWMA_CCMR2 = 0x60;
	PWMA_CCR2H = 0;
	PWMA_CCR2L = 0;

	ccer1 |= 0x05;
	ccer1 |= 0x50;
	ps |= 0;
	eno |= 0x05;
	SetP1nPushPullMode(PIN_0 | PIN_2);

	PWMA_CCER1 = ccer1;
	PWMA_CCER2 = ccer2;
	PWMA_PS = ps;
	PWMA_CCMR1 = 0x68;
	PWMA_CCMR2 = 0x68;

	PWMA_BKR = 0x80;
	PWMA_CR1 = 0x81;
	PWMA_EGR = 0x01;
	PWMA_ENO = eno;
}

static void WS2812DRV_EncodeRowToPwmBuffer(uint8_t bufIdx, uint8_t row)
{
	uint16_t pwmIdx;
	uint8_t col;
	uint8_t colorIdx;
	uint8_t dat;
	uint8_t bitIdx;
    uint8_t xdata *lutBits;
    uint8_t activeCols;
    uint16_t activePwmNum;

    activeCols = g_ws2812ActiveCols;
    activePwmNum = WS2812DRV_GetActivePwmNum();

	/* Reserve reset low window before each row payload to improve decoding stability. */
	pwmIdx = WS2812DRV_ROW_RESET_PREFIX_SLOTS;
	for (bitIdx = 0U; bitIdx < WS2812DRV_ROW_RESET_PREFIX_SLOTS; bitIdx++)
	{
		g_ws2812RowPwmBuf[bufIdx][row][bitIdx] = 0;
	}

	for (col = 0; col < activeCols; col++)
	{
		for (colorIdx = 0; colorIdx < WS2812DRV_PIXEL_CHANNELS; colorIdx++)
		{
			dat = g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][colorIdx];
			lutBits = g_ws2812BitExpandLut[dat];
			for (bitIdx = 0U; bitIdx < 8U; bitIdx++)
			{
				g_ws2812RowPwmBuf[bufIdx][row][pwmIdx] = lutBits[bitIdx];
				pwmIdx++;
			}
		}
	}

	for (; pwmIdx < activePwmNum; pwmIdx++)
	{
		g_ws2812RowPwmBuf[bufIdx][row][pwmIdx] = 0;
	}
}

static uint16_t WS2812DRV_BuildDualRowPwmBufferByBufIdx(uint8_t bufIdx, uint8_t rowA, uint8_t rowB)
{
	uint16_t idx;
	uint16_t outIdx;
	uint8_t xdata *dualBuf;
    uint16_t activePwmNum;
	uint8_t tailIdx;

	if ((rowA >= WS2812DRV_ROW_NUM) || (rowB >= WS2812DRV_ROW_NUM))
	{
		return 0;
	}

	activePwmNum = WS2812DRV_GetActivePwmNum();
	dualBuf = g_ws2812DualRowPwmBuf;
	outIdx = 0;
	for (idx = 0; idx < activePwmNum; idx++)
	{
		dualBuf[outIdx] = g_ws2812RowPwmBuf[bufIdx][rowA][idx];
		outIdx++;
		dualBuf[outIdx] = g_ws2812RowPwmBuf[bufIdx][rowB][idx];
		outIdx++;
	}

	/* Append reset-low tail to latch data and allow LEDs to turn off in time. */
	for (tailIdx = 0U; tailIdx < WS2812DRV_RESET_TAIL_SLOTS; tailIdx++)
	{
		dualBuf[outIdx] = 0U;
		outIdx++;
		dualBuf[outIdx] = 0U;
		outIdx++;
	}

	/* Keep one extra CH1/CH2 guard pair for DMA boundary robustness. */
	dualBuf[outIdx] = 0;
	outIdx++;
	dualBuf[outIdx] = 0;
	outIdx++;

	return outIdx;
}

static uint16_t WS2812DRV_BuildDualRowLegacyBufferByBufIdx(uint8_t bufIdx, uint8_t rowOff, uint8_t rowOn)
{
	uint16_t idx;
	uint16_t outIdx;
	uint8_t xdata *dualBuf;
	uint16_t activePwmNum;
	bit dataOnCh0;
	uint8_t offPwm;
	uint8_t tailIdx;

	if ((rowOff >= WS2812DRV_ROW_NUM) || (rowOn >= WS2812DRV_ROW_NUM))
	{
		return 0;
	}

	activePwmNum = WS2812DRV_GetActivePwmNum();
	dualBuf = g_ws2812DualRowPwmBuf;
	dataOnCh0 = (bit)((rowOn & 0x01U) == 0U);
	outIdx = 0;
	for (idx = 0; idx < activePwmNum; idx++)
	{
		/* For off row, send WS2812 bit-0 code in payload region; keep reset regions low level. */
		offPwm = WS2812DRV_PWM_DUTY_BIT0;
		if ((idx < WS2812DRV_ROW_RESET_PREFIX_SLOTS) || (idx >= (activePwmNum - 2U)))
		{
			offPwm = 0U;
		}

		/* Legacy shift mode: keep channel binding fixed by row parity; only data payload changes. */
		if (dataOnCh0 != 0)
		{
			/* rowOn is even: CH0 carries rowOn data, CH2 carries rowOff-off code. */
			dualBuf[outIdx] = g_ws2812RowPwmBuf[bufIdx][rowOn][idx];
			outIdx++;
			dualBuf[outIdx] = offPwm;
			outIdx++;
		}
		else
		{
			/* rowOn is odd: CH2 carries rowOn data, CH0 carries rowOff-off code. */
			dualBuf[outIdx] = offPwm;
			outIdx++;
			dualBuf[outIdx] = g_ws2812RowPwmBuf[bufIdx][rowOn][idx];
			outIdx++;
		}
	}

	/* Append reset-low tail to latch data and allow LEDs to turn off in time. */
	for (tailIdx = 0U; tailIdx < WS2812DRV_RESET_TAIL_SLOTS; tailIdx++)
	{
		dualBuf[outIdx] = 0U;
		outIdx++;
		dualBuf[outIdx] = 0U;
		outIdx++;
	}

	/* Keep one extra CH0/CH2 guard pair for DMA boundary robustness. */
	dualBuf[outIdx] = 0U;
	outIdx++;
	dualBuf[outIdx] = 0U;
	outIdx++;

	return outIdx;
}

void WS2812DRV_Init(void)
{
	uint8_t row;

	EAXFR = 1;
	WTST = 0;
	CKCON = 0;

	WS2812DRV_PWMAConfig();
	WS2812DRV_InitDualRowDmaBuffer();
	WS2812DRV_InitBitExpandLut();
	WS2812DRV_ClearImage();

	for (row = 0; row < WS2812DRV_ROW_NUM; row++)
	{
		WS2812DRV_EncodeRowToPwmBuffer(WS2812DRV_BUF_ACTIVE, row);
		WS2812DRV_EncodeRowToPwmBuffer(WS2812DRV_BUF_BACK, row);
	}

	g_ws2812DmaBusy = 0;
	g_ws2812ImageDirty = 0;
	g_ws2812PwmSwapPending = 0;
	g_ws2812ActivePwmBufIdx = WS2812DRV_BUF_ACTIVE;
	g_ws2812PendingPwmBufIdx = WS2812DRV_BUF_BACK;
	g_ws2812ScanRowIdx = 0;
	g_ws2812ScanMode = WS2812DRV_SCAN_NORMAL_PAIR;
	g_ws2812LastScanRowA = 0U;
	g_ws2812LastScanRowB = 1U;
	g_ws2812LastScanTxLen = 0U;
}

void WS2812DRV_ClearImage(void)
{
	uint8_t row;
	uint8_t col;
	uint8_t colorIdx;
	uint8_t oldVal;
	uint16_t clearLen;
	uint8_t xdata *backImage;

	backImage = &g_ws2812ImageBuf[WS2812DRV_BUF_BACK][0][0][0];
	clearLen = (uint16_t)WS2812DRV_ROW_NUM * (uint16_t)WS2812DRV_COL_NUM_MAX * (uint16_t)WS2812DRV_PIXEL_CHANNELS;
	if (WS2812DRV_M2MFillZero(backImage, clearLen) != 0)
	{
		g_ws2812ImageDirty = 1;
		return;
	}

	for (row = 0; row < WS2812DRV_ROW_NUM; row++)
	{
		for (col = 0; col < WS2812DRV_COL_NUM_MAX; col++)
		{
			for (colorIdx = 0; colorIdx < WS2812DRV_PIXEL_CHANNELS; colorIdx++)
			{
				oldVal = g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][colorIdx];
				if (oldVal != 0U)
				{
					g_ws2812ImageDirty = 1;
				}
				g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][colorIdx] = 0;
			}
		}
	}
}

void WS2812DRV_BeginFrameWrite(void)
{
	g_ws2812FrameFastWrite = 1;
}

void WS2812DRV_SetPixelRgbFast(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
	if ((row >= WS2812DRV_ROW_NUM) || (col >= WS2812DRV_COL_NUM_MAX))
	{
		return;
	}

	g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][0] = g;
	g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][1] = r;
	g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][2] = b;
}

void WS2812DRV_EndFrameWrite(void)
{
	g_ws2812FrameFastWrite = 0;
	g_ws2812ImageDirty = 1;
}

void WS2812DRV_SetPixelRgb(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
	if ((row >= WS2812DRV_ROW_NUM) || (col >= WS2812DRV_COL_NUM_MAX))
	{
		return;
	}

	if (g_ws2812FrameFastWrite != 0)
	{
		WS2812DRV_SetPixelRgbFast(row, col, r, g, b);
		return;
	}

	/* WS2812 transmit order is GRB, and updates always target back image buffer. */
	if (g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][0] != g)
	{
		g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][0] = g;
		g_ws2812ImageDirty = 1;
	}
	if (g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][1] != r)
	{
		g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][1] = r;
		g_ws2812ImageDirty = 1;
	}
	if (g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][2] != b)
	{
		g_ws2812ImageBuf[WS2812DRV_BUF_BACK][row][col][2] = b;
		g_ws2812ImageDirty = 1;
	}
}

void WS2812DRV_EncodeAllRows(void)
{
	uint8_t row;
	uint8_t buildIdx;

	/* Skip build when source frame has no changes. */
	if (g_ws2812ImageDirty == 0)
	{
		return;
	}

	/* Keep only one pending frame to avoid writer/reader overlap. */
	if (g_ws2812PwmSwapPending != 0)
	{
		return;
	}

	buildIdx = (uint8_t)(g_ws2812ActivePwmBufIdx ^ 0x01U);
	for (row = 0; row < WS2812DRV_ROW_NUM; row++)
	{
		WS2812DRV_EncodeRowToPwmBuffer(buildIdx, row);
	}

	DisableGlobalInt();
	g_ws2812PendingPwmBufIdx = buildIdx;
	g_ws2812PwmSwapPending = 1;
	g_ws2812ImageDirty = 0;
	EnableGlobalInt();
}

uint16_t WS2812DRV_BuildDualRowPwmBuffer(uint8_t rowA, uint8_t rowB)
{
	return WS2812DRV_BuildDualRowPwmBufferByBufIdx(g_ws2812ActivePwmBufIdx, rowA, rowB);
}

uint8_t xdata *WS2812DRV_GetDualRowPwmBuffer(void)
{
	return g_ws2812DualRowPwmBuf;
}

void WS2812DRV_SelectRows(uint8_t rowA, uint8_t rowB)
{
	/* Debug timestamp at exact row-switch start. */
	Test_DebugMarkRowSwitchStart();

	/* New policy: full blank before each row-select update. */
	WS2812DRV_BlankOutputs();
	delay_us(WS2812DRV_LINE_DISCHARGE_US);

	HC595_SelectRows(rowA, rowB);
	delay_us(WS2812DRV_ROW_SWITCH_SETTLE_US);
}

void WS2812DRV_TriggerDualRowDma(uint8_t xdata *txBuf, uint16_t num)
{
	uint16_t addr;
	uint16_t alignedNum;

	if (num < 2U)
	{
		return;
	}

	/* DMA stream must keep CH1/CH2 pair alignment. */
	alignedNum = (uint16_t)(num & (uint16_t)~0x0001U);
	if (alignedNum < 2U)
	{
		return;
	}

	DisableGlobalInt();

	WS2812DRV_ResetDmaPwmat();

	PWMA_DBA = WS2812DRV_DMA_BASE_CCR1H;
	PWMA_DBL = 0x01;
	PWMA_DER = 0x01;
	PWMA_DMACR = 0x14;

	addr = (uint16_t)txBuf;
	DMA_PWMAT_TXAH = (uint8_t)(addr >> 8);
	DMA_PWMAT_TXAL = (uint8_t)addr;
	DMA_PWMAT_AMTH = (uint8_t)((alignedNum - 1U) / 256U);
	DMA_PWMAT_AMT = (uint8_t)((alignedNum - 1U) % 256U);

	g_ws2812DmaBusy = 1;

	DMA_PWMAT_CFG = WS2812DRV_DMA_PWMATIE | WS2812DRV_DMA_PWMATIP | WS2812DRV_DMA_PWMATPTY;
	DMA_PWMAT_CR = WS2812DRV_DMA_ENPWMAT | WS2812DRV_PWMAT_TRIG;

	EnableGlobalInt();
}

bit WS2812DRV_WaitDmaDone(void)
{
	uint16_t waitCnt;

	waitCnt = 0;
	while (g_ws2812DmaBusy != 0)
	{
		waitCnt++;
		if (waitCnt >= WS2812DRV_DMA_WAIT_LOOP_MAX)
		{
			DisableGlobalInt();
			WS2812DRV_ResetDmaPwmat();
			WS2812DRV_BlankOutputs();
			g_ws2812DmaBusy = 0;
			EnableGlobalInt();

			return 0;
		}
	}

	return 1;
}

void WS2812DRV_StopPwmDualChannels(void)
{
	PWMA_CCR1H = 0;
	PWMA_CCR1L = 0;
	PWMA_CCR2H = 0;
	PWMA_CCR2L = 0;
}

bit WS2812DRV_SendRowPair(uint8_t rowA, uint8_t rowB)
{
	uint8_t sendRowA;
	uint8_t sendRowB;
	uint8_t temp;
	uint16_t txLen;

	if ((rowA >= WS2812DRV_ROW_NUM) || (rowB >= WS2812DRV_ROW_NUM))
	{
		return 0;
	}

	sendRowA = rowA;
	sendRowB = rowB;
	if ((sendRowA & 0x01U) != 0U)
	{
		temp = sendRowA;
		sendRowA = sendRowB;
		sendRowB = temp;
	}

	txLen = WS2812DRV_BuildDualRowPwmBuffer(sendRowA, sendRowB);
	if (txLen < 2U)
	{
		return 0;
	}

	WS2812DRV_SelectRows(sendRowA, sendRowB);
	WS2812DRV_TriggerDualRowDma(g_ws2812DualRowPwmBuf, txLen);
	if (WS2812DRV_WaitDmaDone() == 0)
	{
		WS2812DRV_BlankOutputs();

		return 0;
	}

	return 1;
}

void WS2812DRV_RefreshStep(void)
{
	uint8_t rowA;
	uint8_t rowB;
	uint16_t txLen;

	if (g_ws2812DmaBusy != 0)
	{
		return;
	}

	if ((g_ws2812ScanRowIdx == 0U) && (g_ws2812PwmSwapPending != 0))
	{
		g_ws2812ActivePwmBufIdx = g_ws2812PendingPwmBufIdx;
		g_ws2812PwmSwapPending = 0;
	}

	if (g_ws2812ScanMode == WS2812DRV_SCAN_LEGACY_SHIFT)
	{
		rowA = g_ws2812ScanRowIdx;
		rowB = (uint8_t)(g_ws2812ScanRowIdx + 1U);
		if (rowB >= WS2812DRV_ROW_NUM)
		{
			rowB = 0U;
		}

		txLen = WS2812DRV_BuildDualRowLegacyBufferByBufIdx(g_ws2812ActivePwmBufIdx, rowA, rowB);
	}
	else
	{
		rowA = g_ws2812ScanRowIdx;
		rowB = (uint8_t)(g_ws2812ScanRowIdx + 1U);
		txLen = WS2812DRV_BuildDualRowPwmBufferByBufIdx(g_ws2812ActivePwmBufIdx, rowA, rowB);
	}

	if (txLen < 2U)
	{
		return;
	}

	WS2812DRV_SelectRows(rowA, rowB);
	g_ws2812LastScanRowA = rowA;
	g_ws2812LastScanRowB = rowB;
	g_ws2812LastScanTxLen = txLen;
	WS2812DRV_TriggerDualRowDma(g_ws2812DualRowPwmBuf, txLen);

	if (g_ws2812ScanMode == WS2812DRV_SCAN_LEGACY_SHIFT)
	{
		g_ws2812ScanRowIdx++;
	}
	else
	{
		g_ws2812ScanRowIdx = (uint8_t)(g_ws2812ScanRowIdx + 2U);
	}

	if (g_ws2812ScanRowIdx >= WS2812DRV_ROW_NUM)
	{
		g_ws2812ScanRowIdx = 0;
	}
}

void WS2812DRV_OnDmaIsr(void)
{
	/* Debug timestamp at PWM+DMA transfer completion. */
	Test_DebugMarkPwmSendDone();

	g_ws2812DmaBusy = 0;
	DMA_PWMAT_STA = 0;
}

bit WS2812DRV_IsDmaBusy(void)
{
	return g_ws2812DmaBusy;
}

uint8_t WS2812DRV_SetDisplayMode(WS2812DRV_DisplayMode_t mode)
{
	uint8_t activeCols;

	if (mode == WS2812DRV_MODE_16X8)
	{
		activeCols = WS2812DRV_COL_NUM_8;
	}
	else if (mode == WS2812DRV_MODE_16X16)
	{
		activeCols = WS2812DRV_COL_NUM_16;
	}
	else
	{
		return 0;
	}

	if ((g_ws2812DisplayMode == mode) && (g_ws2812ActiveCols == activeCols))
	{
		return 1;
	}

	g_ws2812DisplayMode = mode;
	g_ws2812ActiveCols = activeCols;
	g_ws2812ImageDirty = 1;

	return 1;
}

WS2812DRV_DisplayMode_t WS2812DRV_GetDisplayMode(void)
{
	return g_ws2812DisplayMode;
}

uint8_t WS2812DRV_GetActiveCols(void)
{
	return g_ws2812ActiveCols;
}

uint8_t WS2812DRV_SetScanMode(WS2812DRV_ScanMode_t mode)
{
	if ((mode != WS2812DRV_SCAN_NORMAL_PAIR) && (mode != WS2812DRV_SCAN_LEGACY_SHIFT))
	{
		return 0U;
	}

	DisableGlobalInt();
	g_ws2812ScanMode = mode;
	g_ws2812ScanRowIdx = 0U;
	EnableGlobalInt();

	return 1U;
}

WS2812DRV_ScanMode_t WS2812DRV_GetScanMode(void)
{
	return g_ws2812ScanMode;
}

WS2812DRV_ScanMode_t WS2812DRV_ToggleScanMode(void)
{
	if (g_ws2812ScanMode == WS2812DRV_SCAN_NORMAL_PAIR)
	{
		(void)WS2812DRV_SetScanMode(WS2812DRV_SCAN_LEGACY_SHIFT);
	}
	else
	{
		(void)WS2812DRV_SetScanMode(WS2812DRV_SCAN_NORMAL_PAIR);
	}

	return g_ws2812ScanMode;
}

uint8_t WS2812DRV_GetLastScanRowA(void)
{
	return g_ws2812LastScanRowA;
}

uint8_t WS2812DRV_GetLastScanRowB(void)
{
	return g_ws2812LastScanRowB;
}

uint16_t WS2812DRV_GetLastScanTxLen(void)
{
	return g_ws2812LastScanTxLen;
}
