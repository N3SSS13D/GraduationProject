/*
 * @file rtc_clock.h
 * @author GitHub Copilot
 * @date 2026-05-12
 * @version 1.1
 * @brief Bluetooth-synced 3x5 digital clock pixel provider for local LED rendering.
 */

#ifndef __RTC_CLOCK_H__
#define __RTC_CLOCK_H__

#include "def.h"
#include "gp_led_matrix_protocol.h"

/* Initialize the local clock cache.
 * Parameters: none
 * Returns: 1 when the cached time is already valid, 0 otherwise
 */
uint8_t RtcClock_Init(void);

/* Update the local clock cache from one Bluetooth time-sync payload.
 * Parameters: syncPayload shared protocol payload carrying YY/MM/DD/HH/MM/SS
 * Returns: 1 when the payload is valid and accepted, 0 otherwise
 */
uint8_t RtcClock_SyncTime(const GpMatrixTimeSyncPayload xdata *syncPayload);

/* Advance the cached clock using the cooperative draw-task cadence.
 * Parameters: none
 * Returns: 1 when the visible clock content changed, 0 otherwise
 */
uint8_t RtcClock_Task(uint16_t elapsedMs);

/* Query one foreground/background pixel of the 3x5 HH/MM/SS clock layout.
 * Parameters: row matrix row index
 * Parameters: col matrix column index
 * Returns: RGB332 foreground token or background token
 */
uint8_t RtcClock_GetPixel(uint8_t row, uint8_t col);

#endif