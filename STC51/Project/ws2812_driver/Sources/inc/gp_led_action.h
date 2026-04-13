#ifndef __GP_LED_ACTION_H__
#define __GP_LED_ACTION_H__

#include "gp_led_matrix_protocol.h"

void GpLedAction_Init(void);
void GpLedAction_SetBrightness(uint8_t brightness);
void GpLedAction_ReleaseRemoteMode(void);
uint8_t GpLedAction_IsRemoteModeActive(void);
uint8_t GpLedAction_ShouldBypassDrawScheduler(void);
GpMatrixStatusCode GpLedAction_ApplyAction(const GpMatrixActionPayload xdata *payload);
GpMatrixStatusCode GpLedAction_ApplyFrameRgb332(const uint8_t xdata *frameData, uint16_t length, GpMatrixMode mode);
GpMatrixStatusCode GpLedAction_ApplyGlyphRows(const uint8_t xdata *glyphData,
                                              uint16_t length,
                                              uint8_t glyphCount,
                                              uint8_t glyphWidth,
                                              uint8_t glyphSpacing);

#endif