#include "config.h"
#include "key_ctrl.h"

#define KEYCTRL_DEBOUNCE_TICKS          3U
#define KEYCTRL_LONG_PRESS_TICKS        80U

static volatile uint8_t g_key0Event = 0U;
static volatile uint8_t g_key0Debounce = 0U;
static volatile uint8_t g_key1Pressed = 0U;
static volatile uint8_t g_key1Debounce = 0U;
static volatile uint8_t g_key1PressTicks = 0U;
static volatile uint8_t g_key1LongHandled = 0U;

void KeyCtrl_Init(void)
{
	g_key0Event = 0U;
	g_key0Debounce = 0U;
	g_key1Pressed = 0U;
	g_key1Debounce = 0U;
	g_key1PressTicks = 0U;
	g_key1LongHandled = 0U;
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
	if ((P33 == 0) && (g_key1Debounce == 0U) && (g_key1Pressed == 0U))
	{
		INT1_DisableInt();
		g_key1Pressed = 1U;
		g_key1PressTicks = 0U;
		g_key1LongHandled = 0U;
		g_key1Debounce = KEYCTRL_DEBOUNCE_TICKS;
	}
}

void KeyCtrl_Task10ms(void)
{
	if (g_key0Debounce > 0U)
	{
		g_key0Debounce--;
	}

	if (g_key1Debounce > 0U)
	{
		g_key1Debounce--;
	}

	if (g_key0Event != 0U)
	{
		g_key0Event = 0U;
		Test_NextOfflinePattern();
	}

	if (g_key1Pressed != 0U)
	{
		if (P33 == 0)
		{
			if (g_key1PressTicks < 0xFFU)
			{
				g_key1PressTicks++;
			}

			if ((g_key1PressTicks >= KEYCTRL_LONG_PRESS_TICKS) && (g_key1LongHandled == 0U))
			{
				Test_ToggleControlMode();
				g_key1LongHandled = 1U;
			}
		}
		else if (g_key1Debounce == 0U)
		{
			if (g_key1LongHandled == 0U)
			{
				Test_NextOfflineEffect();
			}

			g_key1Pressed = 0U;
			g_key1PressTicks = 0U;
			g_key1LongHandled = 0U;
			INT1_EnableInt();
		}
	}

	if ((g_key0Debounce == 0U) && (P32 != 0))
	{
		INT0_EnableInt();
	}

	if ((g_key1Debounce == 0U) && (g_key1Pressed == 0U) && (P33 != 0))
	{
		INT1_EnableInt();
	}
}