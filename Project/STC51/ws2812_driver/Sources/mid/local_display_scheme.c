/*
 * @file local_display_scheme.c
 * @author GitHub Copilot
 * @date 2026-05-11
 * @version 1.0
 * @brief Local startup display scheme and offline key-driven display control.
 *
 * Local preset patterns used by the offline startup carousel:
 * 1. Diamond
 * 2. Cross
 * 3. Checker
 * 4. Border
 * 5. Diagonal X
 * 6. JLU emblem
 * 7. Last AI bitmap snapshot
 *
 * Local scroll glyph resources used by the offline subtitle path:
 * 1. 吉
 * 2. 林
 * 3. 大
 * 4. 学
 * 5. JLU emblem
 */

#include "config.h"
#include "local_display_scheme.h"

#include "draw_drv.h"
#include "gp_led_action.h"
#include "offline_pattern.h"
#include "app.h"
#include "local_display_assets.h"

#define LOCALDISPLAY_TASK_PERIOD_MS            10U
#define LOCALDISPLAY_SEQUENCE_STEP_MS_DEFAULT  2000U
#define LOCALDISPLAY_BRIGHTNESS_DEFAULT        200U
#define LOCALDISPLAY_GRADIENT_SPAN_DEFAULT     160U
#define LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS    64U
#define LOCALDISPLAY_FRAME_INTERVAL_SCROLL_MS  96U
#define LOCALDISPLAY_FRAME_INTERVAL_COLOR_MS   80U
#define LOCALDISPLAY_FRAME_INTERVAL_CLOCK_MS   128U
#define LOCALDISPLAY_TEXT_GLYPH_BASE           0U
#define LOCALDISPLAY_TEXT_ONLY_COUNT           4U
#define LOCALDISPLAY_TEXT_WITH_EMBLEM_COUNT    5U
#define LOCALDISPLAY_TEXT_GLYPH_COUNT_MAX      LOCALDISPLAY_TEXT_WITH_EMBLEM_COUNT
#define LOCALDISPLAY_BLUE_THEME_INDEX          4U
#define LOCALDISPLAY_MANUAL_EFFECT_COUNT       11U
#define LOCALDISPLAY_CLOCK_EFFECT_COUNT        7U
#define LOCALDISPLAY_COLOR_THEME_COUNT         7U

#define LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT   0x01U
#define LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT 0x02U

#define LOCALDISPLAY_TIMELINE_NONE              0U
#define LOCALDISPLAY_TIMELINE_BREATH            1U
#define LOCALDISPLAY_TIMELINE_FADE              2U
#define LOCALDISPLAY_TIMELINE_ROW_REVEAL        3U
#define LOCALDISPLAY_TIMELINE_ROW_HIDE          4U

#define LOCALDISPLAY_EFFECT_CMD(effectId, scrollStep, animStep, frameIntervalMs, gradientSpan, flags, timelineId) \
    {(uint8_t)(effectId), (uint8_t)(scrollStep), (uint8_t)(animStep), (uint8_t)(frameIntervalMs), \
     (uint8_t)(gradientSpan), (uint8_t)(flags), (uint8_t)(timelineId)}

#define LOCALDISPLAY_PLAY_ITEM(imageId, colorIndex, effectId, scrollStep, animStep, frameIntervalMs, gradientSpan, \
                               flags, timelineId) \
    {(uint8_t)(imageId), (uint8_t)(colorIndex), \
     LOCALDISPLAY_EFFECT_CMD(effectId, scrollStep, animStep, frameIntervalMs, gradientSpan, flags, timelineId)}

typedef struct
{
    uint8_t primaryR;
    uint8_t primaryG;
    uint8_t primaryB;
} LocalDisplaySchemeColor_t;

typedef struct
{
    uint16_t durationMs;
    uint16_t delayMs;
    uint8_t repeatCount;
    uint8_t timelinePath;
} LocalDisplayTimelineProfile_t;

typedef struct
{
    uint8_t effectId;
    uint8_t scrollStep;
    uint8_t animStep;
    uint8_t frameIntervalMs;
    uint8_t gradientSpan;
    uint8_t flags;
    uint8_t timelineId;
} LocalDisplayEffectCommand_t;

typedef struct
{
    uint8_t imageId;
    uint8_t colorIndex;
    LocalDisplayEffectCommand_t effectCmd;
} LocalDisplayPlayItem_t;

