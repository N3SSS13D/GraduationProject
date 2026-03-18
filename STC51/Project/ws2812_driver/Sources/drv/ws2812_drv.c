#include "config.h"
#include "ws2812_drv.h"
#include "hc595_drv.h"

typedef enum
{
	WS2812DRV_SCAN_IDLE = 0,
	WS2812DRV_SCAN_DMA_WAIT,
	WS2812DRV_SCAN_HOLD
} WS2812DRV_ScanState_t;

static uint8_t xdata g_ws2812Rgb[WS2812DRV_PANEL_HEIGHT][WS2812DRV_PANEL_WIDTH][WS2812DRV_COLORS_PER_LED];
static uint8_t g_scanPairIndex = 0;
static uint8_t g_scanTickFlag = 0;
static uint8_t g_scanHoldTick = 0;
static uint8_t g_forceRefresh = 1;
static uint8_t g_fadePhase = 0;
static uint8_t g_hwInited = 0;
static WS2812DRV_ScanState_t g_scanState = WS2812DRV_SCAN_IDLE;

static void WS2812DRV_StartPwmDmaOutput(void)
{
	PWM_UpdateDuty(PWMA_CH1, 0);
	PWM_UpdateDuty(PWMA_CH2, 0);
	PWMA_GenerateUpdateEvent();
	PWMA_SetCounter(0);
	PWMA_EnableMainOutput();
	PWMA_Run();
}

static void WS2812DRV_StopPwmDmaOutput(void)
{
	PWMA_Stop();
	PWMA_DisableMainOutput();
	SetP1nInitLevelLow(PIN_0 | PIN_2);
}

static uint8_t WS2812DRV_ApplyFade(uint8_t value, uint8_t gain)
{
	uint16_t scaled;

	scaled = (uint16_t)value * (uint16_t)gain;
	scaled = scaled / 255;

	return (uint8_t)scaled;
}

static void WS2812DRV_LoadRowSymbolPair(uint8_t rowA, uint8_t rowB)
{
	uint8_t col;
	uint8_t colorIdx;
	uint8_t bitIdx;
	uint8_t dataA;
	uint8_t dataB;
	uint8_t gainA;
	uint8_t gainB;
	uint16_t txIdx;

	gainA = g_fadePhase;
	gainB = (uint8_t)(255 - g_fadePhase);

	pu8PWMADMATxBuffer[0] = 0;
	pu8PWMADMATxBuffer[1] = 0;
	txIdx = 2;

	for (col = 0; col < WS2812DRV_PANEL_WIDTH; col++)
	{
		for (colorIdx = 0; colorIdx < WS2812DRV_COLORS_PER_LED; colorIdx++)
		{
			dataA = g_ws2812Rgb[rowA][col][colorIdx];
			dataB = g_ws2812Rgb[rowB][col][colorIdx];

			if (colorIdx == 0)
			{
				dataA = WS2812DRV_ApplyFade(dataA, gainA);
				dataB = WS2812DRV_ApplyFade(dataB, gainB);
			}

			for (bitIdx = 0; bitIdx < 8; bitIdx++)
			{
				pu8PWMADMATxBuffer[txIdx] = ((dataA & 0x80) != 0) ? WS2812DRV_PWM_DUTY_BIT1 : WS2812DRV_PWM_DUTY_BIT0;
				pu8PWMADMATxBuffer[txIdx + 1] = ((dataB & 0x80) != 0) ? WS2812DRV_PWM_DUTY_BIT1 : WS2812DRV_PWM_DUTY_BIT0;
				dataA <<= 1;
				dataB <<= 1;
				txIdx += 2;
			}
		}
	}

	pu8PWMADMATxBuffer[txIdx] = 0;
	pu8PWMADMATxBuffer[txIdx + 1] = 0;
}

static void WS2812DRV_LoadSingleRowSymbol(uint8_t row)
{
	uint8_t col;
	uint8_t colorIdx;
	uint8_t bitIdx;
	uint8_t dataA;
	uint16_t txIdx;

	pu8PWMADMATxBuffer[0] = 0;
	pu8PWMADMATxBuffer[1] = 0;
	txIdx = 2;

	for (col = 0; col < WS2812DRV_PANEL_WIDTH; col++)
	{
		for (colorIdx = 0; colorIdx < WS2812DRV_COLORS_PER_LED; colorIdx++)
		{
			dataA = g_ws2812Rgb[row][col][colorIdx];
			for (bitIdx = 0; bitIdx < 8; bitIdx++)
			{
				pu8PWMADMATxBuffer[txIdx] = ((dataA & 0x80) != 0) ? WS2812DRV_PWM_DUTY_BIT1 : WS2812DRV_PWM_DUTY_BIT0;
				pu8PWMADMATxBuffer[txIdx + 1] = 0;
				dataA <<= 1;
				txIdx += 2;
			}
		}
	}

	pu8PWMADMATxBuffer[txIdx] = 0;
	pu8PWMADMATxBuffer[txIdx + 1] = 0;
}

static void WS2812DRV_TriggerRowPairSend(uint8_t pairIdx)
{
	uint8_t rowA;
	uint8_t rowB;

	rowA = (uint8_t)(pairIdx * 2);
	rowB = (uint8_t)(rowA + 1);

	HC595_AllOff();
	delay_us(WS2812DRV_PMOS_OFF_TO_ON_US);
	HC595_SelectRows(rowA, rowB);
	delay_us(WS2812DRV_ROW_SWITCH_SETTLE_US);

	WS2812DRV_LoadRowSymbolPair(rowA, rowB);
	WS2812DRV_StartPwmDmaOutput();
	DMA_PWMA_SetTxAmount(WS2812DRV_TX_BYTE_COUNT - 1);
	DMA_PWMA_ClearTxFlag();
	DMA_PWMA_TriggerTx();
}

