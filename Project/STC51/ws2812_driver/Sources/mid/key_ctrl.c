#include "config.h"
#include "key_ctrl.h"

#define KEYCTRL_DEBOUNCE_TICKS          3U
#define KEYCTRL_LONG_PRESS_TICKS        80U

typedef struct
{
	uint8_t pressed;
	uint8_t debounce;
	uint8_t pressTicks;
	uint8_t longHandled;
} KeyCtrl_KeyState_t;

static volatile KeyCtrl_KeyState_t g_key0State;
static volatile KeyCtrl_KeyState_t g_key1State;

void KeyCtrl_Init(void)
{
	g_key0State.pressed = 0U;
	g_key0State.debounce = 0U;
	g_key0State.pressTicks = 0U;
	g_key0State.longHandled = 0U;
	g_key1State.pressed = 0U;
	g_key1State.debounce = 0U;
	g_key1State.pressTicks = 0U;
	g_key1State.longHandled = 0U;
}

void KeyCtrl_Int0Isr(void)
{
	if ((P32 == 0) && (g_key0State.debounce == 0U) && (g_key0State.pressed == 0U))
	{
		INT0_DisableInt();
		g_key0State.pressed = 1U;
		g_key0State.pressTicks = 0U;
		g_key0State.longHandled = 0U;
		g_key0State.debounce = KEYCTRL_DEBOUNCE_TICKS;
	}
}

void KeyCtrl_Int1Isr(void)
{
	if ((P33 == 0) && (g_key1State.debounce == 0U) && (g_key1State.pressed == 0U))
	{
		INT1_DisableInt();
		g_key1State.pressed = 1U;
		g_key1State.pressTicks = 0U;
		g_key1State.longHandled = 0U;
		g_key1State.debounce = KEYCTRL_DEBOUNCE_TICKS;
	}
}

void KeyCtrl_Task10ms(void)
{
	if (g_key0State.debounce > 0U)
	{
		g_key0State.debounce--;
	}

	if (g_key1State.debounce > 0U)
	{
		g_key1State.debounce--;
	}

	/* P32: short press switches pattern, long press toggles scrolling text and the synchronized clock. */
	if (g_key0State.pressed != 0U)
	{
		if (P32 == 0)
		{
			if (g_key0State.pressTicks < 0xFFU)
			{
				g_key0State.pressTicks++;
			}

			if ((g_key0State.pressTicks >= KEYCTRL_LONG_PRESS_TICKS) && (g_key0State.longHandled == 0U))
			{
				APP_ToggleOfflineTextClock();
				g_key0State.longHandled = 1U;
			}
		}
		else if (g_key0State.debounce == 0U)
		{
			if (g_key0State.longHandled == 0U)
			{
				APP_NextOfflinePattern();
			}

			g_key0State.pressed = 0U;
			g_key0State.pressTicks = 0U;
			g_key0State.longHandled = 0U;
			INT0_EnableInt();
		}
	}

	/* P33: short press switches effect, long press switches color theme. */
	if (g_key1State.pressed != 0U)
	{
		if (P33 == 0)
		{
			if (g_key1State.pressTicks < 0xFFU)
			{
				g_key1State.pressTicks++;
			}

			if ((g_key1State.pressTicks >= KEYCTRL_LONG_PRESS_TICKS) && (g_key1State.longHandled == 0U))
			{
				APP_NextOfflineColor();
				g_key1State.longHandled = 1U;
			}
		}
		else if (g_key1State.debounce == 0U)
		{
			if (g_key1State.longHandled == 0U)
			{
				APP_NextOfflineEffect();
			}

			g_key1State.pressed = 0U;
			g_key1State.pressTicks = 0U;
			g_key1State.longHandled = 0U;
			INT1_EnableInt();
		}
	}

	if ((g_key0State.debounce == 0U) && (g_key0State.pressed == 0U) && (P32 != 0))
	{
		INT0_EnableInt();
	}

	if ((g_key1State.debounce == 0U) && (g_key1State.pressed == 0U) && (P33 != 0))
	{
		INT1_EnableInt();
	}
}