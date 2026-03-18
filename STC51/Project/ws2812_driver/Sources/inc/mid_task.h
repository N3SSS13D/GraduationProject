#ifndef __MID_TASK_H__
#define __MID_TASK_H__

#define MIDTASK_MAX_COUNT               8

typedef void (*MidTaskHook_t)(void);

void MidTask_Init(void);
uint8_t MidTask_Register(uint16_t periodMs, MidTaskHook_t hook);
void MidTask_Tick1ms(void);
void MidTask_Process(void);

#endif