typedef enum
{
    LOCALDISPLAY_IMAGE_DIAMOND = OFFLINE_PATTERN_IDX_DIAMOND,
    LOCALDISPLAY_IMAGE_CROSS = OFFLINE_PATTERN_IDX_CROSS,
    LOCALDISPLAY_IMAGE_JLU_EMBLEM = OFFLINE_PATTERN_IDX_JLU_EMBLEM,
    LOCALDISPLAY_IMAGE_CHECKER = OFFLINE_PATTERN_IDX_CHECKER,
    LOCALDISPLAY_IMAGE_BORDER = OFFLINE_PATTERN_IDX_BORDER,
    LOCALDISPLAY_IMAGE_DIAGONAL_X = OFFLINE_PATTERN_IDX_DIAGONAL_X,
    LOCALDISPLAY_IMAGE_REMOTE_PATTERN = OFFLINE_PATTERN_COUNT,
    LOCALDISPLAY_IMAGE_TEXT_ONLY = OFFLINE_PATTERN_COUNT + 1U,
    LOCALDISPLAY_IMAGE_TEXT_WITH_EMBLEM = OFFLINE_PATTERN_COUNT + 2U,
    LOCALDISPLAY_IMAGE_RTC_CLOCK = OFFLINE_PATTERN_COUNT + 3U,
    LOCALDISPLAY_IMAGE_COUNT
} LocalDisplayImageId_t;

typedef struct
{
    uint8_t autoActive;
    uint8_t autoStep;
    uint16_t autoElapsedMs;
    uint16_t autoStepDurationMs;
    uint8_t patternIndex;
    uint8_t imageId;
    uint8_t effectIndex;
    uint8_t colorIndex;
} LocalDisplaySchemeState_t;

static const LocalDisplayTimelineProfile_t code g_localDisplayTimelineProfiles[] =
{
    {0U, 0U, 0U, DRAWDRV_TIMELINE_PATH_LINEAR},
    {1600U, 240U, 0xFFU, DRAWDRV_TIMELINE_PATH_BREATH_CURVE},
    {1200U, 200U, 0xFFU, DRAWDRV_TIMELINE_PATH_EASE_IN_OUT},
    {960U, 120U, 0xFFU, DRAWDRV_TIMELINE_PATH_LINEAR},
    {960U, 120U, 0xFFU, DRAWDRV_TIMELINE_PATH_LINEAR}
};

static const LocalDisplayEffectCommand_t code g_localDisplayManualEffects[LOCALDISPLAY_MANUAL_EFFECT_COUNT] =
{
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_STATIC, 1U, 1U, DRAWDRV_FRAME_INTERVAL_MS_DEFAULT,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_BREATH, 1U, 2U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_BREATH),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_GRADIENT, 1U, 2U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            180U, LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT | LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT,
                            LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_SCROLL_LEFT, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_SCROLL_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_SCROLL_RIGHT, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_SCROLL_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_FADE_IN, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_FADE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_FADE_OUT, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_FADE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_COLOR_CYCLE, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_COLOR_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_ROW_REVEAL, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_ROW_REVEAL),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_ROW_HIDE, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_ROW_HIDE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_GRADIENT_REVEAL, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            180U, LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT | LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT,
                            LOCALDISPLAY_TIMELINE_ROW_REVEAL)
};

static const LocalDisplayEffectCommand_t code g_localDisplayClockEffects[LOCALDISPLAY_CLOCK_EFFECT_COUNT] =
{
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_STATIC, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_CLOCK_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_BREATH, 1U, 2U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_BREATH),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_GRADIENT, 1U, 2U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            180U, LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT | LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT,
                            LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_FADE_IN, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_FADE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_COLOR_CYCLE, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_COLOR_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_ROW_REVEAL, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_ROW_REVEAL),
    LOCALDISPLAY_EFFECT_CMD(DRAWDRV_EFFECT_GRADIENT_REVEAL, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                            180U, LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT | LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT,
                            LOCALDISPLAY_TIMELINE_ROW_REVEAL)
};

static const LocalDisplaySchemeColor_t g_localDisplayColors[LOCALDISPLAY_COLOR_THEME_COUNT] =
{
    {0xFFU, 0x30U, 0x30U},
    {0x30U, 0xFFU, 0x30U},
    {0xFFU, 0xD0U, 0x20U},
    {0x20U, 0xFFU, 0xFFU},
    {0x20U, 0x60U, 0xFFU},
    {0xFFU, 0x30U, 0xFFU},
    {0xFFU, 0xFFU, 0xFFU}
};

