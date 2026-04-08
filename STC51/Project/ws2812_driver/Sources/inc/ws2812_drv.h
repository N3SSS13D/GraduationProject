#ifndef __WS2812_DRV_H__
#define __WS2812_DRV_H__

#define WS2812DRV_ROW_NUM                    16
#define WS2812DRV_COL_NUM                    8
#define WS2812DRV_PIXEL_CHANNELS             3
#define WS2812DRV_PWM_NUM                    (WS2812DRV_COL_NUM * 24 + 2)
#define WS2812DRV_PWM_NUM_DUAL               (WS2812DRV_PWM_NUM * 2)

#define WS2812DRV_PWM_DUTY_BIT0              12
#define WS2812DRV_PWM_DUTY_BIT1              36
#define WS2812DRV_ROW_SWITCH_SETTLE_US       3
#define WS2812DRV_DMA_TAIL_GUARD_BYTES       2

void WS2812DRV_Init(void);
void WS2812DRV_ClearImage(void);
void WS2812DRV_SetPixelRgb(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b);
void WS2812DRV_EncodeAllRows(void);
uint16_t WS2812DRV_BuildDualRowPwmBuffer(uint8_t rowA, uint8_t rowB);
uint8_t xdata *WS2812DRV_GetDualRowPwmBuffer(void);
void WS2812DRV_SelectRows(uint8_t rowA, uint8_t rowB);
void WS2812DRV_TriggerDualRowDma(uint8_t xdata *txBuf, uint16_t num);
bit WS2812DRV_WaitDmaDone(void);
void WS2812DRV_StopPwmDualChannels(void);
bit WS2812DRV_SendRowPair(uint8_t rowA, uint8_t rowB);
void WS2812DRV_OnDmaIsr(void);
bit WS2812DRV_IsDmaBusy(void);

#endif
