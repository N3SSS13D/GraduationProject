/*
 * @file app.h
 * @author GitHub Copilot
 * @date 2026-05-12
 * @version 1.0
 * @brief APP layer public control interface.
 */

#ifndef __APP_H__
#define __APP_H__

#include "draw_drv.h"

void APP_Init(void);
void APP_TaskLoop(void);

void APP_SetRowIntervalUs(unsigned long intervalUs);
void APP_SetNormalRowIntervalUs(unsigned long intervalUs);
void APP_SetLegacyRowIntervalUs(unsigned long intervalUs);
void APP_SetNormalRowIntervalMs(unsigned int intervalMs);
void APP_SetLegacyRowIntervalMs(unsigned int intervalMs);
unsigned long APP_GetRowIntervalUs(void);
unsigned int APP_GetLastPwmUs(void);
unsigned char APP_SetDisplayMode(unsigned char mode16x);
unsigned char APP_GetDisplayMode(void);

unsigned char APP_SetImageIndex(unsigned char imageIndex);
unsigned char APP_GetImageIndex(void);
void APP_NextImage(void);

unsigned char APP_SetRenderEffect(unsigned char effectId);
unsigned char APP_SetContentType(unsigned char contentType);
unsigned char APP_SetDirection(unsigned char direction);
unsigned char APP_SetColorMode(unsigned char colorMode);
void APP_SetScrollStep(unsigned char step);
void APP_SetAnimStep(unsigned char step);
void APP_SetFrameIntervalMs(unsigned int intervalMs);
void APP_SetGradientSpan(unsigned char span);
void APP_SetBrightness(unsigned char brightness);
void APP_SetRenderUseGradient(unsigned char enable);
void APP_SetForegroundColor(unsigned char r, unsigned char g, unsigned char b);
void APP_SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b);
unsigned char APP_SetGlyphDisplayIndex(unsigned char glyphIndex);
unsigned char APP_SetScrollGlyphSequence(const unsigned char *glyphList, unsigned char count);
void APP_ApplyLocalRenderConfig(const DrawDrv_RenderConfig_t *renderCfg);
void APP_ApplyLocalPatternConfig(const DrawDrv_RenderConfig_t *renderCfg, unsigned char patternId);
void APP_ApplyLocalGlyphConfig(const DrawDrv_RenderConfig_t *renderCfg, unsigned char glyphId);
void APP_ReleaseRemoteModeForLocalDisplay(void);
unsigned char APP_RequestRemoteCachedBitmap(void);
void APP_NextPresetMode(void);
void APP_NextOfflinePattern(void);
void APP_ShowOfflineScrollText(void);
void APP_ShowOfflineClock(void);
void APP_ToggleOfflineTextClock(void);
void APP_NextOfflineEffect(void);
void APP_NextOfflineColor(void);
void APP_ToggleControlMode(void);
unsigned char APP_GetControlMode(void);
unsigned char APP_GetPresetMode(void);
unsigned char APP_ToggleScanMode(void);
unsigned char APP_GetScanMode(void);
void APP_SetDebugMode(unsigned char enable);
unsigned char APP_GetDebugMode(void);
void APP_RestoreNormalScan(void);

#endif