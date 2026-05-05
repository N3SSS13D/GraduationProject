#ifndef __MID_TASK_H__
#define __MID_TASK_H__

#define MIDTASK_MAX_COUNT               8
#define MIDTASK_INVALID_ID              0xFF

typedef void (*MidTaskHook_t)(void);

void MidTask_Init(void);
uint8_t MidTask_Register(uint16_t periodMs, MidTaskHook_t hook);
uint8_t MidTask_RegisterWithId(uint16_t periodMs, MidTaskHook_t hook);
uint8_t MidTask_SetPeriod(uint8_t taskId, uint16_t periodMs);
void MidTask_Tick1ms(void);
void MidTask_Process(void);

#endif