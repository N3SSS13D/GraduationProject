#include "config.h"
#include "hc595_drv.h"
#include "test.h"

#define TEST_LED_NUM                     64
#define TEST_LED_NUM_COMPACT             8
#define TEST_ACTIVE_LED_NUM              8
#define TEST_ROW_NUM                     16
#define TEST_PIXEL_CHANNELS              3
#define TEST_PATTERN_COUNT               3
#define TEST_PATTERN_SIZE                8
#define TEST_PWM_NUM                     (TEST_LED_NUM * 24 + 2)
#define TEST_PWM_DUTY_BIT0               12
#define TEST_PWM_DUTY_BIT1               36
#define TEST_FRAME_DELAY_MS_DEFAULT      500
#define TEST_ROW_TIMER_US_DEFAULT        2000UL
#define TEST_TIMER_US_PRESCALE           39
#define TEST_DMA_BASE_CCR1H              0x0D
#define TEST_DMA_BASE_CCR2H              0x0E
#define TEST_PWMA_ENO_C1P                0x01
#define TEST_PWMA_ENO_C2P                0x04
#define TEST_PWMA_ENO_ALL                (TEST_PWMA_ENO_C1P | TEST_PWMA_ENO_C2P)
#define TEST_FRAME_DELAY_MS_MIN          0
#define TEST_FRAME_DELAY_MS_MAX          999
#define TEST_FRAME_DELAY_US_MIN          1UL
#define TEST_FRAME_DELAY_US_MAX          999000000UL
#define TEST_BG_FILL_BLUE                64

/* PWMAT-DMA控制寄存器位定义。 */
#define TEST_DMA_ENPWMAT                 (1 << 7)
#define TEST_PWMAT_TRIG                  (1 << 6)

/* PWMAT-DMA配置寄存器位定义。 */
#define TEST_DMA_PWMATIE                 (1 << 7)
#define TEST_DMA_PWMATIP                 (0 << 2)
#define TEST_DMA_PWMATPTY                2

static uint8_t xdata g_testLedRgb[TEST_LED_NUM][3];
static uint8_t xdata g_testRowPwmBuf[TEST_ROW_NUM][TEST_PWM_NUM];
static uint8_t xdata g_testSolidPwmBuf[TEST_PWM_NUM];
static uint8_t xdata g_testImageBuf[TEST_ROW_NUM][TEST_LED_NUM][TEST_PIXEL_CHANNELS];
static bit g_testPwmDmaBusy;
static uint8_t g_testRowIndex;
static uint8_t g_testLastRowIndex;
static uint8_t g_testLedCount = TEST_ACTIVE_LED_NUM;
static uint8_t g_testDisplayMethod = TEST_DISPLAY_METHOD_SINGLE;
static uint8_t g_testScanScheme = TEST_SCAN_SCHEME_CLASSIC;
static uint8_t g_testRenderMode = TEST_RENDER_MODE_16X64;
static uint8_t g_testActiveLedNum = TEST_LED_NUM;
static uint8_t g_testReverseDir = 0;
static uint8_t g_testPatternIndex = 0;
static uint8_t g_testColorRed = 64;
static uint8_t g_testColorGreen = 0;
static uint8_t g_testColorBlue = 0;
static uint16_t g_testFrameDelayMs = TEST_FRAME_DELAY_MS_DEFAULT;
static uint32_t g_testFrameDelayUs = TEST_ROW_TIMER_US_DEFAULT;
static uint16_t g_testRowStartUs[TEST_ROW_NUM];
static uint16_t g_testPwmCfgDoneUs[TEST_ROW_NUM];
static uint16_t g_testRowDoneUs[TEST_ROW_NUM];
static uint8_t g_testRowScanOrder[TEST_ROW_NUM];
static volatile bit g_testKeyInt0Pending = 0;
static volatile bit g_testKeyInt1Pending = 0;
static volatile bit g_testRowTimerPending = 1;
static bit g_testTimeLogEnabled = 1;
static bit g_testScanStarted = 0;
static bit g_testFrameProfileActive = 0;
static uint8_t g_testFrameRowCounter = 0;
static bit g_testPwmCacheDirty = 1;
static bit g_testSolidPwmDirty = 1;
static uint16_t g_testPwmDmaLen = TEST_PWM_NUM;

