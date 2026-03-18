#ifndef __WS2812_DRV_H__
#define __WS2812_DRV_H__

#define WS2812DRV_PANEL_WIDTH           16
#define WS2812DRV_PANEL_HEIGHT          8
#define WS2812DRV_COLORS_PER_LED        3

#define WS2812DRV_BITS_PER_LED          24
#define WS2812DRV_DMA_HEAD_ZERO         1
#define WS2812DRV_DMA_TAIL_ZERO         1
#define WS2812DRV_PWM_DUTY_BIT0         12
#define WS2812DRV_PWM_DUTY_BIT1         37
#define WS2812DRV_TEST_RED_LEVEL        64

#define WS2812DRV_PMOS_OFF_TO_ON_US     10
#define WS2812DRV_ROW_SWITCH_SETTLE_US  20
#define WS2812DRV_RESET_HOLD_US         80
#define WS2812DRV_SCAN_HOLD_TICKS       1
#define WS2812DRV_FADE_PHASE_STEP       8

#define WS2812DRV_ROW_PAIRS             (WS2812DRV_PANEL_HEIGHT / 2)
#define WS2812DRV_LED_PER_ROW           WS2812DRV_PANEL_WIDTH
#define WS2812DRV_SYMBOLS_PER_ROW       (WS2812DRV_LED_PER_ROW * WS2812DRV_BITS_PER_LED)
#define WS2812DRV_TX_SYMBOL_COUNT       (WS2812DRV_DMA_HEAD_ZERO + WS2812DRV_SYMBOLS_PER_ROW + WS2812DRV_DMA_TAIL_ZERO)
#define WS2812DRV_TX_BYTE_COUNT         (WS2812DRV_TX_SYMBOL_COUNT * 2)

#if ((WS2812DRV_PANEL_HEIGHT % 2) != 0)
#error "WS2812DRV_PANEL_HEIGHT must be even for row-pair scan"
#endif

#if (WS2812DRV_TX_BYTE_COUNT > PWM_DMA_TX_BUFFER_SIZE)
#error "PWM_DMA_TX_BUFFER_SIZE is too small for WS2812 row-pair transmit"
#endif

void WS2812DRV_Init(void);
void WS2812DRV_ClearAll(void);
void WS2812DRV_SetPixelRgb(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void WS2812DRV_Scan1msHook(void);
void WS2812DRV_TaskService(void);
void WS2812DRV_ForceRefreshAll(void);
uint8_t WS2812DRV_IsBusy(void);
void WS2812DRV_ShowRowRedCount(uint8_t row, uint8_t ledCount);

#endif
