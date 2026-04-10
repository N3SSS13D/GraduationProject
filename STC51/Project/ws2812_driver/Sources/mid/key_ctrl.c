#include "config.h"
#include "key_ctrl.h"

#define KEYCTRL_DEBOUNCE_TICKS          3U

static volatile uint8_t g_key0Event = 0U;
static volatile uint8_t g_key0Debounce = 0U;

void KeyCtrl_Init(void)
{
	g_key0Event = 0U;
	g_key0Debounce = 0U;
}

void KeyCtrl_Int0Isr(void)
{
	if ((P32 == 0) && (g_key0Debounce == 0U))
	{
		INT0_DisableInt();
		g_key0Event = 1U;
		g_key0Debounce = KEYCTRL_DEBOUNCE_TICKS;
	}
}

void KeyCtrl_Int1Isr(void)
{
}

void KeyCtrl_Task10ms(void)
{
	if (g_key0Debounce > 0U)
	{
		g_key0Debounce--;
	}

	if (g_key0Event != 0U)
	{
		g_key0Event = 0U;
		Test_NextPresetMode();
	}

	if ((g_key0Debounce == 0U) && (P32 != 0))
	{
		INT0_EnableInt();
	}
}