/* Auto-playlist items now store image id plus an explicit effect command descriptor. */
static const LocalDisplayPlayItem_t code g_localDisplayAutoPlaylist[] =
{
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_DIAMOND, 0U, DRAWDRV_EFFECT_STATIC, 1U, 1U,
                           DRAWDRV_FRAME_INTERVAL_MS_DEFAULT, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_CROSS, 1U, DRAWDRV_EFFECT_STATIC, 1U, 1U,
                           DRAWDRV_FRAME_INTERVAL_MS_DEFAULT, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_CHECKER, 2U, DRAWDRV_EFFECT_STATIC, 1U, 1U,
                           DRAWDRV_FRAME_INTERVAL_MS_DEFAULT, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_BORDER, 3U, DRAWDRV_EFFECT_STATIC, 1U, 1U,
                           DRAWDRV_FRAME_INTERVAL_MS_DEFAULT, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_DIAGONAL_X, 5U, DRAWDRV_EFFECT_STATIC, 1U, 1U,
                           DRAWDRV_FRAME_INTERVAL_MS_DEFAULT, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX, DRAWDRV_EFFECT_STATIC,
                           1U, 1U, DRAWDRV_FRAME_INTERVAL_MS_DEFAULT, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX, DRAWDRV_EFFECT_BREATH,
                           1U, 2U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_BREATH),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX, DRAWDRV_EFFECT_GRADIENT,
                           1U, 2U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS, 180U,
                           LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT | LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT,
                           LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX, DRAWDRV_EFFECT_ROW_REVEAL,
                           1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_ROW_REVEAL),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX, DRAWDRV_EFFECT_ROW_HIDE,
                           1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_ROW_HIDE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_GRADIENT_REVEAL, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS, 180U,
                           LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT | LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT,
                           LOCALDISPLAY_TIMELINE_ROW_REVEAL),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_SCROLL_LEFT, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_SCROLL_MS,
                           LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_SCROLL_RIGHT, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_SCROLL_MS,
                           LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX, DRAWDRV_EFFECT_FADE_IN,
                           1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_FADE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX, DRAWDRV_EFFECT_FADE_OUT,
                           1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS, LOCALDISPLAY_GRADIENT_SPAN_DEFAULT,
                           0U, LOCALDISPLAY_TIMELINE_FADE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_JLU_EMBLEM, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_COLOR_CYCLE, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_COLOR_MS,
                           LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_TEXT_ONLY, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_TEXT_SCROLL_JLU, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_SCROLL_MS,
                           LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_RTC_CLOCK, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_ROW_REVEAL, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                           LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_ROW_REVEAL),
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_RTC_CLOCK, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_BREATH, 1U, 2U, LOCALDISPLAY_FRAME_INTERVAL_ANIM_MS,
                           LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_BREATH)
};

static const LocalDisplayPlayItem_t code g_localDisplayFinalPlaylistItem =
    LOCALDISPLAY_PLAY_ITEM(LOCALDISPLAY_IMAGE_RTC_CLOCK, LOCALDISPLAY_BLUE_THEME_INDEX,
                           DRAWDRV_EFFECT_STATIC, 1U, 1U, LOCALDISPLAY_FRAME_INTERVAL_CLOCK_MS,
                           LOCALDISPLAY_GRADIENT_SPAN_DEFAULT, 0U, LOCALDISPLAY_TIMELINE_NONE);

static LocalDisplaySchemeState_t g_localDisplayState;

static uint8_t LocalDisplayScheme_IsPatternImage(LocalDisplayImageId_t imageId)
{
    if (imageId < (LocalDisplayImageId_t)OFFLINE_PATTERN_COUNT)
    {
        return 1U;
    }

    if ((imageId == LOCALDISPLAY_IMAGE_REMOTE_PATTERN) && (DrawDrv_HasRemotePattern() != 0U))
    {
        return 1U;
    }

    return 0U;
}

static uint8_t LocalDisplayScheme_IsClockImage(LocalDisplayImageId_t imageId)
{
    return (uint8_t)(imageId == LOCALDISPLAY_IMAGE_RTC_CLOCK);
}

static uint8_t LocalDisplayScheme_IsPatternSelectableImage(LocalDisplayImageId_t imageId)
{
    if (imageId <= LOCALDISPLAY_IMAGE_REMOTE_PATTERN)
    {
        return 1U;
    }

    return 0U;
}

static uint8_t LocalDisplayScheme_GetPatternCycleCount(void)
{
    uint8_t patternCount;

    patternCount = OFFLINE_PATTERN_COUNT;
    if (DrawDrv_HasRemotePattern() != 0U)
    {
        patternCount++;
    }

    return patternCount;
}

static uint8_t LocalDisplayScheme_IsTextImage(LocalDisplayImageId_t imageId)
{
    if ((imageId == LOCALDISPLAY_IMAGE_TEXT_ONLY) || (imageId == LOCALDISPLAY_IMAGE_TEXT_WITH_EMBLEM))
    {
        return 1U;
    }

    return 0U;
}

static void LocalDisplayScheme_SetCurrentImage(LocalDisplayImageId_t imageId)
{
    g_localDisplayState.imageId = imageId;
    if (LocalDisplayScheme_IsPatternSelectableImage(imageId) != 0U)
    {
        g_localDisplayState.patternIndex = (uint8_t)imageId;
    }
}

static uint8_t LocalDisplayScheme_IsLocalDisplayAllowed(void)
{
    /* Manual key presses now always reclaim the local render path, even after a host image has taken over. */
    return 1U;
}

static void LocalDisplayScheme_StopAutoSequence(void)
{
    g_localDisplayState.autoActive = 0U;
    g_localDisplayState.autoStep = 0U;
    g_localDisplayState.autoElapsedMs = 0U;
    g_localDisplayState.autoStepDurationMs = 0U;
}

static void LocalDisplayScheme_BeginManualSelection(void)
{
    LocalDisplayScheme_StopAutoSequence();
    APP_ReleaseRemoteModeForLocalDisplay();
}

