/*
 * @file gp_led_matrix_bt_debug.h
 * @author GitHub Copilot
 * @date 2026-04-20
 * @version 1.0
 * @brief USB to UART2 debug bridge helpers for the HC-05 test path.
 */

#ifndef __GP_LED_MATRIX_BT_DEBUG_H__
#define __GP_LED_MATRIX_BT_DEBUG_H__

#include "AI8051U.H"

void GpLedMatrixBtDebug_SetReady(uint8_t ready);
void GpLedMatrixBtDebug_PrintInit(void);
void GpLedMatrixBtDebug_Task(void);
void GpLedMatrixBtDebug_HandleUsbCommand(const uint8_t *commandBytes, uint8_t length);

#endif