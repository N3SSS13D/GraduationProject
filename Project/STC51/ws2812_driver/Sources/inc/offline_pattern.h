/*
 * @file offline_pattern.h
 * @author GitHub Copilot
 * @date 2026-05-05
 * @version 1.0
 * @brief Local offline pattern storage and pixel query interface.
 */

#ifndef __OFFLINE_PATTERN_H__
#define __OFFLINE_PATTERN_H__

#define OFFLINE_PATTERN_ROWS              16U
#define OFFLINE_PATTERN_COLS              16U
#define OFFLINE_PATTERN_PIXELS_PER_FRAME  (OFFLINE_PATTERN_ROWS * OFFLINE_PATTERN_COLS)
#define OFFLINE_PATTERN_BG_RGB332         0x00U
#define OFFLINE_PATTERN_BITMAP_BYTES      (OFFLINE_PATTERN_ROWS * 2U)
#define OFFLINE_PATTERN_COLOR_BYTES       3U
#define OFFLINE_PATTERN_FRAME_BYTES       (1U + OFFLINE_PATTERN_BITMAP_BYTES + OFFLINE_PATTERN_COLOR_BYTES)

#define OFFLINE_PATTERN_BASE_COUNT        3U
#define OFFLINE_PATTERN_EXTRA_COUNT       3U
#define OFFLINE_PATTERN_COUNT             (OFFLINE_PATTERN_BASE_COUNT + OFFLINE_PATTERN_EXTRA_COUNT)

#define OFFLINE_PATTERN_IDX_DIAMOND       0U
#define OFFLINE_PATTERN_IDX_CROSS         1U
#define OFFLINE_PATTERN_IDX_JLU_EMBLEM    2U
#define OFFLINE_PATTERN_IDX_CHECKER       3U
#define OFFLINE_PATTERN_IDX_BORDER        4U
#define OFFLINE_PATTERN_IDX_DIAGONAL_X    5U

uint8_t OfflinePattern_GetCount(void);
const uint8_t code *OfflinePattern_GetFrame(uint8_t patternIndex);
uint8_t OfflinePattern_GetPixel(uint8_t patternIndex, uint8_t row, uint8_t col);

#endif