static void LocalDisplayScheme_RequestRemotePatternRefresh(LocalDisplayImageId_t imageId)
{
    if (imageId != LOCALDISPLAY_IMAGE_REMOTE_PATTERN)
    {
        return;
    }

    if (DrawDrv_HasRemotePattern() == 0U)
    {
        return;
    }

    (void)APP_RequestRemoteCachedBitmap();
}

static const LocalDisplayTimelineProfile_t code *LocalDisplayScheme_GetTimelineProfile(uint8_t timelineId)
{
    uint8_t timelineCount;

    timelineCount = (uint8_t)(sizeof(g_localDisplayTimelineProfiles) / sizeof(g_localDisplayTimelineProfiles[0]));
    if (timelineId >= timelineCount)
    {
        timelineId = LOCALDISPLAY_TIMELINE_NONE;
    }

    return &g_localDisplayTimelineProfiles[timelineId];
}

static uint8_t LocalDisplayScheme_IsFiniteAutoTimelineEffect(uint8_t effectId)
{
    if ((effectId == DRAWDRV_EFFECT_BREATH)
        || (effectId == DRAWDRV_EFFECT_FADE_IN)
        || (effectId == DRAWDRV_EFFECT_FADE_OUT)
        || (effectId == DRAWDRV_EFFECT_ROW_REVEAL)
        || (effectId == DRAWDRV_EFFECT_ROW_HIDE)
        || (effectId == DRAWDRV_EFFECT_GRADIENT_REVEAL))
    {
        return 1U;
    }

    return 0U;
}

static uint16_t LocalDisplayScheme_GcdU16(uint16_t lhs, uint16_t rhs)
{
    uint16_t remainder;

    while (rhs != 0U)
    {
        remainder = (uint16_t)(lhs % rhs);
        lhs = rhs;
        rhs = remainder;
    }

    return lhs;
}

static uint16_t LocalDisplayScheme_GetTextScrollWidth(LocalDisplayImageId_t imageId)
{
    uint8_t glyphCount;

    glyphCount = LOCALDISPLAY_TEXT_ONLY_COUNT;
    if (imageId == LOCALDISPLAY_IMAGE_TEXT_WITH_EMBLEM)
    {
        glyphCount = LOCALDISPLAY_TEXT_WITH_EMBLEM_COUNT;
    }

    return (uint16_t)glyphCount * (uint16_t)(LOCALDISPLAY_SCROLL_GLYPH_WIDTH + LOCALDISPLAY_SCROLL_GLYPH_SPACING);
}

static uint16_t LocalDisplayScheme_GetScrollCycleColumns(LocalDisplayImageId_t imageId,
                                                         const LocalDisplayEffectCommand_t code *effectCmd)
{
    if (effectCmd->effectId == DRAWDRV_EFFECT_TEXT_SCROLL_JLU)
    {
        return (uint16_t)(LocalDisplayScheme_GetTextScrollWidth(imageId) + LOCALDISPLAY_ASSET_COLS);
    }

    if ((effectCmd->effectId == DRAWDRV_EFFECT_SCROLL_LEFT)
        || (effectCmd->effectId == DRAWDRV_EFFECT_SCROLL_RIGHT))
    {
        if (LocalDisplayScheme_IsPatternImage(imageId) != 0U)
        {
            return OFFLINE_PATTERN_COLS;
        }
    }

    return 0U;
}

static const LocalDisplayEffectCommand_t code *LocalDisplayScheme_GetManualEffectTable(LocalDisplayImageId_t imageId,
                                                                                       uint8_t *effectCount)
{
    if (effectCount == 0)
    {
        return g_localDisplayManualEffects;
    }

    if (LocalDisplayScheme_IsClockImage(imageId) != 0U)
    {
        *effectCount = LOCALDISPLAY_CLOCK_EFFECT_COUNT;
        return g_localDisplayClockEffects;
    }

    *effectCount = LOCALDISPLAY_MANUAL_EFFECT_COUNT;
    return g_localDisplayManualEffects;
}

