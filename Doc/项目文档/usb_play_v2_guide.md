# USB Play v2 Guide

## 1. Overview
This project now uses one unified USB command:

PLAY [params...]

Legacy commands are removed from runtime parsing:
- T=...
- M=...
- IMG=...
- FX=...
- GRAD=...
- FG=...
- BG=...

## 2. Parameter Definitions
Use one or more parameters in a single PLAY line.

- CT=x: content type
  - 0 = pattern image
  - 1 = glyph text
- DIR=x: display direction
  - 0 = normal
  - 1 = rotate 180
  - 2 = rotate clockwise 90
  - 3 = rotate counter-clockwise 90
- FX=x: render effect
  - 0 = static
  - 1 = breath (fade in/out)
  - 2 = gradient
  - 3 = scroll left
  - 4 = scroll right
  - 5 = JLU text scroll
  - 6 = fade in
  - 7 = fade out
  - 8 = color cycle
- SPD=x: scroll step in pixel/frame (1..255)
  - 1 = minimum speed (move 1 pixel every frame)
  - larger value = faster scroll
- ANI=x: animation step (1..255), larger is faster
- CM=x: color mode
  - 0 = solid
  - 1 = gradient
- GS=x: gradient span alpha (0..255)
- BR=x: brightness (0..255)
  - 0 = off
  - 255 = max brightness
- IMG=x: image index (0..TEST_IMAGE_COUNT-1)
- GI=x: static glyph index (0..TEST_SCROLL_GLYPH_COUNT-1)
  - used when CT=1 and non-scroll display
- SQ=a,b,c...: scroll sequence array
  - each value is a glyph index
  - example: SQ=0,1,2,3
- FG=RRGGBB: foreground RGB888 hex color
- BG=RRGGBB: background RGB888 hex color
- DBG=x: debug mode
  - 0 = off
  - 1 = on
- NMS=x: normal scan interval in ms
- LMS=x: legacy scan interval in ms

## 3. Example Commands
- PLAY CT=0 FX=0 IMG=2 BR=180
- PLAY CT=1 FX=5 SPD=3 FG=00FF00 BG=000000 BR=160
- PLAY CT=0 FX=2 CM=1 GS=180 IMG=1 BR=220
- PLAY CT=1 FX=0 GI=2 BR=180
- PLAY CT=1 FX=5 SPD=1 SQ=0,1,2,3 FG=FFFFFF BG=000000
- PLAY DBG=1
- PLAY NMS=2 LMS=1

## 4. Key Interrupt Preset Switching (P3.2)
A low-active key on P3.2 (INT0) switches preset modes in sequence:

1. Diamond + fade in/out
2. Cross + color gradient
3. Python image static
4. JLU text scroll

Each key press moves to the next preset and wraps around.

## 5. Display Scan Mode Toggle (P3.3)
A low-active key on P3.3 (INT1) toggles row-scan sending mode:

- `normal_pair`: dual-row pair output, sequence 0/1, 2/3, ...
- `legacy_shift`: shift-window output, sequence 0/1, 1/2, 2/3, ...
  - the first row in window uses WS2812 bit0 off-code in payload region
  - the second row in window outputs image PWM

## 6. Notes
- Keep USB command uppercase to avoid parser mismatch.
- Parameters can be combined in any order on one PLAY line.
- If command is invalid, firmware replies with a PLAY usage hint.
- In legacy mode, firmware may auto-clamp too-small interval to a safe minimum to avoid scan jitter/flicker.
