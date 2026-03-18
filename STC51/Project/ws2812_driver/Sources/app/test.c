#include "config.h"

#define TEST_COLOR_LEVEL                 50
#define TEST_LED_NUM                     24
#define TEST_PWM_NUM                     (TEST_LED_NUM * 24 + 2)
#define TEST_PWM_DUTY_BIT0               12
#define TEST_PWM_DUTY_BIT1               37
#define TEST_FRAME_DELAY_MS              50

/* PWMAT-DMA控制寄存器位定义。 */
#define TEST_DMA_ENPWMAT                 (1 << 7)
#define TEST_PWMAT_TRIG                  (1 << 6)

/* PWMAT-DMA配置寄存器位定义。 */
#define TEST_DMA_PWMATIE                 (1 << 7)
#define TEST_DMA_PWMATIP                 (0 << 2)
#define TEST_DMA_PWMATPTY                2

static uint8_t xdata g_testLedRgb[TEST_LED_NUM][3];
static uint8_t xdata g_testLedPwm[TEST_PWM_NUM];
static bit g_testPwmDmaBusy;
static uint8_t g_testScanIndex;
static uint8_t g_testLedCount = TEST_LED_NUM;
static uint8_t g_testReverseDir = 0;

static void Test_LoadSPI(void)
{
	uint8_t xdata *pRgb;
	uint16_t i;
	uint16_t j;
	uint8_t k;
	uint8_t dat;

	for (i = 0; i < TEST_PWM_NUM; i++)
	{
		g_testLedPwm[i] = 0;
	}

	pRgb = &g_testLedRgb[0][0];
	for (i = 0, j = 1; i < (uint16_t)(g_testLedCount * 3); i++)
	{
		dat = *pRgb;
		pRgb++;
		for (k = 0; k < 8; k++)
		{
			if ((dat & 0x80) != 0)
			{
				g_testLedPwm[j] = TEST_PWM_DUTY_BIT1;
			}
			else
			{
				g_testLedPwm[j] = TEST_PWM_DUTY_BIT0;
			}
			dat <<= 1;
			j++;
		}
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
	PWMA_ARRL = (50 - 1);

	PWMA_CCMR1 = 0x60;
	PWMA_CCR1H = 0;
	PWMA_CCR1L = 0;
	ccer1 |= 0x05;
	ps |= 0;
	eno |= 0x01;
	SetP1nPushPullMode(PIN_0);

	PWMA_CCER1 = ccer1;
	PWMA_CCER2 = ccer2;
	PWMA_PS = ps;
	PWMA_CCMR1 = 0x68;

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

	PWMA_DBA = 0x0D;
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
	g_testPwmDmaBusy = 0;
	g_testScanIndex = 0;

	pRgb = &g_testLedRgb[0][0];
	for (i = 0; i < (uint16_t)(TEST_LED_NUM * 3); i++)
	{
		*pRgb = 0;
		pRgb++;
	}
}

void Test_TaskLoop(void)
{
	uint16_t i;
	uint8_t xdata *pRgb;
	uint8_t idx;
	uint8_t ledCount;

	if (g_testPwmDmaBusy != 0)
	{
		return;
	}

	pRgb = &g_testLedRgb[0][0];
	for (i = 0; i < (uint16_t)(TEST_LED_NUM * 3); i++)
	{
		*pRgb = 0;
		pRgb++;
	}

	ledCount = g_testLedCount;
	if (ledCount == 0)
	{
		ledCount = 1;
	}
	if (ledCount > TEST_LED_NUM)
	{
		ledCount = TEST_LED_NUM;
	}

	idx = g_testScanIndex;
	g_testLedRgb[idx][1] = TEST_COLOR_LEVEL;

	idx++;
	if (idx >= ledCount)
	{
		idx = 0;
	}
	g_testLedRgb[idx][0] = TEST_COLOR_LEVEL;

	idx++;
	if (idx >= ledCount)
	{
		idx = 0;
	}
	g_testLedRgb[idx][2] = TEST_COLOR_LEVEL;

	Test_LoadSPI();
	Test_PWMATDmaTrig(g_testLedPwm, TEST_PWM_NUM);

	if (g_testReverseDir == 0)
	{
		g_testScanIndex++;
		if (g_testScanIndex >= ledCount)
		{
			g_testScanIndex = 0;
		}
	}
	else
	{
		if (g_testScanIndex == 0)
		{
			g_testScanIndex = (uint8_t)(ledCount - 1);
		}
		else
		{
			g_testScanIndex--;
		}
	}

	delay_ms(TEST_FRAME_DELAY_MS);
}

void Test_RequestLedCount(uint8_t ledCount)
{
	if (ledCount == 0)
	{
		g_testLedCount = 1;

		return;
	}

	if (ledCount > TEST_LED_NUM)
	{
		g_testLedCount = TEST_LED_NUM;

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
	g_testScanIndex++;
	if (g_testScanIndex >= g_testLedCount)
	{
		g_testScanIndex = 0;
	}
}

void PWMAT_DMA_ISR(void) interrupt DMA_PWMAT_VECTOR
{
	/* DMA中断中仅做状态释放，保持关键路径最短。 */
	g_testPwmDmaBusy = 0;
	DMA_PWMAT_STA = 0;
}

