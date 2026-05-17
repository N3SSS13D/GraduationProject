# Module Interface Specification

Part of the GraduationProject documentation. See `Doc/Instructions/README.md` for navigation.

## Purpose

This document defines the external interfaces exposed and consumed by each of the four system modules. It serves as the contract reference for cross-module integration and debugging.

---

## 1. LED端显示驱动 (LED Side Display Driver)

**Path**: `Project/STC51/ws2812_driver/`
**MCU**: AI8051U (STC 32-bit 8051), 33.1776 MHz

### Exposed Interfaces

| Interface | Type | Description |
|---|---|---|
| UART2 RX (P1.0 alt) | Hardware | Receives protocol packets from HC-05 Bluetooth slave module. 460800 bps, 8N1. DMA-assisted 192-byte ring buffer. |
| UART2 TX (P1.1 alt) | Hardware | Sends ACK/reply protocol packets back to HC-05. Same baud rate. |
| PWM Channel 1 (P1.0) | Hardware | WS2812 data line for even rows (0,2,4,...,14). PWMAT-DMA auto-transfer. |
| PWM Channel 2 (P1.2) | Hardware | WS2812 data line for odd rows (1,3,5,...,15). PWMAT-DMA auto-transfer. |
| 74HC595 SPI (P1.3-P1.5) | Hardware | Row selection: SER(data), SRCLK(clock), RCLK(latch). 3 cascaded shift registers for 16 rows. |
| GPIO P32 (INT0) | Hardware | Button 1: short=next pattern, long=toggle text/clock mode |
| GPIO P33 (INT1) | Hardware | Button 2: short=next effect, long=next color theme |
| USB (P3.0/P3.1) | Hardware | AiCube ISP programming + DEBUG command interface |
| Timer1 ISR | Software | ~1ms scan step trigger. Not user-callable. |
| Timer0 ISR | Software | 1ms scheduler tick. Not user-callable. |

### Consumed Interfaces

| Interface | Source | Description |
|---|---|---|
| `GpMatrixPacketHeader` | `Project/Protocols/gp_led_matrix_protocol.h` | Protocol packet structure: magic, flags, sequence, command, payload_len, crc |
| `GpMatrixCommand` enum | `Project/Protocols/gp_led_matrix_protocol.h` | All valid command IDs (0x01-0x30) |
| `GpMatrixActionPayload` | `Project/Protocols/gp_led_matrix_protocol.h` | 28-byte action descriptor for SetAction command |
| `GpLedDisplayProfile` | `Sources/inc/gp_led_display_profile.h` | Internal display profile: content type, effect, color, timing, direction |
| CRC8/CRC16 tables | Built-in (generated from protocol header constants) | Polynomial: CRC-8/MAXIM, CRC-16/XMODEM |

### Pin Assignment Summary
| Pin | Function |
|---|---|
| P1.0 | WS2812 Data CH1 (even rows) |
| P1.2 | WS2812 Data CH2 (odd rows) |
| P1.3 | 74HC595 SER (data) |
| P1.4 | 74HC595 SRCLK (shift clock) |
| P1.5 | 74HC595 RCLK (latch) |
| P3.0/P3.1 | UART2 RX/TX (HC-05 data) |
| P3.2 (INT0) | Button 1 |
| P3.3 (INT1) | Button 2 |
| P2.0-P2.7 | Debug 7-segment LED outputs |

---

## 2. AI端接口调度 (AI-side Interface Orchestration)

**Path**: `Project/xiaozhi-esp32/main/gp_port/`
**Platform**: ESP32-S3, ESP-IDF v5.4.3

### Exposed Interfaces

| Interface | Type | Description |
|---|---|---|
| `GpLedMatrixEsp32` class | C++ API | Core matrix controller. Methods: `ShowAction()`, `ShowRgb332Frame()`, `ShowBitmapFrame()`, `ShowLayeredFrame()`, `ShowLayeredAnimation()`, `SyncClockTime()`, `PollIncomingRequest()`, `ReadReply()`, `RunStartupLinkTest()` |
| `GpMatrixTransport` abstract | C++ API | Transport abstraction: `WritePacket()`, `ReadPacket()`, `DescribeLink()`. Implementations: `GpMatrixBtUartTransport` |
| Debug WebSocket (port 8766) | Network | JSON messages: `matrix_pattern_result`, `matrix_animation_start`, `matrix_animation_frame`, `matrix_animation_end`, `matrix_action_result` |
| HTTP Preview | Network | `POST /debug/preview_image` (PNG upload), `GET /debug/preview_status` (status query). On-demand only. |
| LVGL Debug UI | LCD Display | Touch-based color control, preset selection, animation selector, matrix preview (256 LVGL objects), AI link status |
| `GpColorDebugState` | Data struct | Intermediate state object carrying debug color/animation/preset intent |
| MCP Local Tools | MCP | `self.screen.matrix_16x16.local.*` namespace: local-only debug tools including `snapshot`, `preview` |