void WS2812DRV_Init(void)
{
	uint8_t row;
	uint8_t col;

	if (g_hwInited != 0)
	{
		return;
	}

	HC595_Init();
	HC595_AllOff();
	WS2812DRV_StopPwmDmaOutput();

	for (row = 0; row < WS2812DRV_PANEL_HEIGHT; row++)
	{
		for (col = 0; col < WS2812DRV_PANEL_WIDTH; col++)
		{
			g_ws2812Rgb[row][col][0] = 0;
			g_ws2812Rgb[row][col][1] = 0;
			g_ws2812Rgb[row][col][2] = 0;
		}
	}

	g_scanPairIndex = 0;
	g_scanTickFlag = 0;
	g_scanHoldTick = 0;
	g_scanState = WS2812DRV_SCAN_IDLE;
	g_forceRefresh = 1;
	g_fadePhase = 0;
	g_hwInited = 1;
}

void WS2812DRV_ClearAll(void)
{
	uint8_t row;
	uint8_t col;

	for (row = 0; row < WS2812DRV_PANEL_HEIGHT; row++)
	{
		for (col = 0; col < WS2812DRV_PANEL_WIDTH; col++)
		{
			g_ws2812Rgb[row][col][0] = 0;
			g_ws2812Rgb[row][col][1] = 0;
			g_ws2812Rgb[row][col][2] = 0;
		}
	}

	g_forceRefresh = 1;
}

void WS2812DRV_SetPixelRgb(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
	if ((x >= WS2812DRV_PANEL_WIDTH) || (y >= WS2812DRV_PANEL_HEIGHT))
	{
		return;
	}

	g_ws2812Rgb[y][x][0] = g;
	g_ws2812Rgb[y][x][1] = r;
	g_ws2812Rgb[y][x][2] = b;
	g_forceRefresh = 1;
}

void WS2812DRV_Scan1msHook(void)
{
	g_scanTickFlag = 1;
}

void WS2812DRV_ForceRefreshAll(void)
{
	g_forceRefresh = 1;
}

uint8_t WS2812DRV_IsBusy(void)
{
	return (g_scanState != WS2812DRV_SCAN_IDLE) ? 1 : 0;
}

void WS2812DRV_TaskService(void)
{
	if ((g_hwInited == 0) || ((g_scanTickFlag == 0) && (g_scanState == WS2812DRV_SCAN_IDLE) && (g_forceRefresh == 0)))
	{
		return;
	}

	if (g_scanState == WS2812DRV_SCAN_IDLE)
	{
		g_scanTickFlag = 0;
		WS2812DRV_TriggerRowPairSend(g_scanPairIndex);
		g_scanState = WS2812DRV_SCAN_DMA_WAIT;

		return;
	}

	if (g_scanState == WS2812DRV_SCAN_DMA_WAIT)
	{
		if (DMA_PWMA_CheckTxFlag() == 0)
		{
			return;
		}

		DMA_PWMA_ClearTxFlag();
		WS2812DRV_StopPwmDmaOutput();
		g_scanState = WS2812DRV_SCAN_HOLD;
		g_scanHoldTick = 0;

		return;
	}

	if (g_scanState == WS2812DRV_SCAN_HOLD)
	{
		if (g_scanTickFlag == 0)
		{
			return;
		}

		g_scanTickFlag = 0;
		g_scanHoldTick++;
		if (g_scanHoldTick < WS2812DRV_SCAN_HOLD_TICKS)
		{
			return;
		}

		HC595_AllOff();
		g_scanPairIndex++;
		if (g_scanPairIndex >= WS2812DRV_ROW_PAIRS)
		{
			g_scanPairIndex = 0;
			g_fadePhase = (uint8_t)(g_fadePhase + WS2812DRV_FADE_PHASE_STEP);
			g_forceRefresh = 0;
		}
		g_scanState = WS2812DRV_SCAN_IDLE;
	}
}

void WS2812DRV_ShowRowRedCount(uint8_t row, uint8_t ledCount)
{
	uint8_t col;

	if (row >= WS2812DRV_PANEL_HEIGHT)
	{
		return;
	}

	if (ledCount > WS2812DRV_PANEL_WIDTH)
	{
		ledCount = WS2812DRV_PANEL_WIDTH;
	}

	WS2812DRV_ClearAll();
	for (col = 0; col < ledCount; col++)
	{
		WS2812DRV_SetPixelRgb(col, row, WS2812DRV_TEST_RED_LEVEL, 0, 0);
	}

	HC595_AllOff();
	delay_us(WS2812DRV_PMOS_OFF_TO_ON_US);
	HC595_SelectRows(row, 0xFF);
	delay_us(WS2812DRV_ROW_SWITCH_SETTLE_US);

	WS2812DRV_LoadSingleRowSymbol(row);
	WS2812DRV_StartPwmDmaOutput();
	DMA_PWMA_SetTxAmount(WS2812DRV_TX_BYTE_COUNT - 1);
	DMA_PWMA_ClearTxFlag();
	DMA_PWMA_TriggerTx();
	while (DMA_PWMA_CheckTxFlag() == 0)
	{
	}
	DMA_PWMA_ClearTxFlag();

	WS2812DRV_StopPwmDmaOutput();
	HC595_AllOff();
	delay_us(WS2812DRV_RESET_HOLD_US);
}
