/*
 * @file rtc_clock.c
 * @author GitHub Copilot
 * @date 2026-05-12
 * @version 1.3
 * @brief Bluetooth-synced local YYYY/MM.DD/HH:MM clock renderer with progress and edge animation.
 */

#include "config.h"
#include "rtc_clock.h"

#include "local_display_assets.h"

#define RTCCLOCK_DIGIT_WIDTH            3U
#define RTCCLOCK_DIGIT_HEIGHT           5U
#define RTCCLOCK_LINE_COUNT             3U
#define RTCCLOCK_DIGIT_COUNT_PER_LINE   4U
#define RTCCLOCK_LINE0_TOP_ROW          0U
#define RTCCLOCK_LINE1_TOP_ROW          5U
#define RTCCLOCK_LINE2_TOP_ROW          10U
#define RTCCLOCK_PROGRESS_ROW           15U
#define RTCCLOCK_ANIM_COL               15U
#define RTCCLOCK_SEPARATOR_COL          7U
#define RTCCLOCK_DATE_SEPARATOR_ROW     (RTCCLOCK_LINE1_TOP_ROW + RTCCLOCK_DIGIT_HEIGHT - 1U)
#define RTCCLOCK_TIME_SEPARATOR_ROW0    (RTCCLOCK_LINE2_TOP_ROW + 1U)
#define RTCCLOCK_TIME_SEPARATOR_ROW1    (RTCCLOCK_LINE2_TOP_ROW + 3U)
#define RTCCLOCK_ANIM_STEP_MS           128U
#define RTCCLOCK_ANIM_PHASE_COUNT       16U
#define RTCCLOCK_PROGRESS_STEP_SECONDS  4U
#define RTCCLOCK_PROGRESS_COL_COUNT     15U
#define RTCCLOCK_YEAR_BASE              2000U
#define RTCCLOCK_SECONDS_PER_MINUTE     60U
#define RTCCLOCK_MINUTES_PER_HOUR       60U
#define RTCCLOCK_HOURS_PER_DAY          24U
#define RTCCLOCK_MS_PER_SECOND          1000UL
#define RTCCLOCK_YEAR_RGB332            0x1FU
#define RTCCLOCK_DATE_RGB332            0xFCU
#define RTCCLOCK_TIME_RGB332            0x1CU
#define RTCCLOCK_PROGRESS_RGB332        0x03U
#define RTCCLOCK_ANIM_HEAD_RGB332       0xE0U
#define RTCCLOCK_ANIM_TAIL_RGB332       0xC3U

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} RtcClock_Time_t;

static const uint8_t code g_rtcClockDigitRows[10][RTCCLOCK_DIGIT_HEIGHT] =
{
    {0x07U, 0x05U, 0x05U, 0x05U, 0x07U},
    {0x02U, 0x06U, 0x02U, 0x02U, 0x07U},
    {0x07U, 0x01U, 0x07U, 0x04U, 0x07U},
    {0x07U, 0x01U, 0x07U, 0x01U, 0x07U},
    {0x05U, 0x05U, 0x07U, 0x01U, 0x01U},
    {0x07U, 0x04U, 0x07U, 0x01U, 0x07U},
    {0x07U, 0x04U, 0x07U, 0x05U, 0x07U},
    {0x07U, 0x01U, 0x01U, 0x01U, 0x01U},
    {0x07U, 0x05U, 0x07U, 0x05U, 0x07U},
    {0x07U, 0x05U, 0x07U, 0x01U, 0x07U}
};

static const uint8_t code g_rtcClockLineTopRows[RTCCLOCK_LINE_COUNT] =
{
    RTCCLOCK_LINE0_TOP_ROW,
    RTCCLOCK_LINE1_TOP_ROW,
    RTCCLOCK_LINE2_TOP_ROW
};

static const uint8_t code g_rtcClockDigitLeftCols[RTCCLOCK_DIGIT_COUNT_PER_LINE] =
{
    0U,
    4U,
    8U,
    12U
};

static RtcClock_Time_t xdata g_rtcClockTime;
static uint16_t xdata g_rtcClockElapsedMs = 0U;
static uint16_t xdata g_rtcClockAnimElapsedMs = 0U;
static uint8_t xdata g_rtcClockAnimPhase = 0U;
static uint8_t g_rtcClockValid = 0U;

