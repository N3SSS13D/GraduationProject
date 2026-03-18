#include "config.h"
#include "mid_task.h"

typedef struct
{
    uint8_t run;
    uint16_t tickCount;
    uint16_t period;
    MidTaskHook_t hook;
} MidTaskComponent_t;

static MidTaskComponent_t g_midTasks[MIDTASK_MAX_COUNT];
static uint8_t g_midTaskCount = 0;

void MidTask_Init(void)
{
    uint8_t idx;

    for (idx = 0; idx < MIDTASK_MAX_COUNT; idx++)
    {
        g_midTasks[idx].run = 0;
        g_midTasks[idx].tickCount = 0;
        g_midTasks[idx].period = 0;
        g_midTasks[idx].hook = NULL;
    }

    g_midTaskCount = 0;
}

uint8_t MidTask_Register(uint16_t periodMs, MidTaskHook_t hook)
{
    if ((hook == NULL) || (periodMs == 0) || (g_midTaskCount >= MIDTASK_MAX_COUNT))
    {
        return 0;
    }

    g_midTasks[g_midTaskCount].run = 0;
    g_midTasks[g_midTaskCount].tickCount = periodMs;
    g_midTasks[g_midTaskCount].period = periodMs;
    g_midTasks[g_midTaskCount].hook = hook;
    g_midTaskCount++;

    return 1;
}

void MidTask_Tick1ms(void)
{
    uint8_t idx;

    for (idx = 0; idx < g_midTaskCount; idx++)
    {
        if (g_midTasks[idx].tickCount > 0)
        {
            g_midTasks[idx].tickCount--;
            if (g_midTasks[idx].tickCount == 0)
            {
                g_midTasks[idx].tickCount = g_midTasks[idx].period;
                g_midTasks[idx].run = 1;
            }
        }
    }
}

void MidTask_Process(void)
{
    uint8_t idx;

    for (idx = 0; idx < g_midTaskCount; idx++)
    {
        if (g_midTasks[idx].run != 0)
        {
            g_midTasks[idx].run = 0;
            g_midTasks[idx].hook();
        }
    }
}