#include "config.h"
#include "key_ctrl.h"
#include "test.h"

#define KEYCTRL_DEBOUNCE_10MS           5

static volatile uint8_t g_key0RawEvent = 0;
static volatile uint8_t g_key1RawEvent = 0;
static uint8_t g_key0Debounce = 0;
static uint8_t g_key1Debounce = 0;

extern void Test_ToggleDebug(void);
extern void Test_NextMode(void);

void KeyCtrl_Init(void)
{
    g_key0RawEvent = 0;
    g_key1RawEvent = 0;
    g_key0Debounce = 0;
    g_key1Debounce = 0;
}

void KeyCtrl_Int0Isr(void)
{
    g_key0RawEvent = 1;
}

void KeyCtrl_Int1Isr(void)
{
    g_key1RawEvent = 1;
}

void KeyCtrl_Task10ms(void)
{
    if (g_key0Debounce > 0)
    {
        g_key0Debounce--;
    }

    if (g_key1Debounce > 0)
    {
        g_key1Debounce--;
    }

    if ((g_key0RawEvent != 0) && (g_key0Debounce == 0))
    {
        g_key0RawEvent = 0;
        g_key0Debounce = KEYCTRL_DEBOUNCE_10MS;
        Test_ToggleDebug();
    }
    else
    {
        g_key0RawEvent = 0;
    }

    if ((g_key1RawEvent != 0) && (g_key1Debounce == 0))
    {
        g_key1RawEvent = 0;
        g_key1Debounce = KEYCTRL_DEBOUNCE_10MS;
        Test_NextMode();
    }
    else
    {
        g_key1RawEvent = 0;
    }
}