static uint8_t RtcClock_GetMonthDays(uint8_t year, uint8_t month)
{
    static const uint8_t code monthDays[12] =
    {
        31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U
    };
    uint8_t days;

    if ((month == 0U) || (month > 12U))
    {
        return 0U;
    }

    days = monthDays[month - 1U];
    if ((month == 2U) && ((year & 0x03U) == 0U))
    {
        days = 29U;
    }

    return days;
}

static uint8_t RtcClock_IsTimeValid(const RtcClock_Time_t *time)
{
    uint8_t monthDays;

    if (time == 0)
    {
        return 0U;
    }
    if ((time->month == 0U) || (time->month > 12U))
    {
        return 0U;
    }
    monthDays = RtcClock_GetMonthDays(time->year, time->month);
    if ((time->day == 0U) || (time->day > monthDays))
    {
        return 0U;
    }
    if (time->hour > 23U)
    {
        return 0U;
    }
    if (time->minute > 59U)
    {
        return 0U;
    }
    if (time->second > 59U)
    {
        return 0U;
    }

    return 1U;
}

static uint8_t RtcClock_LoadFromPayload(const GpMatrixTimeSyncPayload xdata *syncPayload,
                                        RtcClock_Time_t *time)
{
    if ((syncPayload == 0) || (time == 0))
    {
        return 0U;
    }

    time->year = syncPayload->year;
    time->month = syncPayload->month;
    time->day = syncPayload->day;
    time->hour = syncPayload->hour;
    time->minute = syncPayload->minute;
    time->second = syncPayload->second;
    return RtcClock_IsTimeValid(time);
}

static void RtcClock_UpdateCache(const RtcClock_Time_t *time)
{
    g_rtcClockTime = *time;
    g_rtcClockValid = 1U;
}

static void RtcClock_AdvanceOneSecond(void)
{
    uint8_t monthDays;

    g_rtcClockTime.second++;
    if (g_rtcClockTime.second < RTCCLOCK_SECONDS_PER_MINUTE)
    {
        return;
    }

    g_rtcClockTime.second = 0U;
    g_rtcClockTime.minute++;
    if (g_rtcClockTime.minute < RTCCLOCK_MINUTES_PER_HOUR)
    {
        return;
    }

    g_rtcClockTime.minute = 0U;
    g_rtcClockTime.hour++;
    if (g_rtcClockTime.hour < RTCCLOCK_HOURS_PER_DAY)
    {
        return;
    }

    g_rtcClockTime.hour = 0U;
    g_rtcClockTime.day++;
    monthDays = RtcClock_GetMonthDays(g_rtcClockTime.year, g_rtcClockTime.month);
    if ((monthDays != 0U) && (g_rtcClockTime.day <= monthDays))
    {
        return;
    }

    g_rtcClockTime.day = 1U;
    g_rtcClockTime.month++;
    if (g_rtcClockTime.month <= 12U)
    {
        return;
    }

    g_rtcClockTime.month = 1U;
    g_rtcClockTime.year++;
}

uint8_t RtcClock_Init(void)
{
    g_rtcClockElapsedMs = 0U;
    g_rtcClockAnimElapsedMs = 0U;
    g_rtcClockAnimPhase = 0U;
    return g_rtcClockValid;
}

uint8_t RtcClock_SyncTime(const GpMatrixTimeSyncPayload xdata *syncPayload)
{
    RtcClock_Time_t time;

    if (RtcClock_LoadFromPayload(syncPayload, &time) == 0U)
    {
        return 0U;
    }

    RtcClock_UpdateCache(&time);
    g_rtcClockElapsedMs = 0U;
    g_rtcClockAnimElapsedMs = 0U;
    return 1U;
}

