/*
 * @file uart.h
 * @author GitHub Copilot
 * @date 2026-04-20
 * @version 1.0
 * @brief UART2 helpers for the WS2812 driver project.
 */

#ifndef __UART_H__
#define __UART_H__

#include "AI8051U.H"

void UART2_Init(void);
uint32_t UART2_GetBaudrate(void);
uint8_t UART2_SetBaudrate(uint32_t baudrate);
void UART2_SetBtAtMode(uint8_t enable);
uint8_t UART2_GetBtAtMode(void);
void UART2_ResetRxRing(void);
uint8_t UART2_GetRxOverflow(void);
uint8_t UART2_TakeRxOverflow(void);
uint8_t UART2_TryPopByte(uint8_t *rxByte);
uint16_t UART2_GetRxTotalCount(void);
uint8_t UART2_GetLastRxByte(uint8_t *rxByte);
uint8_t UART2_DebugCopyRecentRx(uint8_t *buffer, uint8_t maxLength, uint8_t clearAfterCopy);
void UART2_DebugResetRecentRx(void);
uint8_t UART2_DebugHasRecentRx(void);
uint8_t UART2_SendBuffer(const uint8_t *buffer, uint8_t length);
uint8_t UART2_SendText(const char *text);

#endif