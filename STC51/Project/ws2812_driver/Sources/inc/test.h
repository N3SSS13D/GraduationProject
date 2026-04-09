#ifndef __TEST_H__
#define __TEST_H__

void Test_Init(void);
void Test_TaskLoop(void);

void Test_SetRowIntervalUs(unsigned long intervalUs);
unsigned long Test_GetRowIntervalUs(void);
unsigned int Test_GetLastPwmUs(void);
unsigned char Test_SetDebugRow(unsigned char row);
void Test_ClearDebugRow(void);
unsigned char Test_SetDisplayMode(unsigned char mode16x);
unsigned char Test_GetDisplayMode(void);

#endif
