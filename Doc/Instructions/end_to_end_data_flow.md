# End-to-End Data Flow Reference

Part of the GraduationProject documentation. See `Doc/Instructions/README.md` for navigation.

## Overview

This document traces a complete user request through all four modules to physical LED output. Understanding this chain is essential for debugging any cross-module issue.

```
User Request (voice/text)
  -> Script Layer (MCP tools, drawing, encoding)
    -> AI-side Orchestration (WebSocket receive, protocol packing, UART send)
      -> Bluetooth Transport (HC-05 SPP, byte stream on wire)
        -> LED-side Execution (UART2 DMA, protocol parse, WS2812 scan)
          -> Physical LED Matrix Output
```

---

## Stage 1: Host Drawing Request → JSON Result (Script Layer)

### Trigger
User speaks "画一个红心" (draw a red heart). XiaoZhi voice pipeline recognizes the intent and emits `matrix_pattern_request` with the drawing intent as a prompt.

### Flow
1. XiaoZhi MCP endpoint receives `tools/call` for `self.screen.matrix_16x16.draw_python` (or `render_prompt`)
2. `gp_mcp_endpoint_client.py` dispatches the call to the appropriate handler:
   - `render_prompt`: natural language → pattern template
   - `draw_python`: AST-whitelisted Pillow operations → 16x16 frame
   - `draw_text`: character glyph → 16x16 bitmap frame sequence
3. Handler generates output in one of three formats:
   - **RGB332 full frame**: 256 bytes (16×16, 1 byte/pixel)
   - **Compact bitmap + RGB888**: 38 bytes (32 bytes bitmap + 3 bytes color + 3 bytes header)
   - **Layered bitmap**: 36 bytes/layer (1B header + 32B bitmap + 3B RGB888), up to 4 layers
4. Result wrapped as `matrix_pattern_result` JSON object:
```json
{
  "type": "matrix_pattern_result",
  "bitmap_rows_hex": ["0x0000", "0x07e0", ...],
  "rgb_color": [255, 0, 0],
  "frame_format": "layered_bitmap"
}
```
5. Sent via Debug WebSocket (port 8766) to ESP32, or via HTTP POST to `/control/matrix_prompt_16x16`

### Key Files
- `Project/Script/mcp/gp_matrix/gp_mcp_endpoint_client.py`
- `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
- `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`

---

## Stage 2: WebSocket Receive → Protocol Encode → UART Send (AI-side)

### Flow
1. `LichuangDevBoard::HandleCustomPayload()` receives the `matrix_pattern_result` JSON via Debug WebSocket
2. Extracts `bitmap_rows_hex`, `rgb_color`, and `frame_format`
3. Calls `GpLedMatrixEsp32::ShowBitmapFrame()` with the parsed bitmap data
4. `ShowBitmapFrame()` determines the optimal protocol command:
   - 1-4 layers → `LayeredFrame` single packet (command `0x18`, 144 bytes max)
   - >4 layers → `FrameStart/Chunk/Commit` chunked transfer (commands `0x10`/`0x11`/`0x12`)
5. `SendStagedFrame()` builds the protocol packet:
   - 6-byte header: magic(0x47) + flags + sequence + command + payload_len + header_crc8
   - Payload: serialized layer data `[total_layer:4|seq:4][bitmap:32][RGB:3]` per layer
   - 2-byte trailer: packet_crc16 (CRC of header + payload)
6. `GpMatrixBtUartTransport::WritePacket()` sends bytes over UART to HC-05 at 460800 bps
7. `ReadReply()` polls for ACK reply (retry 12 times, 8ms interval)

### Concrete Example (single-layer red heart)
```
Header:  47 01 05 18 24 XX  (magic, flags, seq, cmd=LayeredFrame, len=36, header_crc8)
Payload: 11 00 00 07 E0 ... (total_layers=1|seq=0, then 32B bitmap, 3B RGB=FF0000)
  Total payload: 1 + 32 + 3 = 36 bytes
CRC16:   YY YY              (CRC-16/XMODEM over header+payload)
```

### Key Files
- `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
- `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc`
- `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`

---

## Stage 3: Bluetooth Byte Stream → UART2 DMA → Protocol Parse (LED-side)

### Flow
1. HC-05 slave receives bytes over Bluetooth SPP, forwards to AI8051U UART2 RX pin
2. AI8051U UART2 ISR feeds bytes into a 192-byte ring buffer (`GpMatrixRxRingBuf`)
3. `GpLedMatrixAi8051u_Poll()` (called from main loop) drains the ring buffer byte by byte
4. State machine `RxState` tracks:
   - `RX_IDLE` → wait for magic byte `0x47`
   - `RX_HEADER` → collect 5 remaining header bytes, verify `header_crc8`
   - `RX_PAYLOAD` → collect `payload_length` bytes
   - `RX_CRC` → collect 2 trailing bytes, verify `packet_crc16`
5. On CRC failure: drop packet, increment error counter, restart search for next `0x47`
6. On success: `ProcessPacket()` dispatches to command handler based on `command` byte

### Error Recovery
- Magic byte `0x47` appearing inside payload: harmless — byte is not `0x47`-after-`0x50` (V2 pattern) or `0x47`-not-`0x50` (V3 pattern)
- Byte corruption: caught by dual CRC (header-level for length safety, packet-level for data safety)
- Partial packet loss: re-sync on next `0x47`; AI-side retry mechanism handles timeout