uint8_t RtcClock_Task(uint16_t elapsedMs)
{
    uint32_t totalMs;
    uint32_t animMs;
    uint8_t changed;

    if (g_rtcClockValid == 0U)
    {
        return 0U;
    }

    totalMs = (uint32_t)g_rtcClockElapsedMs + (uint32_t)elapsedMs;
    changed = 0U;
    while (totalMs >= RTCCLOCK_MS_PER_SECOND)
    {
        totalMs -= RTCCLOCK_MS_PER_SECOND;
        RtcClock_AdvanceOneSecond();
        changed = 1U;
    }

    g_rtcClockElapsedMs = (uint16_t)totalMs;

    animMs = (uint32_t)g_rtcClockAnimElapsedMs + (uint32_t)elapsedMs;
    while (animMs >= RTCCLOCK_ANIM_STEP_MS)
    {
        animMs -= RTCCLOCK_ANIM_STEP_MS;
        g_rtcClockAnimPhase++;
        if (g_rtcClockAnimPhase >= RTCCLOCK_ANIM_PHASE_COUNT)
        {
            g_rtcClockAnimPhase = 0U;
        }
        changed = 1U;
    }

    g_rtcClockAnimElapsedMs = (uint16_t)animMs;
    return changed;
}

static uint8_t RtcClock_IsDigitPixelOn(uint8_t digitValue, uint8_t row, uint8_t col)
{
    uint8_t rowBits;

    if ((digitValue >= 10U) || (row >= RTCCLOCK_DIGIT_HEIGHT) || (col >= RTCCLOCK_DIGIT_WIDTH))
    {
        return 0U;
    }

    rowBits = g_rtcClockDigitRows[digitValue][row];
    if ((rowBits & (uint8_t)(1U << (RTCCLOCK_DIGIT_WIDTH - 1U - col))) != 0U)
    {
        return 1U;
    }

    return 0U;
}

static uint8_t RtcClock_GetDigitPixel(uint8_t digitValue,
                                      uint8_t digitTopRow,
                                      uint8_t digitLeftCol,
                                      uint8_t row,
                                      uint8_t col)
{
    uint8_t digitRow;
    uint8_t digitCol;

    if ((row < digitTopRow)
        || (row >= (uint8_t)(digitTopRow + RTCCLOCK_DIGIT_HEIGHT))
        || (col < digitLeftCol)
        || (col >= (uint8_t)(digitLeftCol + RTCCLOCK_DIGIT_WIDTH)))
    {
        return 0U;
    }

    digitRow = (uint8_t)(row - digitTopRow);
    digitCol = (uint8_t)(col - digitLeftCol);
    return RtcClock_IsDigitPixelOn(digitValue, digitRow, digitCol);
}

static uint8_t RtcClock_GetFourDigitPixel(uint16_t value,
                                          uint8_t lineIndex,
                                          uint8_t row,
                                          uint8_t col)
{
    uint8_t digitTopRow;
    uint8_t digitValue;

    if (lineIndex >= RTCCLOCK_LINE_COUNT)
    {
        return 0U;
    }

    digitTopRow = g_rtcClockLineTopRows[lineIndex];

    digitValue = (uint8_t)((value / 1000U) % 10U);
    if (RtcClock_GetDigitPixel(digitValue, digitTopRow, g_rtcClockDigitLeftCols[0], row, col) != 0U)
    {
        return 1U;
    }

    digitValue = (uint8_t)((value / 100U) % 10U);
    if (RtcClock_GetDigitPixel(digitValue, digitTopRow, g_rtcClockDigitLeftCols[1], row, col) != 0U)
    {
        return 1U;
    }

    digitValue = (uint8_t)((value / 10U) % 10U);
    if (RtcClock_GetDigitPixel(digitValue, digitTopRow, g_rtcClockDigitLeftCols[2], row, col) != 0U)
    {
        return 1U;
    }

    digitValue = (uint8_t)(value % 10U);
    if (RtcClock_GetDigitPixel(digitValue, digitTopRow, g_rtcClockDigitLeftCols[3], row, col) != 0U)
    {
        return 1U;
    }

    return 0U;
}

static uint8_t RtcClock_GetDateSeparatorPixel(uint8_t row, uint8_t col)
{
    if ((row == RTCCLOCK_DATE_SEPARATOR_ROW) && (col == RTCCLOCK_SEPARATOR_COL))
    {
        return 1U;
    }

    return 0U;
}

