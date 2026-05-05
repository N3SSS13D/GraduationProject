#ifndef __KEY_CTRL_H__
#define __KEY_CTRL_H__

void KeyCtrl_Init(void);
void KeyCtrl_Int0Isr(void);
void KeyCtrl_Int1Isr(void);
void KeyCtrl_Task10ms(void);

#endif