### Consumed Interfaces

| Interface | Source | Description |
|---|---|---|
| `GpMatrixPacketHeader` | `Project/Protocols/gp_led_matrix_protocol.h` | Same protocol structures as LED side |
| `GpMatrixCommand` enum | `Project/Protocols/gp_led_matrix_protocol.h` | Command ID definitions |
| `GpMatrixActionPayload` | `Project/Protocols/gp_led_matrix_protocol.h` | Action descriptor for local control actions |
| HC-05 UART | ESP32 UART hardware | GPIO10 (TX), GPIO11 (RX). AT command: 38400 bps. Data: 460800 bps. |
| XiaoZhi Application | `main/application.cc` | `DeviceState` enum, `Led` base class, MCP tool registration |
| `Board` class | `main/boards/lichuang-dev/lichuang_dev_board.cc` | `HandleCustomPayload()` for WS message routing |
| Wi-Fi NTP | ESP-IDF net | `time()` for clock sync via `SyncClockTime()` |

### HC-05 Configuration
| Parameter | Value |
|---|---|
| Master module | ESP32 side |
| Slave module | AI8051U side |
| AT baud rate | 38400 |
| Data baud rate | 460800 |
| Pairing code | 19220309 |
| Local name | XiaoZhi |
| Remote name | WS2812 |
| Role (master) | Master, fixed address binding |
| Role (slave) | Slave |

---

## 3. 蓝牙通信协议 (Bluetooth Communication Protocol)

**Path**: `Project/Protocols/`

### Exposed Interfaces

| Interface | Type | Description |
|---|---|---|
| `gp_led_matrix_protocol.h` | C/C++ Header | Shared truth source. Defines: magic byte, flag bits, command IDs, payload structs (`GpMatrixPacketHeader`, `GpMatrixActionPayload`, etc.), CRC constants, field size limits |
| `gp_led_matrix_protocol_spec.md` | Markdown Doc | Packet-level behavior: V2/V3 format, CRC scheme, frame chunking, animation batch, ACK semantics |
| `gp_matrix_pattern_protocol.md` | Markdown Doc | Host↔AI drawing contract: JSON format for `matrix_pattern_result`, bitmap formats, animation parameters, MCP tool boundaries |

### Core Protocol Constants (from `gp_led_matrix_protocol.h`)
| Constant | Value | Meaning |
|---|---|---|
| `GP_MATRIX_MAGIC` | `0x47` | Packet start delimiter |
| `GP_MATRIX_HEADER_SIZE` | 6 | V3 compact header size in bytes |
| `GP_MATRIX_PAYLOAD_MAX` | 255 | Maximum payload bytes per packet |
| `GP_MATRIX_CHUNK_SIZE` | 64 | Max chunk payload for FrameChunk |
| `GP_MATRIX_MAX_ANIM_FRAMES` | 32 | Maximum frames per animation |
| `GP_MATRIX_MAX_LAYERS` | 16 | Max layers for static frames |
| `GP_MATRIX_MAX_ANIM_LAYERS` | 4 | Max layers per animation frame |

### Image Format Comparison
| Format | Size | Use Case |
|---|---|---|
| RGB332 full frame | 256 bytes (16×16×1) | Full-color arbitrary frame |
| Compact bitmap + RGB888 | 38 bytes (32B bitmap + 3B color + 3B header) | Single-color pattern |
| Layered bitmap (1 layer) | 36 bytes (1B header + 32B bitmap + 3B color) | Single-color overlay |
| Layered bitmap (4 layers) | 144 bytes | 4-color composite image |
| Layered bitmap (16 layers) | 576 bytes | 16-color full image |

