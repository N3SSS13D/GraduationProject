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

static uint8_t xdata g_ws2812ImageBuf[WS2812DRV_ROW_NUM][WS2812DRV_COL_NUM][WS2812DRV_PIXEL_CHANNELS];
static uint8_t xdata g_ws2812RowPwmBuf[WS2812DRV_ROW_NUM][WS2812DRV_PWM_NUM];
static uint8_t xdata g_ws2812DualRowPwmBufRaw[WS2812DRV_PWM_NUM_DUAL + WS2812DRV_DMA_TAIL_GUARD_BYTES + 1];
static uint8_t xdata *g_ws2812DualRowPwmBuf = 0;
static bit g_ws2812DmaBusy = 0;

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

static void WS2812DRV_EncodeRowToPwm(uint8_t row)
{
	uint16_t i;
	uint16_t pwmIdx;
	uint8_t col;
	uint8_t colorIdx;
	uint8_t dat;
	uint8_t bitIdx;

	for (i = 0; i < WS2812DRV_PWM_NUM; i++)
	{
		g_ws2812RowPwmBuf[row][i] = 0;
	}

	pwmIdx = 1;
	for (col = 0; col < WS2812DRV_COL_NUM; col++)
	{
		for (colorIdx = 0; colorIdx < WS2812DRV_PIXEL_CHANNELS; colorIdx++)
		{
			dat = g_ws2812ImageBuf[row][col][colorIdx];
			for (bitIdx = 0; bitIdx < 8; bitIdx++)
			{
				if ((dat & 0x80) != 0)
				{
					g_ws2812RowPwmBuf[row][pwmIdx] = WS2812DRV_PWM_DUTY_BIT1;
				}
				else
				{
					g_ws2812RowPwmBuf[row][pwmIdx] = WS2812DRV_PWM_DUTY_BIT0;
				}

				dat <<= 1;
				pwmIdx++;
			}
		}
	}
}

void WS2812DRV_Init(void)
{
	EAXFR = 1;
	WTST = 0;
	CKCON = 0;

	WS2812DRV_PWMAConfig();
	WS2812DRV_InitDualRowDmaBuffer();
	WS2812DRV_ClearImage();
	WS2812DRV_EncodeAllRows();

	g_ws2812DmaBusy = 0;
}

void WS2812DRV_ClearImage(void)
{
	uint8_t row;
	uint8_t col;

	for (row = 0; row < WS2812DRV_ROW_NUM; row++)
	{
		for (col = 0; col < WS2812DRV_COL_NUM; col++)
		{
			g_ws2812ImageBuf[row][col][0] = 0;
			g_ws2812ImageBuf[row][col][1] = 0;
			g_ws2812ImageBuf[row][col][2] = 0;
		}
	}
}

void WS2812DRV_SetPixelRgb(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
	if ((row >= WS2812DRV_ROW_NUM) || (col >= WS2812DRV_COL_NUM))
	{
		return;
	}

	/* WS2812 transmit order is GRB. */
	g_ws2812ImageBuf[row][col][0] = g;
	g_ws2812ImageBuf[row][col][1] = r;
	g_ws2812ImageBuf[row][col][2] = b;
}

void WS2812DRV_EncodeAllRows(void)
{
	uint8_t row;

	for (row = 0; row < WS2812DRV_ROW_NUM; row++)
	{
		WS2812DRV_EncodeRowToPwm(row);
	}
}

uint16_t WS2812DRV_BuildDualRowPwmBuffer(uint8_t rowA, uint8_t rowB)
{
	uint16_t idx;
	uint16_t outIdx;
	uint8_t xdata *dualBuf;

	if ((rowA >= WS2812DRV_ROW_NUM) || (rowB >= WS2812DRV_ROW_NUM))
	{
		return 0;
	}

	dualBuf = g_ws2812DualRowPwmBuf;
	outIdx = 0;
	for (idx = 0; idx < WS2812DRV_PWM_NUM; idx++)
	{
		dualBuf[outIdx] = g_ws2812RowPwmBuf[rowA][idx];
		outIdx++;
		dualBuf[outIdx] = g_ws2812RowPwmBuf[rowB][idx];
		outIdx++;
	}

	/* Append one CH1/CH2 zero pair as DMA tail guard. */
	dualBuf[outIdx] = 0;
	outIdx++;
	dualBuf[outIdx] = 0;
	outIdx++;

	return outIdx;
}

uint8_t xdata *WS2812DRV_GetDualRowPwmBuffer(void)
{
	return g_ws2812DualRowPwmBuf;
}

void WS2812DRV_SelectRows(uint8_t rowA, uint8_t rowB)
{
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
		WS2812DRV_StopPwmDualChannels();

		return 0;
	}

	WS2812DRV_StopPwmDualChannels();

	return 1;
}

void WS2812DRV_OnDmaIsr(void)
{
	g_ws2812DmaBusy = 0;
	DMA_PWMAT_STA = 0;
}

bit WS2812DRV_IsDmaBusy(void)
{
	return g_ws2812DmaBusy;
}
