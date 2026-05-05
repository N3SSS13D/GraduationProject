//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息  
// 文件名称: port.h
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>


#ifndef __PORT_H__
#define __PORT_H__


//<<AICUBE_USER_DEFINE_BEGIN>>
// 在此添加用户宏定义  
//<<AICUBE_USER_DEFINE_END>>


void PORT2_Init(void);
void PORT0_Init(void);
void PORT1_Init(void);
void PORT3_Init(void);
void PORT4_Init(void);
void PORT2_ClearDebugLeds(void);
void PORT2_SetDebugLedDigit(uint8_t digit);
uint8_t PORT2_GetDebugLedDigit(void);



//<<AICUBE_USER_EXTERNAL_DECLARE_BEGIN>>
// 在此添加用户外部函数和外部变量声明  
//<<AICUBE_USER_EXTERNAL_DECLARE_END>>


#endif