static const uint8_t g_testPatternMask[TEST_PATTERN_COUNT][TEST_PATTERN_SIZE] = {
	{0x18, 0x3C, 0x7E, 0xFF, 0x18, 0x18, 0x18, 0x18},
	{0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},
	{0xFF, 0x81, 0xBD, 0xA5, 0xA5, 0xBD, 0x81, 0xFF}
};

static void Test_OnRowTimerExpired(void)
{
	g_testRowTimerPending = 1;
}

static void Test_UpdateRenderModeParams(void)
{
	/* Render mode changes both visible LED columns and WS2812 DMA payload length. */
	if (g_testRenderMode == TEST_RENDER_MODE_16X8)
	{
		g_testActiveLedNum = TEST_LED_NUM_COMPACT;
	}
	else
	{
		g_testActiveLedNum = TEST_LED_NUM;
	}

	g_testPwmDmaLen = (uint16_t)(g_testActiveLedNum * 24U + 2U);
	/* Force row/single-color PWM cache rebuild after mode switch. */
	g_testPwmCacheDirty = 1;
	g_testSolidPwmDirty = 1;
}

static void Test_ProcessPendingKeyEvent(void)
{
	if (g_testKeyInt0Pending != 0)
	{
		g_testKeyInt0Pending = 0;
		if (g_testScanScheme == TEST_SCAN_SCHEME_CLASSIC)
		{
			g_testScanScheme = TEST_SCAN_SCHEME_QUAD;
			printf("[KEY P32] scheme=quad\r\n");
		}
		else
		{
			g_testScanScheme = TEST_SCAN_SCHEME_CLASSIC;
			printf("[KEY P32] scheme=classic\r\n");
		}
	}

	if (g_testKeyInt1Pending != 0)
	{
		g_testKeyInt1Pending = 0;
		g_testTimeLogEnabled = (g_testTimeLogEnabled == 0) ? 1 : 0;
		printf("[KEY P33] timing log %s\r\n", (g_testTimeLogEnabled != 0) ? "on" : "off");
	}
}

static uint8_t Test_WrapRow(uint8_t row)
{
	while (row >= TEST_ROW_NUM)
	{
		row = (uint8_t)(row - TEST_ROW_NUM);
	}

	return row;
}

static void Test_AdvanceRow(void)
{
	g_testLastRowIndex = g_testRowIndex;
	if (g_testReverseDir == 0)
	{
		g_testRowIndex++;
		if (g_testRowIndex >= TEST_ROW_NUM)
		{
			g_testRowIndex = 0;
		}
	}
	else
	{
		if (g_testRowIndex == 0)
		{
			g_testRowIndex = (TEST_ROW_NUM - 1);
		}
		else
		{
			g_testRowIndex--;
		}
	}
}

static void Test_Timer1Config(void)
{
	TIMER1_Stop();
	TIMER1_TimerMode();
	TIMER1_1TMode();
	TIMER1_Mode0();
	TIMER1_SetPrescale(TEST_TIMER_US_PRESCALE);
	TIMER1_DisableInt();
	TIMER1_SetReload16(0);
	TIMER1_ClearFlag();
}

static void Test_Timer1Start(void)
{
	DisableGlobalInt();
	TIMER1_Stop();
	TIMER1_SetReload16(0);
	TIMER1_ClearFlag();
	TIMER1_Run();
	EnableGlobalInt();
}

static uint16_t Test_Timer1StopGetUs(void)
{
	uint16_t elapsed;

	DisableGlobalInt();
	TIMER1_Stop();
	elapsed = ((uint16_t)TH1 << 8) | TL1;
	if (TIMER1_CheckFlag() != 0)
	{
		elapsed = 0xFFFF;
		TIMER1_ClearFlag();
	}
	EnableGlobalInt();

	return elapsed;
}