static uint16_t LocalDisplayScheme_GetAutoStepDurationMs(const LocalDisplayPlayItem_t code *playItem)
{
    const LocalDisplayTimelineProfile_t code *timelineProfile;
    uint16_t frameIntervalMs;
    uint16_t cycleColumns;
    uint16_t cycleGcd;
    uint16_t cycleSteps;
    uint32_t totalMs;
    uint8_t scrollStep;

    if (playItem == 0)
    {
        return LOCALDISPLAY_SEQUENCE_STEP_MS_DEFAULT;
    }

    frameIntervalMs = DrawDrv_NormalizeFrameIntervalMs(playItem->effectCmd.frameIntervalMs);
    cycleColumns = LocalDisplayScheme_GetScrollCycleColumns((LocalDisplayImageId_t)playItem->imageId,
                                                            &playItem->effectCmd);
    if (cycleColumns != 0U)
    {
        scrollStep = playItem->effectCmd.scrollStep;
        if (scrollStep == 0U)
        {
            scrollStep = 1U;
        }

        cycleGcd = LocalDisplayScheme_GcdU16(cycleColumns, (uint16_t)scrollStep);
        if (cycleGcd == 0U)
        {
            return LOCALDISPLAY_SEQUENCE_STEP_MS_DEFAULT;
        }

        cycleSteps = (uint16_t)(cycleColumns / cycleGcd);
        if ((playItem->effectCmd.effectId == DRAWDRV_EFFECT_TEXT_SCROLL_JLU) && (cycleSteps > 1U))
        {
            cycleSteps--;
        }

        totalMs = (uint32_t)cycleSteps * (uint32_t)frameIntervalMs;
        if (totalMs > 65535UL)
        {
            return 65535U;
        }

        return (uint16_t)totalMs;
    }

    if ((LocalDisplayScheme_IsFiniteAutoTimelineEffect(playItem->effectCmd.effectId) != 0U)
        && (playItem->effectCmd.timelineId != LOCALDISPLAY_TIMELINE_NONE))
    {
        timelineProfile = LocalDisplayScheme_GetTimelineProfile(playItem->effectCmd.timelineId);
        totalMs = (uint32_t)timelineProfile->delayMs
                  + (uint32_t)timelineProfile->durationMs
                  + (uint32_t)frameIntervalMs;
        if (totalMs > 65535UL)
        {
            return 65535U;
        }

        return (uint16_t)totalMs;
    }

    return LOCALDISPLAY_SEQUENCE_STEP_MS_DEFAULT;
}

static void LocalDisplayScheme_ClampAutoTimelineRepeat(DrawDrv_RenderConfig_t *renderCfg)
{
    if (renderCfg == 0)
    {
        return;
    }

    if ((renderCfg->timelineDurationMs == 0U)
        || (LocalDisplayScheme_IsFiniteAutoTimelineEffect((uint8_t)renderCfg->effect) == 0U))
    {
        return;
    }

    /* Auto-play should finish on the effect's terminal frame instead of wrapping into a partial second cycle. */
    renderCfg->timelineRepeatCount = 0U;
}

static void LocalDisplayScheme_LoadBaseRenderConfig(DrawDrv_RenderConfig_t *renderCfg)
{
    renderCfg->fgR = 0xFFU;
    renderCfg->fgG = 0xFFU;
    renderCfg->fgB = 0xFFU;
    renderCfg->bgR = 0x00U;
    renderCfg->bgG = 0x00U;
    renderCfg->bgB = 0x00U;
    renderCfg->brightness = LOCALDISPLAY_BRIGHTNESS_DEFAULT;
    renderCfg->contentType = DRAWDRV_CONTENT_PATTERN;
    renderCfg->colorMode = DRAWDRV_COLOR_SOLID;
    renderCfg->direction = DRAWDRV_DIR_NORMAL;
    renderCfg->useGradient = 0U;
    renderCfg->gradientSpan = LOCALDISPLAY_GRADIENT_SPAN_DEFAULT;
    renderCfg->scrollStep = 1U;
    renderCfg->animStep = 1U;
    renderCfg->effect = DRAWDRV_EFFECT_STATIC;
    renderCfg->frameIntervalMs = DRAWDRV_FRAME_INTERVAL_MS_DEFAULT;
    renderCfg->timelineDurationMs = 0U;
    renderCfg->timelineRepeatDelayMs = 0U;
    renderCfg->timelineRepeatCount = 0U;
    renderCfg->timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;
}

static void LocalDisplayScheme_ApplyColorTheme(DrawDrv_RenderConfig_t *renderCfg, uint8_t colorIndex)
{
    const LocalDisplaySchemeColor_t *colorTheme;

    colorTheme = &g_localDisplayColors[colorIndex % LOCALDISPLAY_COLOR_THEME_COUNT];
    renderCfg->fgR = colorTheme->primaryR;
    renderCfg->fgG = colorTheme->primaryG;
    renderCfg->fgB = colorTheme->primaryB;

    /* Keep every local preset on a black base so the new compact single-layer assets stay visually consistent. */
    renderCfg->bgR = 0x00U;
    renderCfg->bgG = 0x00U;
    renderCfg->bgB = 0x00U;
}

