#ifndef __GP_LED_ACTION_H__
#define __GP_LED_ACTION_H__

#include "gp_led_display_profile.h"
#include "gp_led_matrix_protocol.h"

typedef enum GpLedControlMode {
    kGpLedControlModeOffline = 0,
    kGpLedControlModeOnline = 1,
} GpLedControlMode;

void GpLedAction_Init(void);
void GpLedAction_SetBrightness(uint8_t brightness);
void GpLedAction_ReleaseRemoteMode(void);
void GpLedAction_NotifyCommunicationActive(void);
void GpLedAction_Tick1ms(void);
void GpLedAction_Task10ms(void);
void GpLedAction_ToggleModeOverride(void);
uint8_t GpLedAction_IsRemoteModeActive(void);
uint8_t GpLedAction_IsOnlineModeActive(void);
uint8_t GpLedAction_IsHostControlEnabled(void);
GpLedControlMode GpLedAction_GetControlMode(void);
uint8_t GpLedAction_ShouldBypassDrawScheduler(void);

/* Process any pending animation frame render deferred from the Timer0 ISR.
   Call this from the main loop so heavy rendering/encoding stays out of interrupt context. */
void GpLedAction_RenderPendingAnimationFrame(void);

GpMatrixStatusCode GpLedAction_SetDebugLedFlow(uint8_t enable);
GpMatrixStatusCode GpLedAction_ApplyDisplayProfile(const GpLedDisplayProfile xdata *profile);
GpMatrixStatusCode GpLedAction_ApplyLocalDisplayProfile(const GpLedDisplayProfile xdata *profile);
GpMatrixStatusCode GpLedAction_ApplyAction(const GpMatrixActionPayload xdata *payload);
GpMatrixStatusCode GpLedAction_SyncClockTime(const GpMatrixTimeSyncPayload xdata *payload);
GpMatrixStatusCode GpLedAction_BeginAnimationUpload(uint8_t frameFormat,
                                                    uint8_t frameCount,
                                                    uint16_t frameIntervalMs,
                                                    uint8_t flags);
GpMatrixStatusCode GpLedAction_StoreAnimationFrame(uint8_t frameIndex,
                                                   const uint8_t xdata *frameData,
                                                   uint16_t length);
GpMatrixStatusCode GpLedAction_CommitAnimation(uint8_t frameCount);
GpMatrixStatusCode GpLedAction_ApplyFrameRgb332(const uint8_t xdata *frameData, uint16_t length, GpMatrixMode mode);
GpMatrixStatusCode GpLedAction_ApplyFrameBitmapRgb888(const uint8_t xdata *frameData,
                                                      uint16_t length,
                                                      GpMatrixMode mode);
GpMatrixStatusCode GpLedAction_ApplyFrameBitmapLayered(const uint8_t xdata *frameData,
                                                        uint16_t length,
                                                        GpMatrixMode mode);
GpMatrixStatusCode GpLedAction_ApplyGlyphRows(const uint8_t xdata *glyphData,
                                              uint16_t length,
                                              uint8_t glyphCount,
                                              uint8_t glyphWidth,
                                              uint8_t glyphSpacing);

#endif