static uint16_t Test_Timer1GetNowUs(void)
{
	uint16_t elapsed;

	DisableGlobalInt();
	elapsed = ((uint16_t)TH1 << 8) | TL1;
	if (TIMER1_CheckFlag() != 0)
	{
		elapsed = 0xFFFF;
	}
	EnableGlobalInt();

	return elapsed;
}

static void Test_SelectPwmChannelByRow(uint8_t row)
{
	if ((row & 0x01) == 0)
	{
		/* 偶数行使用CH1，对应P10。 */
		PWMA_DBA = TEST_DMA_BASE_CCR1H;
	}
	else
	{
		/* 奇数行使用CH2，对应P12。 */
		PWMA_DBA = TEST_DMA_BASE_CCR2H;
	}
}

static void Test_SetZeroPwmOnRowChannel(uint8_t row)
{
	if ((row & 0x01) == 0)
	{
		PWMA_CCR1H = 0;
		PWMA_CCR1L = TEST_PWM_DUTY_BIT0;
	}
	else
	{
		PWMA_CCR2H = 0;
		PWMA_CCR2L = TEST_PWM_DUTY_BIT0;
	}
}

static void Test_StopPwmOnRowChannel(uint8_t row)
{
	if ((row & 0x01) == 0)
	{
		PWMA_CCR1H = 0;
		PWMA_CCR1L = 0;
	}
	else
	{
		PWMA_CCR2H = 0;
		PWMA_CCR2L = 0;
	}
}

static void Test_BuildPatternImage(uint8_t patternIdx)
{
	uint8_t row;
	uint8_t col;
	uint8_t rowMask;
	uint8_t bitMask;

	if (patternIdx >= TEST_PATTERN_COUNT)
	{
		patternIdx = 0;
	}

	g_testPatternIndex = patternIdx;

	for (row = 0; row < TEST_ROW_NUM; row++)
	{
		if (row < TEST_PATTERN_SIZE)
		{
			rowMask = g_testPatternMask[g_testPatternIndex][row];
		}
		else
		{
			rowMask = 0;
		}

		for (col = 0; col < TEST_LED_NUM; col++)
		{
			if (col < TEST_PATTERN_SIZE)
			{
				bitMask = (uint8_t)(1U << (TEST_PATTERN_SIZE - 1U - col));
				if ((rowMask & bitMask) != 0)
				{
					g_testImageBuf[row][col][0] = g_testColorGreen;
					g_testImageBuf[row][col][1] = g_testColorRed;
					g_testImageBuf[row][col][2] = g_testColorBlue;
				}
				else
				{
					g_testImageBuf[row][col][0] = 0;
					g_testImageBuf[row][col][1] = 0;
					g_testImageBuf[row][col][2] = TEST_BG_FILL_BLUE;
				}
			}
			else
			{
				g_testImageBuf[row][col][0] = 0;
				g_testImageBuf[row][col][1] = 0;
				g_testImageBuf[row][col][2] = TEST_BG_FILL_BLUE;
			}
		}
	}
	g_testPwmCacheDirty = 1;
}

static void Test_RebuildSolidPwmCache(void)
{
	uint8_t colorIdx;
	uint8_t ledIdx;
	uint16_t i;
	uint16_t pwmIdx;
	uint8_t k;
	uint8_t dat;
	uint8_t wsByte;

	for (i = 0; i < g_testPwmDmaLen; i++)
	{
		g_testSolidPwmBuf[i] = 0;
	}

	pwmIdx = 1;
	for (ledIdx = 0; ledIdx < g_testActiveLedNum; ledIdx++)
	{
		for (colorIdx = 0; colorIdx < 3; colorIdx++)
		{
			if (colorIdx == 0)
			{
				wsByte = g_testColorGreen;
			}
			else if (colorIdx == 1)
			{
				wsByte = g_testColorRed;
			}
			else
			{
				wsByte = g_testColorBlue;
			}

			dat = wsByte;
			for (k = 0; k < 8; k++)
			{
				if ((dat & 0x80) != 0)
				{
					g_testSolidPwmBuf[pwmIdx] = TEST_PWM_DUTY_BIT1;
				}
				else
				{
					g_testSolidPwmBuf[pwmIdx] = TEST_PWM_DUTY_BIT0;
				}
				dat <<= 1;
				pwmIdx++;
			}
		}
	}

	g_testSolidPwmDirty = 0;
}

