#ifndef __TEST_H__
#define __TEST_H__

#define TEST_DISPLAY_METHOD_SINGLE      1
#define TEST_DISPLAY_METHOD_DUAL        2
#define TEST_SCAN_SCHEME_CLASSIC        0
#define TEST_SCAN_SCHEME_QUAD           1
#define TEST_RENDER_MODE_16X64          0
#define TEST_RENDER_MODE_16X8           1
#define TEST_RENDER_MODE_FULL           TEST_RENDER_MODE_16X64
#define TEST_RENDER_MODE_COMPACT        TEST_RENDER_MODE_16X8

void Test_Init(void);
void Test_TaskLoop(void);
void Test_RequestLedCount(unsigned char ledCount);
void Test_ToggleDebug(void);
void Test_NextMode(void);
void Test_SetDisplayMethod(unsigned char method);
unsigned char Test_GetDisplayMethod(void);
void Test_SetSolidColor(unsigned char red, unsigned char green, unsigned char blue);
void Test_GetSolidColor(unsigned char *red, unsigned char *green, unsigned char *blue);
void Test_SetScanScheme(unsigned char scheme);
unsigned char Test_GetScanScheme(void);
void Test_SetRenderMode(unsigned char mode);
unsigned char Test_GetRenderMode(void);
void Test_SetPatternIndex(unsigned char patternIndex);
unsigned char Test_GetPatternIndex(void);
void Test_SetPixel(unsigned char row, unsigned char col, unsigned char red, unsigned char green, unsigned char blue);
void Test_FillBackgroundBlue(void);
void Test_BuildTriangleImage(void);
void Test_SetFrameDelayMs(unsigned int delayMs);
unsigned int Test_GetFrameDelayMs(void);
void Test_SetFrameDelayUs(unsigned long delayUs);
unsigned long Test_GetFrameDelayUs(void);
void Test_OnKeyInt0Pressed(void);
void Test_OnKeyInt1Pressed(void);

#endif
