# AI Interface Orchestration Technical Reference

Concise technical reference for `AI端接口调度`. Derived from thesis architecture doc. See `Doc/Instructions/README.md` for navigation.

## File Map

```
Project/xiaozhi-esp32/
  main/
    gp_port/
      gp_led_matrix_esp32.h/.cc       -- Matrix controller: action → protocol packet, ACK handling
      gp_matrix_local_tools.h/.cc     -- Frame presets, logging helpers
      transport/
        gp_led_matrix_transport.h/.cc -- UART transport: background RX task, packet queue, write mutex
      ui/
        gp_debug_display.h/.cc        -- LVGL debug overlay: color control, matrix preview, link status
    boards/lichuang-dev/
      lichuang_dev_board.cc           -- Board integration: WS dispatch, preview HTTP, debug WS, MCP
    application.cc                    -- XiaoZhi app: state machine, MCP registration, WS routing
```

## Key Classes & Functions

| Class/Function | File | Role |
|---|---|---|
| `GpLedMatrixEsp32` | gp_led_matrix_esp32.cc | Core controller. Inherits `Led`. Manages protocol encoding, link state, ACK polling. |
| `ShowAction(GpMatrixActionPayload&)` | gp_led_matrix_esp32.cc | Send full 28B action. Dedup: skip if identical to `last_action_`. |
| `ShowRgb332Frame(uint8_t*)` | gp_led_matrix_esp32.cc | Send 256B RGB332 full frame via chunked transfer. |
| `ShowBitmapFrame(rows, color, format)` | gp_led_matrix_esp32.cc | Smart send: ≤4 layers → `LayeredFrame` single packet; >4 → `FrameStart/Chunk/Commit`. |
| `ShowLayeredFrame(vector<Layer>)` | gp_led_matrix_esp32.cc | Serialize layers `[hdr:1][bitmap:32][RGB:3]` per layer, send as `LayeredFrame` cmd (0x18). |
| `ShowLayeredAnimation(frames)` | gp_led_matrix_esp32.cc | Upload animation frames as `LayeredAnimFrame` (0x19), commit with `AnimationEnd`. |
| `ShowDebugState(GpColorDebugState&)` | gp_led_matrix_esp32.cc | Map debug UI state → `SetAction` for preset/color/animation selection. |
| `SendLocalControlAction(action)` | gp_led_matrix_esp32.cc | Send local-only actions: `next_pattern`, `show_clock`, etc. |
| `SyncClockTime()` | gp_led_matrix_esp32.cc | Forward Wi-Fi NTP time via `SetTime` cmd (6B: HH,MM,SS,YY,MM,DD). |
| `ReadReply()` | gp_led_matrix_esp32.cc | Poll reply queue up to 12 retries × 8ms. Match by `IS_REPLY` + `sequence` + `command`. |
| `PollIncomingRequest()` | gp_led_matrix_esp32.cc | Handle LED-initiated requests (`RequestCachedBitmap` → resend cached bitmap). |
| `RunStartupLinkTest()` | gp_led_matrix_esp32.cc | Send R→G→B solid color test sequence to verify Bluetooth link. |
| `TrySendBackgroundDebugLedCommand()` | gp_led_matrix_esp32.cc | Opportunistic debug LED update (no foreground activity required). |
| `GpMatrixBtUartTransport` | gp_led_matrix_transport.cc | UART transport: dedicated FreeRTOS RX task, byte→packet state machine, FreeRTOS RX queue. |
| `WritePacket(buf, len)` | gp_led_matrix_transport.cc | TX over UART with mutex protection. |
| `ReadPacket(buf, timeout)` | gp_led_matrix_transport.cc | Dequeue validated packet from RX queue. |
| `GpDebugLcdDisplay` | gp_debug_display.cc | LVGL debug overlay: color sliders, preset selector, 16x16 preview (256 objects), AI link indicator. |
| `LichuangDevBoard::HandleCustomPayload()` | lichuang_dev_board.cc | Dispatch WS `type:"custom"` → `matrix_pattern_result` / `matrix_action_result` / `matrix_animation_*`. |

## Dual Control Path Design

```
Path A: Local Touch + Voice Keywords
  Touch UI / keyword detect
    → GpColorDebugState / local action
    → SetAction (0x05) directly via Bluetooth
    → LED-side applies display profile
  Latency: <20ms (no round-trip to host)

Path B: Free-form Voice + MCP Tools
  Voice ASR → matrix_pattern_request
    → MCP tool call (self.screen.matrix_16x16.*)
    → Host Python drawing → matrix_pattern_result JSON
    → Debug WebSocket → lichuang_dev_board → ShowBitmapFrame()
    → Bluetooth upload to LED
  Latency: 0.5-3s (depends on LLM + network)
```

## Link State Management

```cpp
bool link_verified_;            // TRUE after successful Ping reply or frame ACK
bool remote_override_active_;   // TRUE when remote content owns display
GpMatrixActionPayload last_action_; // For dedup: skip redundant SetAction
int success_count_, fail_count_;    // Link quality counters
vector<uint8_t> cached_bitmap_rows_[16]; // Cached last frame for RequestCachedBitmap replay
```

## Transport Details

- **UART pins**: TX=GPIO10, RX=GPIO11
- **Baud rate**: 460800 (data), 38400 (HC-05 AT config)
- **RX architecture**: FreeRTOS task (`gp_bt_uart_rx_task`) continuously reads UART bytes into protocol state machine; validated packets pushed to `FreeRTOS queue` (depth: 8)
- **TX architecture**: Mutex-protected `uart_write_bytes()` with pre-assembled packet buffer
- **HC-05 pairing**: Master module on ESP32, slave on AI8051U; fixed address binding
- **Pairing code**: 19220309, local name "XiaoZhi", remote name "WS2812"

## Debug Services (On-Demand Only)

| Service | Port/Path | Trigger |
|---|---|---|
| Debug WebSocket | ws://:8766/debug | Host drawing result delivery |
| Preview HTTP | POST /debug/preview_image | Image preview upload |
| Status HTTP | GET /debug/preview_status | Link status query |
| Debug cmd worker | internal task | Background debug command processing |

All debug services started on-demand (not at boot) to conserve ESP32-S3 internal SRAM during listening/idle states.