static void Test_SelectRows4(uint8_t row0, uint8_t row1, uint8_t row2, uint8_t row3)
{
	uint16_t value;

	value = 0xFFFF;
	if (row0 < TEST_ROW_NUM)
	{
		value &= ~((uint16_t)1 << row0);
	}
	if (row1 < TEST_ROW_NUM)
	{
		value &= ~((uint16_t)1 << row1);
	}
	if (row2 < TEST_ROW_NUM)
	{
		value &= ~((uint16_t)1 << row2);
	}
	if (row3 < TEST_ROW_NUM)
	{
		value &= ~((uint16_t)1 << row3);
	}

	HC595_Write16(value);
}

static void Test_EncodeRowToPwm(uint8_t row, uint8_t xdata *pPwm)
{
	uint8_t colorIdx;
	uint8_t ledIdx;
	uint8_t grb;
	uint16_t i;
	uint16_t pwmIdx;
	uint8_t k;
	uint8_t dat;
	uint8_t wsByte;

	for (i = 0; i < g_testPwmDmaLen; i++)
	{
		pPwm[i] = 0;
	}

	pwmIdx = 1;
	for (ledIdx = 0; ledIdx < g_testActiveLedNum; ledIdx++)
	{
		for (colorIdx = 0; colorIdx < 3; colorIdx++)
		{
			grb = colorIdx;
			if (grb == 0)
			{
				wsByte = g_testImageBuf[row][ledIdx][0];
			}
			else if (grb == 1)
			{
				wsByte = g_testImageBuf[row][ledIdx][1];
			}
			else
			{
				wsByte = g_testImageBuf[row][ledIdx][2];
			}

			dat = wsByte;
			for (k = 0; k < 8; k++)
			{
				if ((dat & 0x80) != 0)
				{
					pPwm[pwmIdx] = TEST_PWM_DUTY_BIT1;
				}
				else
				{
					pPwm[pwmIdx] = TEST_PWM_DUTY_BIT0;
				}
				dat <<= 1;
				pwmIdx++;
			}
		}
	}
}

static void Test_RebuildPwmCache(void)
{
	uint8_t row;

	for (row = 0; row < TEST_ROW_NUM; row++)
	{
		Test_EncodeRowToPwm(row, &g_testRowPwmBuf[row][0]);
	}
	g_testPwmCacheDirty = 0;
}

static void Test_WaitDmaDone(void)
{
	while (g_testPwmDmaBusy != 0)
	{
	}
}

static void Test_PWMAConfig(void)
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
	eno |= TEST_PWMA_ENO_ALL;
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

static void Test_PWMATDmaTrig(uint8_t xdata *txBuf, uint16_t num)
{
	uint16_t addr;

	if (num == 0)
	{
		return;
	}

	PWMA_DBL = 0x00;
	PWMA_DER = 0x01;
	PWMA_DMACR = 0x14;

	addr = (uint16_t)txBuf;
	DMA_PWMAT_TXAH = (uint8_t)(addr >> 8);
	DMA_PWMAT_TXAL = (uint8_t)addr;
	DMA_PWMAT_AMTH = (uint8_t)((num - 1) / 256);
	DMA_PWMAT_AMT = (uint8_t)((num - 1) % 256);
	DMA_PWMAT_STA = 0x00;
	DMA_PWMAT_CFG = TEST_DMA_PWMATIE | TEST_DMA_PWMATIP | TEST_DMA_PWMATPTY;
	DMA_PWMAT_CR = TEST_DMA_ENPWMAT | TEST_PWMAT_TRIG;
	g_testPwmDmaBusy = 1;
}

