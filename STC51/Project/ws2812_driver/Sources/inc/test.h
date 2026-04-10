#ifndef __TEST_H__
#define __TEST_H__

void Test_Init(void);
void Test_TaskLoop(void);

void Test_SetRowIntervalUs(unsigned long intervalUs);
unsigned long Test_GetRowIntervalUs(void);
unsigned int Test_GetLastPwmUs(void);
unsigned char Test_SetDisplayMode(unsigned char mode16x);
unsigned char Test_GetDisplayMode(void);

unsigned char Test_SetImageIndex(unsigned char imageIndex);
unsigned char Test_GetImageIndex(void);
void Test_NextImage(void);

unsigned char Test_SetRenderEffect(unsigned char effectId);
void Test_SetRenderUseGradient(unsigned char enable);
void Test_SetForegroundColor(unsigned char r, unsigned char g, unsigned char b);
void Test_SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b);

#endif
