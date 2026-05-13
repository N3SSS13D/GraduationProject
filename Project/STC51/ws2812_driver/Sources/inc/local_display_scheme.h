/*
 * @file local_display_scheme.h
 * @author GitHub Copilot
 * @date 2026-05-11
 * @version 1.0
 * @brief Local startup display scheme and offline key-action routing.
 */

#ifndef __LOCAL_DISPLAY_SCHEME_H__
#define __LOCAL_DISPLAY_SCHEME_H__

void LocalDisplayScheme_Init(void);
void LocalDisplayScheme_Task10ms(void);
void LocalDisplayScheme_NextPattern(void);
void LocalDisplayScheme_ShowTextScroll(void);
void LocalDisplayScheme_ShowClock(void);
void LocalDisplayScheme_ToggleTextClock(void);
void LocalDisplayScheme_NextEffect(void);
void LocalDisplayScheme_NextColor(void);

#endif