void Test_Init(void)
{
	uint16_t i;
	uint8_t xdata *pRgb;

	EAXFR = 1;
	WTST = 0;
	CKCON = 0;

	Test_PWMAConfig();
	Test_Timer1Config();
	g_testPwmDmaBusy = 0;
	g_testRowIndex = 0;
	g_testLastRowIndex = (TEST_ROW_NUM - 1);
	g_testDisplayMethod = TEST_DISPLAY_METHOD_SINGLE;
	g_testFrameDelayMs = TEST_FRAME_DELAY_MS_DEFAULT;
	g_testFrameDelayUs = TEST_ROW_TIMER_US_DEFAULT;
	g_testRowTimerPending = 1;
	g_testTimeLogEnabled = 1;
	g_testScanScheme = TEST_SCAN_SCHEME_CLASSIC;
	g_testRenderMode = TEST_RENDER_MODE_16X64;
	Test_UpdateRenderModeParams();
	g_testScanStarted = 0;
	g_testFrameProfileActive = 0;
	g_testFrameRowCounter = 0;
	TIMER0_RegisterUsHook(Test_OnRowTimerExpired);
	Test_SelectPwmChannelByRow(g_testRowIndex);
	HC595_SelectRows(g_testRowIndex, 0xFF);
	Test_BuildPatternImage(0);

	pRgb = &g_testLedRgb[0][0];
	for (i = 0; i < (uint16_t)(TEST_LED_NUM * 3); i++)
	{
		*pRgb = 0;
		pRgb++;
	}

	for (i = 0; i < TEST_ROW_NUM; i++)
	{
		g_testRowStartUs[i] = 0;
		g_testPwmCfgDoneUs[i] = 0;
		g_testRowDoneUs[i] = 0;
		g_testRowScanOrder[i] = 0;
	}
}

