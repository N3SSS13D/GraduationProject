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

## 6.1 Bluetooth debug command

The USB parser now accepts a small structured Bluetooth command set on `BT_Version`:

- `BT SEND text`
- `BT STATUS`
- `BT text` (legacy direct payload form, still supported)

Common examples:

- `BT SEND AT`
- `BT AT+VERSION?`
- `BT AT+UART?`
- `BT AT+UART=460800,0,0`
- `BT SEND LED 0`

Behavior:

- `BT SEND text` forwards `text` to the HC-05 on `UART2(P4.2/P4.3)` and always appends `\r\n` if missing.
- `BT STATUS` prints the compact UART2 state summary without sending anything to the HC-05.
- `BT text` remains available as a compatibility shortcut and behaves like `BT SEND text`.
- Startup now runs an automatic HC-05 setup sequence at `38400 8N1` in strict set-then-query order: `AT`, version query, slave role plus role query, `WS2812` plus name query, `19220309` plus password query, `AT+UART=460800,0,0` plus `AT+UART?`, and finally `AT+RESET`; the local baudrate switch happens only after the reset reply is `OK`.
- The AT path no longer depends on `P4.1` or `PIO11`; the firmware keeps `P4.1` low and uses pure UART AT transactions.
- If `text` is `AT+UART=<baud>,0,0`, firmware first waits for the HC-05 reply on the old baudrate, then sends `AT+RESET`, and switches the local `UART2` baudrate only after both replies contain `OK`; startup now begins at `38400 8N1` and ends at `460800 8N1`.
- The received HC-05 reply is printed through USB serial as ASCII and HEX summaries.
- Firmware prints compact tagged logs such as `[BT_CMD]`, `[BT_RSP]`, `[BT_STA]`, `[BT_MON]`, and `[BT_ACT]` so command flow, reply flow, periodic monitor output, and board actions are easier to distinguish.
- RX-side debug text is captured in the UART ISR and then printed/handled by a 50ms scheduled debug task after the byte stream becomes idle, so incoming text such as `LED 0` can still light the board LED without doing heavy work inside the ISR.