static void LocalDisplayScheme_ApplyEffectCommand(DrawDrv_RenderConfig_t *renderCfg,
                                                  const LocalDisplayEffectCommand_t code *effectCmd)
{
    const LocalDisplayTimelineProfile_t code *timelineProfile;

    renderCfg->effect = (DrawDrv_Effect_t)effectCmd->effectId;
    renderCfg->colorMode = DRAWDRV_COLOR_SOLID;
    renderCfg->useGradient = 0U;
    renderCfg->gradientSpan = LOCALDISPLAY_GRADIENT_SPAN_DEFAULT;
    renderCfg->scrollStep = 1U;
    renderCfg->animStep = 1U;
    renderCfg->frameIntervalMs = DRAWDRV_FRAME_INTERVAL_MS_DEFAULT;
    renderCfg->timelineDurationMs = 0U;
    renderCfg->timelineRepeatDelayMs = 0U;
    renderCfg->timelineRepeatCount = 0U;
    renderCfg->timelinePath = DRAWDRV_TIMELINE_PATH_LINEAR;

    if (effectCmd->scrollStep != 0U)
    {
        renderCfg->scrollStep = effectCmd->scrollStep;
    }

    if (effectCmd->animStep != 0U)
    {
        renderCfg->animStep = effectCmd->animStep;
    }

    if (effectCmd->frameIntervalMs != 0U)
    {
        renderCfg->frameIntervalMs = effectCmd->frameIntervalMs;
    }

    if (effectCmd->gradientSpan != 0U)
    {
        renderCfg->gradientSpan = effectCmd->gradientSpan;
    }

    if ((effectCmd->flags & LOCALDISPLAY_EFFECT_FLAG_USE_GRADIENT) != 0U)
    {
        renderCfg->useGradient = 1U;
    }

    if ((effectCmd->flags & LOCALDISPLAY_EFFECT_FLAG_COLOR_GRADIENT) != 0U)
    {
        renderCfg->colorMode = DRAWDRV_COLOR_GRADIENT;
    }

    if (effectCmd->timelineId == LOCALDISPLAY_TIMELINE_NONE)
    {
        return;
    }

    timelineProfile = LocalDisplayScheme_GetTimelineProfile(effectCmd->timelineId);
    renderCfg->timelineDurationMs = timelineProfile->durationMs;
    renderCfg->timelineRepeatDelayMs = timelineProfile->delayMs;
    renderCfg->timelineRepeatCount = timelineProfile->repeatCount;
    renderCfg->timelinePath = (DrawDrv_TimelinePath_t)timelineProfile->timelinePath;
}

static void LocalDisplayScheme_ApplyPatternEffect(uint8_t patternId,
                                                  const LocalDisplayEffectCommand_t code *effectCmd,
                                                  uint8_t colorIndex,
                                                  uint8_t autoOneShot)
{
    DrawDrv_RenderConfig_t renderCfg;

    LocalDisplayScheme_LoadBaseRenderConfig(&renderCfg);
    LocalDisplayScheme_ApplyEffectCommand(&renderCfg, effectCmd);
    if (autoOneShot != 0U)
    {
        LocalDisplayScheme_ClampAutoTimelineRepeat(&renderCfg);
    }
    LocalDisplayScheme_ApplyColorTheme(&renderCfg, colorIndex);
    renderCfg.contentType = DRAWDRV_CONTENT_PATTERN;
    APP_ApplyLocalPatternConfig(&renderCfg, patternId);
}

static void LocalDisplayScheme_ApplyClockEffect(const LocalDisplayEffectCommand_t code *effectCmd,
                                                uint8_t colorIndex,
                                                uint8_t autoOneShot)
{
    DrawDrv_RenderConfig_t renderCfg;

    LocalDisplayScheme_LoadBaseRenderConfig(&renderCfg);
    LocalDisplayScheme_ApplyEffectCommand(&renderCfg, effectCmd);
    if (autoOneShot != 0U)
    {
        LocalDisplayScheme_ClampAutoTimelineRepeat(&renderCfg);
    }
    LocalDisplayScheme_ApplyColorTheme(&renderCfg, colorIndex);
    renderCfg.contentType = DRAWDRV_CONTENT_CLOCK;
    APP_ApplyLocalRenderConfig(&renderCfg);
}

static void LocalDisplayScheme_ApplyTextScroll(LocalDisplayImageId_t imageId,
                                               uint8_t colorIndex,
                                               uint8_t autoOneShot)
{
    DrawDrv_RenderConfig_t renderCfg;
    uint8_t glyphSequence[LOCALDISPLAY_TEXT_GLYPH_COUNT_MAX];
    uint8_t glyphCount;
    uint8_t glyphIndex;

    LocalDisplayScheme_LoadBaseRenderConfig(&renderCfg);
    renderCfg.effect = DRAWDRV_EFFECT_TEXT_SCROLL_JLU;
    renderCfg.frameIntervalMs = LOCALDISPLAY_FRAME_INTERVAL_SCROLL_MS;
    renderCfg.scrollStep = 1U;
    if (autoOneShot != 0U)
    {
        renderCfg.timelineRepeatCount = 0U;
    }
    LocalDisplayScheme_ApplyColorTheme(&renderCfg, colorIndex);
    renderCfg.contentType = DRAWDRV_CONTENT_GLYPH;

    if (imageId == LOCALDISPLAY_IMAGE_TEXT_WITH_EMBLEM)
    {
        glyphCount = LOCALDISPLAY_TEXT_WITH_EMBLEM_COUNT;
    }
    else
    {
        glyphCount = LOCALDISPLAY_TEXT_ONLY_COUNT;
    }

    for (glyphIndex = 0U; glyphIndex < glyphCount; ++glyphIndex)
    {
        glyphSequence[glyphIndex] = (uint8_t)(LOCALDISPLAY_TEXT_GLYPH_BASE + glyphIndex);
    }

    (void)APP_SetScrollGlyphSequence(glyphSequence, glyphCount);
    APP_ApplyLocalGlyphConfig(&renderCfg, 0U);
}