static uint8_t RtcClock_GetTimeSeparatorPixel(uint8_t row, uint8_t col)
{
    if ((g_rtcClockTime.second & 0x01U) != 0U)
    {
        return 0U;
    }

    if (col != RTCCLOCK_SEPARATOR_COL)
    {
        return 0U;
    }

    if ((row == RTCCLOCK_TIME_SEPARATOR_ROW0) || (row == RTCCLOCK_TIME_SEPARATOR_ROW1))
    {
        return 1U;
    }

    return 0U;
}

static uint8_t RtcClock_GetProgressPixel(uint8_t row, uint8_t col)
{
    uint8_t litIndex;

    if ((row != RTCCLOCK_PROGRESS_ROW) || (col >= RTCCLOCK_PROGRESS_COL_COUNT))
    {
        return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
    }

    litIndex = (uint8_t)(g_rtcClockTime.second / RTCCLOCK_PROGRESS_STEP_SECONDS);
    if (col <= litIndex)
    {
        return (uint8_t)RTCCLOCK_PROGRESS_RGB332;
    }

    return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
}

static uint8_t RtcClock_GetAnimPixel(uint8_t row, uint8_t col)
{
    uint8_t tailPhase;

    if (col != RTCCLOCK_ANIM_COL)
    {
        return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
    }

    if (row == g_rtcClockAnimPhase)
    {
        return (uint8_t)RTCCLOCK_ANIM_HEAD_RGB332;
    }

    tailPhase = (uint8_t)((g_rtcClockAnimPhase == 0U)
        ? (RTCCLOCK_ANIM_PHASE_COUNT - 1U)
        : (g_rtcClockAnimPhase - 1U));
    if (row == tailPhase)
    {
        return (uint8_t)RTCCLOCK_ANIM_TAIL_RGB332;
    }

    return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
}

static uint8_t RtcClock_MapStorageRow(uint8_t row)
{
    return (uint8_t)((LOCALDISPLAY_ASSET_ROWS - 1U) - row);
}

uint8_t RtcClock_GetPixel(uint8_t row, uint8_t col)
{
    uint8_t storageRow;
    uint16_t yearValue;
    uint16_t monthDayValue;
    uint16_t timeValue;
    uint8_t packed;

    if ((row >= LOCALDISPLAY_ASSET_ROWS) || (col >= LOCALDISPLAY_ASSET_COLS) || (g_rtcClockValid == 0U))
    {
        return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
    }

    /* Match the existing text/image buffer convention where row 15 is the visual top row. */
    storageRow = RtcClock_MapStorageRow(row);

    yearValue = (uint16_t)(RTCCLOCK_YEAR_BASE + g_rtcClockTime.year);
    if (RtcClock_GetFourDigitPixel(yearValue, 0U, storageRow, col) != 0U)
    {
        return (uint8_t)RTCCLOCK_YEAR_RGB332;
    }

    monthDayValue = (uint16_t)g_rtcClockTime.month * 100U + (uint16_t)g_rtcClockTime.day;
    if (RtcClock_GetFourDigitPixel(monthDayValue, 1U, storageRow, col) != 0U)
    {
        return (uint8_t)RTCCLOCK_DATE_RGB332;
    }
    if (RtcClock_GetDateSeparatorPixel(storageRow, col) != 0U)
    {
        return (uint8_t)RTCCLOCK_DATE_RGB332;
    }

    timeValue = (uint16_t)g_rtcClockTime.hour * 100U + (uint16_t)g_rtcClockTime.minute;
    if (RtcClock_GetFourDigitPixel(timeValue, 2U, storageRow, col) != 0U)
    {
        return (uint8_t)RTCCLOCK_TIME_RGB332;
    }
    if (RtcClock_GetTimeSeparatorPixel(storageRow, col) != 0U)
    {
        return (uint8_t)RTCCLOCK_TIME_RGB332;
    }

    packed = RtcClock_GetProgressPixel(storageRow, col);
    if (packed != (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332)
    {
        return packed;
    }

    packed = RtcClock_GetAnimPixel(storageRow, col);
    if (packed != (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332)
    {
        return packed;
    }

    return (uint8_t)LOCALDISPLAY_ASSET_BG_RGB332;
}