### Consumed By
- `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c` (LED-side parser)
- `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc` (AI-side encoder)
- `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc` (transport layer)
- `Project/Script/mcp/gp_matrix/gp_mcp_endpoint_client.py` (script-side contract compliance)

---

## 4. 本地绘图脚本 (Local Drawing Scripts)

**Path**: `Project/Script/`
**Runtime**: Python 3.x

### Exposed Interfaces

| Interface | Type | Description |
|---|---|---|
| `self.screen.matrix_16x16.draw_python` | MCP Tool | Execute restricted Python drawing code (AST whitelist). Input: Python source string. Output: bitmap_rows_hex + RGB color. |
| `self.screen.matrix_16x16.render_prompt` | MCP Tool | Natural language → pattern template. Input: text description. Output: bitmap_rows_hex. |
| `self.screen.matrix_16x16.show_text` | MCP Tool | Text string → glyph → 16×16 bitmap frame sequence. Input: text, font size, color, speed. Output: animation frames. |
| `self.screen.matrix_16x16.show_scroll_subtitle` | MCP Tool | Long text → offscreen wide bitmap → chunked scroll frames. Input: text, scroll direction, speed. Output: scroll frame sequence. |
| `self.screen.matrix_16x16.show_effect` | MCP Tool | Direct native effect command (no frame-by-frame). Input: effect name + parameters. Output: `matrix_action_result` JSON. |
| `self.screen.matrix_16x16.draw_animation` | MCP Tool | Multi-frame animation. Input: frame list + timing. Output: animation batch JSON. |
| Debug WebSocket (port 8766) | Network | Sends `matrix_pattern_result` / `matrix_animation_*` JSON to ESP32. |
| HTTP Fallback (port 8765) | Network | `POST /control/matrix_prompt_16x16` when WebSocket unavailable. |
| `ws2812_auto_debug.py` | CLI Tool | Automated chain: Keil rebuild → STC serial monitor → ESP-IDF build/flash/monitor. |

### Consumed Interfaces

| Interface | Source | Description |
|---|---|---|
| MCP WebSocket | `wss://api.xiaozhi.me/mcp/` | Remote MCP server for tool registration and invocation |
| ESP32 Debug WebSocket | `ws://<esp32-ip>:8766/debug` | AI-side frame preview and animation delivery |
| ESP32 HTTP API | `http://<esp32-ip>:8765/control/*` | Matrix prompt submission (HTTP fallback) |
| `gp_matrix_pattern_protocol.md` | `Project/Protocols/` | JSON format contract for all drawing results |
| `gp_led_matrix_protocol.h` | `Project/Protocols/` | Field size limits, layer max, animation max |
| Keil toolchain | `S:\Embedded\Keil` | For auto-debug rebuild step |
| ESP-IDF toolchain | `S:\Embedded\ESP\v5.4.3\esp-idf` | For auto-debug build/flash step |

### MCP Tool Namespace Convention
- **Host LLM tools**: `self.screen.matrix_16x16.<tool_name>` — for AI model use via MCP
- **Local debug tools**: `self.screen.matrix_16x16.local.<tool_name>` — for direct ESP32-side debugging
- Documents and prompts must not mix these two namespaces.

---

## Cross-Module Contract Summary

```
┌─────────────┐     JSON (WebSocket/HTTP)     ┌─────────────┐
│  Script     │ ─────────────────────────────> │  AI-side    │
│  Layer      │   matrix_pattern_result       │  (ESP32)    │
└─────────────┘                                └──────┬──────┘
                                                      │
                              Binary Protocol (UART)   │
                              gp_led_matrix_protocol.h │
                                                      │
                                               ┌──────┴──────┐
                                               │  LED-side   │
                                               │  (AI8051U)  │
                                               └─────────────┘
```

**Rules**:
1. Script layer must NOT assume direct LED-side serial port details. Always go through AI-side WebSocket/HTTP.
2. Protocol header (`gp_led_matrix_protocol.h`) is the single source of truth for field sizes, command IDs, and constants. Both AI-side and LED-side MUST derive from it.
3. MCP tool namespace: host uses `self.screen.matrix_16x16.*`, local debug uses `.local.*`.
4. All drawing results use documented JSON formats from `gp_matrix_pattern_protocol.md`.
5. AI-side debug services (preview HTTP, debug WS) are on-demand only — not always-on.
