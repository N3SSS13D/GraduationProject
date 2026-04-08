#ifndef __TEST_H__
#define __TEST_H__

void Test_Init(void);
void Test_TaskLoop(void);

void Test_SetRowIntervalUs(unsigned long intervalUs);
unsigned long Test_GetRowIntervalUs(void);
unsigned int Test_GetLastPwmUs(void);

#endif