void Test_TaskLoop(void)
{
	uint16_t totalUs;
	uint16_t cfgDurUs;
	uint16_t rowDurUs;
	uint8_t idx;

	Test_ProcessPendingKeyEvent();

	if (g_testPwmDmaBusy != 0)
	{
		return;
	}

	if ((g_testScanStarted != 0) && (g_testRowTimerPending == 0))
	{
		return;
	}

	if (g_testScanStarted != 0)
	{
		g_testRowTimerPending = 0;
		Test_AdvanceRow();
	}
	else
	{
		g_testRowTimerPending = 0;
	}

	if (g_testPwmCacheDirty != 0)
	{
		Test_RebuildPwmCache();
	}
	if (g_testSolidPwmDirty != 0)
	{
		Test_RebuildSolidPwmCache();
	}

	if (g_testFrameProfileActive == 0)
	{
		Test_Timer1Start();
		g_testFrameProfileActive = 1;
		g_testFrameRowCounter = 0;
	}

	g_testRowScanOrder[g_testFrameRowCounter] = g_testRowIndex;
	g_testRowStartUs[g_testFrameRowCounter] = Test_Timer1GetNowUs();

	if (g_testScanScheme == TEST_SCAN_SCHEME_CLASSIC)
	{
		/* 旧方案：两行窗口，当前行显示图案，上一行固定0码清除。 */
		HC595_SelectRows(g_testRowIndex, g_testLastRowIndex);
		Test_SetZeroPwmOnRowChannel(g_testLastRowIndex);
		Test_SelectPwmChannelByRow(g_testRowIndex);
		Test_PWMATDmaTrig(&g_testRowPwmBuf[g_testRowIndex][0], g_testPwmDmaLen);
	}
	else
	{
		uint8_t row0;
		uint8_t row1;
		uint8_t row2;
		uint8_t row3;

		/* 新方案：四行窗口，row1/row3显示纯色，row0/row2固定0码。 */
		row0 = g_testRowIndex;
		row1 = Test_WrapRow((uint8_t)(g_testRowIndex + 1));
		row2 = Test_WrapRow((uint8_t)(g_testRowIndex + 2));
		row3 = Test_WrapRow((uint8_t)(g_testRowIndex + 3));

		Test_SelectRows4(row0, row1, row2, row3);
		Test_SetZeroPwmOnRowChannel(row0);
		Test_SelectPwmChannelByRow(row1);
		Test_PWMATDmaTrig(g_testSolidPwmBuf, g_testPwmDmaLen);
	}
	g_testPwmCfgDoneUs[g_testFrameRowCounter] = Test_Timer1GetNowUs();
	Test_WaitDmaDone();
	if (g_testScanScheme == TEST_SCAN_SCHEME_CLASSIC)
	{
		Test_StopPwmOnRowChannel(g_testLastRowIndex);
	}
	else
	{
		Test_StopPwmOnRowChannel(g_testRowIndex);
	}
	g_testRowDoneUs[g_testFrameRowCounter] = Test_Timer1GetNowUs();
	TIMER0_StartOneShotUs(g_testFrameDelayUs);

	g_testFrameRowCounter++;
	if (g_testFrameRowCounter >= TEST_ROW_NUM)
	{
		totalUs = Test_Timer1StopGetUs();
		if (g_testTimeLogEnabled != 0)
		{
			printf("[FRAME] rows=%u leds=%u mode=%u pat=%u total=%u us\r\n", (unsigned int)TEST_ROW_NUM,
				(unsigned int)g_testActiveLedNum, (unsigned int)g_testRenderMode,
				(unsigned int)g_testPatternIndex, (unsigned int)totalUs);
			for (idx = 0; idx < TEST_ROW_NUM; idx++)
			{
				cfgDurUs = (uint16_t)(g_testPwmCfgDoneUs[idx] - g_testRowStartUs[idx]);
				rowDurUs = (uint16_t)(g_testRowDoneUs[idx] - g_testRowStartUs[idx]);
				printf("[ROW %u] row=%u tStart=%u tCfgDone=%u tDone=%u cfgDur=%u rowDur=%u us\r\n",
					(unsigned int)idx,
					(unsigned int)g_testRowScanOrder[idx],
					(unsigned int)g_testRowStartUs[idx],
					(unsigned int)g_testPwmCfgDoneUs[idx],
					(unsigned int)g_testRowDoneUs[idx],
					(unsigned int)cfgDurUs,
					(unsigned int)rowDurUs);
			}
		}

		g_testFrameProfileActive = 0;
		g_testFrameRowCounter = 0;
	}

	g_testScanStarted = 1;

	/* 基准测试阶段不插入任何额外延时，仅统计完整16x64扫描时长。 */
}

void Test_RequestLedCount(uint8_t ledCount)
{
	if (ledCount == 0)
	{
		g_testLedCount = 1;

		return;
	}

	if (ledCount > TEST_ACTIVE_LED_NUM)
	{
		g_testLedCount = TEST_ACTIVE_LED_NUM;

		return;
	}

	g_testLedCount = ledCount;
}

void Test_ToggleDebug(void)
{
	g_testReverseDir = (g_testReverseDir == 0) ? 1 : 0;
}

void Test_NextMode(void)
{
	if (g_testDisplayMethod == TEST_DISPLAY_METHOD_SINGLE)
	{
		g_testDisplayMethod = TEST_DISPLAY_METHOD_DUAL;
	}
	else
	{
		g_testDisplayMethod = TEST_DISPLAY_METHOD_SINGLE;
	}
}

void Test_SetDisplayMethod(uint8_t method)
{
	if ((method == TEST_DISPLAY_METHOD_SINGLE) || (method == TEST_DISPLAY_METHOD_DUAL))
	{
		g_testDisplayMethod = method;
	}
}

uint8_t Test_GetDisplayMethod(void)
{
	return g_testDisplayMethod;
}

void Test_SetSolidColor(uint8_t red, uint8_t green, uint8_t blue)
{
	g_testColorRed = red;
	g_testColorGreen = green;
	g_testColorBlue = blue;
	Test_BuildPatternImage(g_testPatternIndex);
	g_testSolidPwmDirty = 1;
}