### Key Files
- `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- `Project/Protocols/gp_led_matrix_protocol.h`

---

## Stage 4: Command Dispatch → WS2812 Encode → PWM+DMA Output (LED-side)

### Flow
1. Command handler in `gp_led_matrix_ai8051u.c` routes to `GpLedAction`:
   - `SetAction (0x05)` → `GpLedAction_ApplyAction()` → set display profile
   - `LayeredFrame (0x18)` → `GpLedAction_ApplyFrameBitmapLayered()` → write to frame buffer
   - `CommitFrame (0x12)` → `GpLedAction_CommitFrame()` → render and display
2. `GpLedAction_ApplyFrameBitmapLayered()`:
   - Parses each layer: `[header:1][bitmap:32][RGB:3]`
   - Layer 0 forms the base (pixels not set remain black)
   - Subsequent layers overlay on top of previous layers (pixel set=1 → use layer color)
   - Result assembled into the WS2812 driver's image buffer
3. `WS2812DRV_BeginFrameWrite()` / `SetPixelRgbFast()` / `EndFrameWrite()` writes pixel data
4. `WS2812DRV_EncodeAllRows()` converts RGB pixels to PWM duty cycles:
   - Each pixel = GRB order, 24 bits total (8 bits per channel)
   - 256-entry × 8-bit lookup table maps bit patterns to PWM compare values
   - Output: per-row PWM sequence array
5. Timer1 ISR triggers `WS2812DRV_RefreshStep()` every ~1ms:
   - Step index 0-7, each step drives one row pair (0+1, 2+3, ..., 14+15)
   - Build dual-row PWM buffer (CH1=even row, CH2=odd row)
   - Configure PWMAT-DMA channel 1 and 2 with buffer addresses
   - Trigger 74HC595 row selection via `HC595DRV_SetRow()`
   - DMA auto-transfers PWM sequence to P1.0/P1.2 output pins
6. After 8 steps (~8ms), one full frame is displayed; repeat for 30fps

### Key Timing Parameters
| Parameter | Value |
|---|---|
| PWM clock | 33.1776 MHz / 48 = 691.2 kHz (~1.447us/bit) |
| T0H (0-code high) | 0.35us (compare value: ~0x18) |
| T0L (0-code low) | 0.80us (compare value: ~0x38) |
| T1H (1-code high) | 0.70us (compare value: ~0x30) |
| T1L (1-code low) | 0.60us (compare value: ~0x28) |
| Reset gap | 50us minimum (compare values: 0x0000 for reset) |
| Row-pair step interval | ~1000us (Timer1 reload) |
| Full frame time | ~8ms (8 row-pairs) |
| Frame rate | ~30fps (32ms render task interval) |

### Key Files
- `Project/STC51/ws2812_driver/Sources/mid/gp_led_action.c`
- `Project/STC51/ws2812_driver/Sources/mid/draw_drv.c`
- `Project/STC51/ws2812_driver/Sources/drv/ws2812_drv.c`
- `Project/STC51/ws2812_driver/Sources/drv/hc595_drv.c`

---

## Byte-Level Trace: "Red Heart" — Single Layered Frame

### Step 1: Script builds bitmap for a 16×16 heart shape
```
Bitmap rows (hex, MSB-first per row):
  0000 07E0 0FF0 1FF8 1FF8 3FFC 3FFC 7FFE
  7FFE 7FFE 3FFC 3FFC 1FF8 1FF8 0FF0 07E0
RGB color: FF 00 00 (red)
```

### Step 2: Layer header + bitmap + color (36 bytes)
```
11          <- total_layers=1, seq=0
0000        <- row 0 bitmap (uint16 LE: 0x0000)
E007        <- row 1 bitmap (uint16 LE: 0x07E0)
F00F        <- row 2 bitmap (uint16 LE: 0x0FF0)
... remaining 13 rows ...
FF0000      <- RGB888 red
```

### Step 3: Protocol packet (over UART, sent by ESP32 HC-05)
```
47          <- magic
01          <- flags (ack_req=1)
05          <- sequence number
18          <- command (LayeredFrame)
24          <- payload_length (36 = 0x24)
XX          <- header_crc8 (computed over bytes 0-4: 47 01 05 18 24)
11 0000...  <- 36-byte payload from Step 2
YY YY       <- packet_crc16 (CRC-16/XMODEM over bytes 0-41)
```
**Total: 44 bytes on wire.**

### Step 4: AI8051U UART2 receives 44 bytes, state machine assembles packet
### Step 5: CRC passes → dispatch to `HandleLayeredFrame()` → `GpLedAction_ApplyFrameBitmapLayered()`
### Step 6: Frame rendered → WS2812 EncodeAllRows → Timer1 RefreshStep → PWM+DMA → LEDs light up

---

## Common Failure Points

| Symptom | Likely Layer | Check |
|---|---|---|
| No LED response, no ACK | Transport | HC-05 paired? UART baud match? |
| LED shows garbage/random colors | Protocol | CRC errors? Check serial log for `[GP_CRC]` |
| Correct pattern but wrong color | Script/AI-side | Bitmap→RGB mapping correct? Endianness correct? |
| Flickering or tearing | LED driver | Double-buffer swap timing? DMA alignment? |
| Animation freezes mid-playback | LED action layer | Frame count mismatch? Corrupted stored frame? |
| MCP tool returns error | Script layer | Valid AST whitelist? Bitmap hex format correct? |
| Debug WS not receiving results | AI-side | Debug WS enabled? Port 8766 reachable? |
