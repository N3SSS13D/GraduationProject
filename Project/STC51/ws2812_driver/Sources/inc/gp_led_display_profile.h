/*
 * @file gp_led_display_profile.h
 * @author GitHub Copilot
 * @date 2026-05-04
 * @version 1.0
 * @brief Unified display-profile descriptor for LED-side pattern/color/animation control.
 */

#ifndef __GP_LED_DISPLAY_PROFILE_H__
#define __GP_LED_DISPLAY_PROFILE_H__

#include "gp_led_matrix_protocol.h"

#define GP_LED_PROFILE_VERSION_V1             0x01U
#define GP_LED_PROFILE_FLAG_APPLY_PATTERN    0x01U
#define GP_LED_PROFILE_FLAG_APPLY_GLYPH      0x02U

#define GP_LED_TIMELINE_PATH_LINEAR          0x00U
#define GP_LED_TIMELINE_PATH_EASE_IN_OUT     0x01U
#define GP_LED_TIMELINE_PATH_BREATH_CURVE    0x02U

typedef struct
{
    /* Structure version for forward-compatible extension. */
    uint8_t version;
    /* Reserved profile-level flags, keep as 0 for now. */
    uint8_t profileFlags;

    /* Source and content kind use GP protocol enums. */
    uint8_t source;
    uint8_t content;
    /* Effect and direction use GP protocol enums. */
    uint8_t effect;
    uint8_t direction;
    /* Color mode uses GP protocol enum. */
    uint8_t colorMode;
    /* 0..255 brightness scale. */
    uint8_t brightness;

    /* Foreground (primary) RGB888. */
    uint8_t primaryR;
    uint8_t primaryG;
    uint8_t primaryB;
    /* Background (secondary) RGB888. */
    uint8_t secondaryR;
    uint8_t secondaryG;
    uint8_t secondaryB;

    /* Content selectors for pattern/glyph routes. */
    uint8_t patternId;
    uint8_t glyphId;
    /* Animation pacing controls (0 means use default fallback). */
    uint8_t scrollStep;
    uint8_t animStep;
    /* Gradient blend amount (0 means use module default). */
    uint8_t gradientSpan;
    /* Reuse protocol action flags such as USE_SECONDARY / REMOTE_RELEASE. */
    uint8_t actionFlags;

    /* Timeline-like extension fields for staged animation control. */
    uint16_t frameIntervalMs;
    uint16_t timelineDurationMs;
    uint16_t timelineRepeatDelayMs;
    uint8_t timelineRepeatCount;
    uint8_t timelinePath;

    /* Animation batch flags from protocol, e.g. LOOP. */
    uint8_t animationFlags;
    /* Apply-selection flags, e.g. apply patternId/glyphId this round. */
    uint8_t applyFlags;
} GpLedDisplayProfile;

#endif
