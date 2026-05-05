#ifndef __TEST_IMAGE_H__
#define __TEST_IMAGE_H__

#define TEST_IMAGE_ROWS                  16
#define TEST_IMAGE_COLS                  16

#ifndef TEST_IMAGE_BG_RGB332
#define TEST_IMAGE_BG_RGB332             0x00
#endif

#ifndef TEST_SCROLL_GLYPH_COUNT
#define TEST_SCROLL_GLYPH_COUNT          4U
#endif

#ifndef TEST_SCROLL_GLYPH_WIDTH
#define TEST_SCROLL_GLYPH_WIDTH          16U
#endif

#ifndef TEST_SCROLL_GLYPH_SPACING
#define TEST_SCROLL_GLYPH_SPACING        1U
#endif

static const uint16_t code g_testScrollGlyphRows[TEST_SCROLL_GLYPH_COUNT][TEST_IMAGE_ROWS] =
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
    }
};

#endif
