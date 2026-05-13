/*
 * @file local_display_assets.h
 * @author GitHub Copilot
 * @date 2026-05-12
 * @version 1.0
 * @brief Local display canvas and scroll-glyph asset definitions.
 */

#ifndef __LOCAL_DISPLAY_ASSETS_H__
#define __LOCAL_DISPLAY_ASSETS_H__

#define LOCALDISPLAY_ASSET_ROWS                         16U
#define LOCALDISPLAY_ASSET_COLS                         16U

#ifndef LOCALDISPLAY_ASSET_BG_RGB332
#define LOCALDISPLAY_ASSET_BG_RGB332                    0x00
#endif

#ifndef LOCALDISPLAY_SCROLL_GLYPH_COUNT
#define LOCALDISPLAY_SCROLL_GLYPH_COUNT                 5U
#endif

#ifndef LOCALDISPLAY_SCROLL_GLYPH_DEFAULT_SEQUENCE_COUNT
#define LOCALDISPLAY_SCROLL_GLYPH_DEFAULT_SEQUENCE_COUNT 4U
#endif

#ifndef LOCALDISPLAY_SCROLL_GLYPH_WIDTH
#define LOCALDISPLAY_SCROLL_GLYPH_WIDTH                 16U
#endif

#ifndef LOCALDISPLAY_SCROLL_GLYPH_SPACING
#define LOCALDISPLAY_SCROLL_GLYPH_SPACING               1U
#endif

static const uint16_t code g_localDisplayScrollGlyphRows[LOCALDISPLAY_SCROLL_GLYPH_COUNT]
                                                        [LOCALDISPLAY_ASSET_ROWS] =
{
    /* 0: 吉 */
    {
        0x0000,
        0x0080,
        0x0080,
        0x3FFE,
        0x35D6,
        0x0080,
        0x1FFC,
        0x1F7C,
        0x0000,
        0x0FF8,
        0x0808,
        0x0808,
        0x0FF8,
        0x0FF8,
        0x0800,
        0x0000
    },
    /* 1: 林 */
    {
        0x0000,
        0x0860,
        0x0860,
        0x1860,
        0x3FFC,
        0x1860,
        0x1C70,
        0x1EF0,
        0x39F8,
        0x292C,
        0x0B64,
        0x0860,
        0x0860,
        0x0000,
        0x0000,
        0x0000
    },
    /* 2: 大 */
    {
        0x0000,
        0x0080,
        0x0080,
        0x0080,
        0x0180,
        0x3FFE,
        0x1FFC,
        0x0180,
        0x01C0,
        0x0360,
        0x0230,
        0x0E38,
        0x1C1C,
        0x3006,
        0x0000,
        0x0000
    },
    /* 3: 学 */
    {
        0x0000,
        0x0000,
        0x1110,
        0x19B0,
        0x0D30,
        0x3FFC,
        0x300C,
        0x0FE0,
        0x04F0,
        0x00C0,
        0x3FFC,
        0x3FF8,
        0x0080,
        0x0380,
        0x0300,
        0x0000
    },
    /* 4: JLU emblem */
    {
        0x7FFC,
        0x4104,
        0x4104,
        0x4104,
        0x410F,
        0x416C,
        0x80D9,
        0x41B2,
        0x337C,
        0x4CF0,
        0x4004,
        0x4104,
        0x2108,
        0x3018,
        0x0C60,
        0x0380
    }
};

#endif