void Test_SetScanScheme(uint8_t scheme)
{
	if ((scheme == TEST_SCAN_SCHEME_CLASSIC) || (scheme == TEST_SCAN_SCHEME_QUAD))
	{
		g_testScanScheme = scheme;
	}
}

uint8_t Test_GetScanScheme(void)
{
	return g_testScanScheme;
}

void Test_SetRenderMode(uint8_t mode)
{
	if ((mode == TEST_RENDER_MODE_16X64) || (mode == TEST_RENDER_MODE_16X8))
	{
		g_testRenderMode = mode;
		Test_UpdateRenderModeParams();
	}
}

uint8_t Test_GetRenderMode(void)
{
	return g_testRenderMode;
}

void Test_SetPatternIndex(uint8_t patternIndex)
{
	if (patternIndex >= TEST_PATTERN_COUNT)
	{
		patternIndex = (uint8_t)(patternIndex % TEST_PATTERN_COUNT);
	}
	Test_BuildPatternImage(patternIndex);
}

uint8_t Test_GetPatternIndex(void)
{
	return g_testPatternIndex;
}

void Test_SetPixel(uint8_t row, uint8_t col, uint8_t red, uint8_t green, uint8_t blue)
{
	if ((row >= TEST_ROW_NUM) || (col >= TEST_LED_NUM))
	{
		return;
	}

	g_testImageBuf[row][col][0] = green;
	g_testImageBuf[row][col][1] = red;
	g_testImageBuf[row][col][2] = blue;
	g_testPwmCacheDirty = 1;
}

void Test_FillBackgroundBlue(void)
{
	uint8_t row;
	uint8_t col;

	for (row = 0; row < TEST_ROW_NUM; row++)
	{
		for (col = 0; col < TEST_LED_NUM; col++)
		{
			g_testImageBuf[row][col][0] = 0;
			g_testImageBuf[row][col][1] = 0;
			g_testImageBuf[row][col][2] = TEST_BG_FILL_BLUE;
		}
	}
	g_testPwmCacheDirty = 1;
}

void Test_BuildTriangleImage(void)
{
	Test_BuildPatternImage(g_testPatternIndex);
}

void Test_GetSolidColor(uint8_t *red, uint8_t *green, uint8_t *blue)
{
	if (red != NULL)
	{
		*red = g_testColorRed;
	}
	if (green != NULL)
	{
		*green = g_testColorGreen;
	}
	if (blue != NULL)
	{
		*blue = g_testColorBlue;
	}
}

void Test_SetFrameDelayMs(uint16_t delayMs)
{
	if (delayMs < TEST_FRAME_DELAY_MS_MIN)
	{
		delayMs = TEST_FRAME_DELAY_MS_MIN;
	}
	if (delayMs > TEST_FRAME_DELAY_MS_MAX)
	{
		delayMs = TEST_FRAME_DELAY_MS_MAX;
	}
	g_testFrameDelayMs = delayMs;
}

uint16_t Test_GetFrameDelayMs(void)
{
	return g_testFrameDelayMs;
}

void Test_SetFrameDelayUs(uint32_t delayUs)
{
	if (delayUs < TEST_FRAME_DELAY_US_MIN)
	{
		delayUs = TEST_FRAME_DELAY_US_MIN;
	}
	if (delayUs > TEST_FRAME_DELAY_US_MAX)
	{
		delayUs = TEST_FRAME_DELAY_US_MAX;
	}
	g_testFrameDelayUs = delayUs;
}

uint32_t Test_GetFrameDelayUs(void)
{
	return g_testFrameDelayUs;
}

void Test_OnKeyInt0Pressed(void)
{
	g_testKeyInt0Pending = 1;
}

void Test_OnKeyInt1Pressed(void)
{
	g_testKeyInt1Pending = 1;
}

void PWMAT_DMA_ISR(void) interrupt DMA_PWMAT_VECTOR
{
	/* DMA中断中仅做状态释放，保持关键路径最短。 */
	g_testPwmDmaBusy = 0;
	DMA_PWMAT_STA = 0;
}

