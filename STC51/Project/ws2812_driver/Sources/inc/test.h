#ifndef __TEST_H__
#define __TEST_H__

void Test_Init(void);
void Test_TaskLoop(void);

void Test_SetRowIntervalUs(unsigned long intervalUs);
void Test_SetNormalRowIntervalUs(unsigned long intervalUs);
void Test_SetLegacyRowIntervalUs(unsigned long intervalUs);
void Test_SetNormalRowIntervalMs(unsigned int intervalMs);
void Test_SetLegacyRowIntervalMs(unsigned int intervalMs);
unsigned long Test_GetRowIntervalUs(void);
unsigned int Test_GetLastPwmUs(void);
unsigned char Test_SetDisplayMode(unsigned char mode16x);
unsigned char Test_GetDisplayMode(void);

unsigned char Test_SetImageIndex(unsigned char imageIndex);
unsigned char Test_GetImageIndex(void);
void Test_NextImage(void);

unsigned char Test_SetRenderEffect(unsigned char effectId);
unsigned char Test_SetContentType(unsigned char contentType);
unsigned char Test_SetDirection(unsigned char direction);
unsigned char Test_SetColorMode(unsigned char colorMode);
void Test_SetScrollStep(unsigned char step);
void Test_SetAnimStep(unsigned char step);
void Test_SetGradientSpan(unsigned char span);
void Test_SetBrightness(unsigned char brightness);
void Test_SetRenderUseGradient(unsigned char enable);
void Test_SetForegroundColor(unsigned char r, unsigned char g, unsigned char b);
void Test_SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b);
unsigned char Test_SetGlyphDisplayIndex(unsigned char glyphIndex);
unsigned char Test_SetScrollGlyphSequence(const unsigned char *glyphList, unsigned char count);
void Test_NextPresetMode(void);
void Test_NextOfflinePattern(void);
void Test_NextOfflineEffect(void);
void Test_ToggleControlMode(void);
unsigned char Test_GetControlMode(void);
unsigned char Test_GetPresetMode(void);
unsigned char Test_ToggleScanMode(void);
unsigned char Test_GetScanMode(void);
void Test_SetDebugMode(unsigned char enable);
unsigned char Test_GetDebugMode(void);
void Test_DebugMarkRowSwitchStart(void);
void Test_DebugMarkPwmSendDone(void);

#endif
