/*
 * @file gp_led_matrix_usb_debug.h
 * @author GitHub Copilot
 * @date 2026-05-14
 * @version 1.0
 * @brief Temporary USB/key DEBUG handler: lock two rows ON, cycle white/black 5s each.
 */

#ifndef __GP_LED_MATRIX_USB_DEBUG_H__
#define __GP_LED_MATRIX_USB_DEBUG_H__

#include "AI8051U.H"

void GpLedMatrixUsbDebug_Enter(void);
void GpLedMatrixUsbDebug_Exit(void);
void GpLedMatrixUsbDebug_Toggle(void);
void GpLedMatrixUsbDebug_Run(void);

#endif
