//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// File name: exti.c
// File desc: External interrupt handlers for key switch
// File ver : V1.1
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>

#include "config.h"

//<<AICUBE_USER_INCLUDE_BEGIN>>
#include "key_ctrl.h"
//<<AICUBE_USER_INCLUDE_END>>

//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
//<<AICUBE_USER_GLOBAL_DEFINE_END>>

////////////////////////////////////////
// EXTI0 init (P3.2 / low-active key)
////////////////////////////////////////
void EXTI0_Init(void)
{
	IT0 = 0;                            // low-level effective (active low key on P3.2)
	INT0_SetIntPriority(0);
	INT0_EnableInt();
}

////////////////////////////////////////
// EXTI1 init (P3.3 / low-active key)
////////////////////////////////////////
void EXTI1_Init(void)
{
	IT1 = 0;                            // low-level effective (active low key on P3.3)
	INT1_SetIntPriority(0);
	INT1_EnableInt();
}

////////////////////////////////////////
// EXTI0 ISR
////////////////////////////////////////
void EXTI0_ISR(void) interrupt INT0_VECTOR
{
	KeyCtrl_Int0Isr();
}

////////////////////////////////////////
// EXTI1 ISR
////////////////////////////////////////
void EXTI1_ISR(void) interrupt INT1_VECTOR
{
	KeyCtrl_Int1Isr();
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>