static void LocalDisplayScheme_ApplyImageEffect(LocalDisplayImageId_t imageId,
                                                const LocalDisplayEffectCommand_t code *effectCmd,
                                                uint8_t colorIndex,
                                                uint8_t autoOneShot)
{
    if (LocalDisplayScheme_IsPatternImage(imageId) != 0U)
    {
        LocalDisplayScheme_ApplyPatternEffect((uint8_t)imageId, effectCmd, colorIndex, autoOneShot);
        return;
    }

    if (LocalDisplayScheme_IsClockImage(imageId) != 0U)
    {
        LocalDisplayScheme_ApplyClockEffect(effectCmd, colorIndex, autoOneShot);
        return;
    }

    LocalDisplayScheme_ApplyTextScroll(imageId, colorIndex, autoOneShot);
}

static void LocalDisplayScheme_ApplyCurrentView(void)
{
    const LocalDisplayEffectCommand_t code *effectCmd;
    uint8_t effectCount;

    if ((LocalDisplayScheme_IsPatternImage(g_localDisplayState.imageId) != 0U)
        || (LocalDisplayScheme_IsClockImage(g_localDisplayState.imageId) != 0U))
    {
        effectCmd = LocalDisplayScheme_GetManualEffectTable((LocalDisplayImageId_t)g_localDisplayState.imageId,
                                                            &effectCount);
        effectCmd = &effectCmd[g_localDisplayState.effectIndex % effectCount];
        LocalDisplayScheme_ApplyImageEffect(g_localDisplayState.imageId, effectCmd, g_localDisplayState.colorIndex, 0U);
        return;
    }

    LocalDisplayScheme_ApplyTextScroll(g_localDisplayState.imageId, g_localDisplayState.colorIndex, 0U);
}

static void LocalDisplayScheme_ApplyPlaylistItem(const LocalDisplayPlayItem_t code *playItem)
{
    LocalDisplayScheme_SetCurrentImage((LocalDisplayImageId_t)playItem->imageId);
    g_localDisplayState.colorIndex = playItem->colorIndex;
    g_localDisplayState.autoStepDurationMs = LocalDisplayScheme_GetAutoStepDurationMs(playItem);
    LocalDisplayScheme_ApplyImageEffect((LocalDisplayImageId_t)playItem->imageId,
                                        &playItem->effectCmd,
                                        playItem->colorIndex,
                                        1U);
}

static void LocalDisplayScheme_ApplyAutoStep(uint8_t stepIndex)
{
    uint8_t playlistCount;

    playlistCount = (uint8_t)(sizeof(g_localDisplayAutoPlaylist) / sizeof(g_localDisplayAutoPlaylist[0]));
    if (stepIndex >= playlistCount)
    {
        return;
    }

    LocalDisplayScheme_ApplyPlaylistItem(&g_localDisplayAutoPlaylist[stepIndex]);
}

void LocalDisplayScheme_Init(void)
{
    uint8_t firstPatternIndex;

    firstPatternIndex = (uint8_t)LOCALDISPLAY_IMAGE_DIAMOND;
    g_localDisplayState.autoActive = 1U;
    g_localDisplayState.autoStep = 0U;
    g_localDisplayState.autoElapsedMs = 0U;
    g_localDisplayState.autoStepDurationMs = 0U;
    g_localDisplayState.patternIndex = firstPatternIndex;
    g_localDisplayState.imageId = (LocalDisplayImageId_t)firstPatternIndex;
    g_localDisplayState.effectIndex = 0U;
    g_localDisplayState.colorIndex = 0U;

    LocalDisplayScheme_ApplyAutoStep(0U);
}

void LocalDisplayScheme_Task10ms(void)
{
    uint16_t targetDurationMs;
    uint8_t playlistCount;

    if ((g_localDisplayState.autoActive == 0U) || (LocalDisplayScheme_IsLocalDisplayAllowed() == 0U))
    {
        return;
    }

    targetDurationMs = g_localDisplayState.autoStepDurationMs;
    if (targetDurationMs == 0U)
    {
        targetDurationMs = LOCALDISPLAY_SEQUENCE_STEP_MS_DEFAULT;
    }

    g_localDisplayState.autoElapsedMs = (uint16_t)(g_localDisplayState.autoElapsedMs + LOCALDISPLAY_TASK_PERIOD_MS);
    if (g_localDisplayState.autoElapsedMs < targetDurationMs)
    {
        return;
    }

    g_localDisplayState.autoElapsedMs = 0U;
    g_localDisplayState.autoStep++;
    playlistCount = (uint8_t)(sizeof(g_localDisplayAutoPlaylist) / sizeof(g_localDisplayAutoPlaylist[0]));
    if (g_localDisplayState.autoStep < playlistCount)
    {
        LocalDisplayScheme_ApplyAutoStep(g_localDisplayState.autoStep);
        return;
    }

    g_localDisplayState.autoActive = 0U;
    LocalDisplayScheme_ApplyPlaylistItem(&g_localDisplayFinalPlaylistItem);
}

void LocalDisplayScheme_NextPattern(void)
{
    uint8_t patternCount;

    if (LocalDisplayScheme_IsLocalDisplayAllowed() == 0U)
    {
        return;
    }

    LocalDisplayScheme_BeginManualSelection();
    patternCount = LocalDisplayScheme_GetPatternCycleCount();
    if (patternCount == 0U)
    {
        return;
    }

    g_localDisplayState.patternIndex = (uint8_t)((g_localDisplayState.patternIndex + 1U) % patternCount);
    LocalDisplayScheme_SetCurrentImage((LocalDisplayImageId_t)g_localDisplayState.patternIndex);
    LocalDisplayScheme_ApplyCurrentView();
    LocalDisplayScheme_RequestRemotePatternRefresh((LocalDisplayImageId_t)g_localDisplayState.imageId);
}

void LocalDisplayScheme_ShowTextScroll(void)
{
    if (LocalDisplayScheme_IsLocalDisplayAllowed() == 0U)
    {
        return;
    }

    LocalDisplayScheme_BeginManualSelection();
    LocalDisplayScheme_SetCurrentImage(LOCALDISPLAY_IMAGE_TEXT_ONLY);
    LocalDisplayScheme_ApplyCurrentView();
}

void LocalDisplayScheme_ShowClock(void)
{
    if (LocalDisplayScheme_IsLocalDisplayAllowed() == 0U)
    {
        return;
    }

    LocalDisplayScheme_BeginManualSelection();
    LocalDisplayScheme_SetCurrentImage(LOCALDISPLAY_IMAGE_RTC_CLOCK);
    LocalDisplayScheme_ApplyCurrentView();
}

void LocalDisplayScheme_ToggleTextClock(void)
{
    if (LocalDisplayScheme_IsLocalDisplayAllowed() == 0U)
    {
        return;
    }

    LocalDisplayScheme_BeginManualSelection();
    if (LocalDisplayScheme_IsClockImage((LocalDisplayImageId_t)g_localDisplayState.imageId) != 0U)
    {
        LocalDisplayScheme_SetCurrentImage(LOCALDISPLAY_IMAGE_TEXT_ONLY);
    }
    else if (LocalDisplayScheme_IsTextImage((LocalDisplayImageId_t)g_localDisplayState.imageId) != 0U)
    {
        LocalDisplayScheme_SetCurrentImage(LOCALDISPLAY_IMAGE_RTC_CLOCK);
    }
    else
    {
        LocalDisplayScheme_SetCurrentImage(LOCALDISPLAY_IMAGE_TEXT_ONLY);
    }

    LocalDisplayScheme_ApplyCurrentView();
}

void LocalDisplayScheme_NextEffect(void)
{
    uint8_t effectCount;
    uint8_t patternCount;

    if (LocalDisplayScheme_IsLocalDisplayAllowed() == 0U)
    {
        return;
    }

    LocalDisplayScheme_BeginManualSelection();
    if (LocalDisplayScheme_IsClockImage((LocalDisplayImageId_t)g_localDisplayState.imageId) != 0U)
    {
        (void)LocalDisplayScheme_GetManualEffectTable((LocalDisplayImageId_t)g_localDisplayState.imageId,
                                                      &effectCount);
        g_localDisplayState.effectIndex = (uint8_t)((g_localDisplayState.effectIndex + 1U) % effectCount);
        LocalDisplayScheme_ApplyCurrentView();
        return;
    }

    patternCount = LocalDisplayScheme_GetPatternCycleCount();
    if (patternCount == 0U)
    {
        return;
    }
    if (g_localDisplayState.patternIndex >= patternCount)
    {
        g_localDisplayState.patternIndex = 0U;
    }

    LocalDisplayScheme_SetCurrentImage((LocalDisplayImageId_t)g_localDisplayState.patternIndex);
    g_localDisplayState.effectIndex = (uint8_t)((g_localDisplayState.effectIndex + 1U)
                                                % LOCALDISPLAY_MANUAL_EFFECT_COUNT);
    LocalDisplayScheme_ApplyCurrentView();
}

void LocalDisplayScheme_NextColor(void)
{
    if (LocalDisplayScheme_IsLocalDisplayAllowed() == 0U)
    {
        return;
    }

    LocalDisplayScheme_BeginManualSelection();
    g_localDisplayState.colorIndex = (uint8_t)((g_localDisplayState.colorIndex + 1U)
                                               % LOCALDISPLAY_COLOR_THEME_COUNT);
    LocalDisplayScheme_ApplyCurrentView();
}