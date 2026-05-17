# Bluetooth Communication Protocol

## Category

`蓝牙通信协议`

## Active files

- 共享协议头：`Project/Protocols/gp_led_matrix_protocol.h`
- 主协议说明：`Project/Protocols/gp_led_matrix_protocol_spec.md`
- 图案请求说明：`Project/Protocols/gp_matrix_pattern_protocol.md`

## Scope

本分类用于保存 `AI端` 与 `LED端` 共用的蓝牙通信协议定义、字段约束和请求契约。

## Prompt / Skill 入口

- Prompt：`.github/prompts/ws2812-bluetooth-protocol*.prompt.md`
- Skill：`.github/skills/bluetooth-protocol/SKILL.md`

两端业务实现请分别查看：

- `Project/xiaozhi-esp32/main/gp_port/`
- `Project/STC51/ws2812_driver/`

## Artifact quick map

- `gp_led_matrix_protocol.h`
  - 当前共享协议唯一源头：包头常量、命令字、负载结构、分片限制和动画上限都以此为准。
- `gp_led_matrix_protocol_spec.md`
  - 描述线上的包结构、命令流程、ACK/状态语义和推荐传输顺序。
- `gp_matrix_pattern_protocol.md`
  - 描述主机绘图请求、`bitmap_rows_hex` / `matrix_action_result` 表达方式，以及主机 -> `AI端` -> `LED端` 的契约边界。

## Current protocol flow

1. 主机绘图、原生效果命令或 `AI端` 本地动作生成协议负载
2. `AI端` 按 `gp_led_matrix_protocol.h` 拼包发送
3. `LED端` 解析相同字段并执行
4. 若协议语义变化，文档与两端实现必须同步更新

当前 `SetAction` 附加约定：当 `content=state` 且 `animation_flags` 携带 `GpMatrixLocalControlAction` 时，`LED端` 直接执行
离线本地方案动作（`next_pattern / show_text_scroll / show_clock / toggle_text_clock / next_effect / next_color`），不新增命令字，也不改变 `28` 字节动作负载长度。

## Consumer entry points

- `AI端` 发送侧：`Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
- `LED端` 接收侧：`Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- 主机绘图契约：`Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
- 主机 websocket 转发入口：`Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`

## Key Constants & Structures Reference

### Protocol Header (`gp_led_matrix_protocol.h`)

| Symbol | Value | Description |
|---|---|---|
| `GP_MATRIX_MAGIC` | `0x47` | Packet start delimiter |
| `GP_MATRIX_HEADER_SIZE_V3` | 6 | V3 compact header bytes |
| `GP_MATRIX_HEADER_SIZE_V2` | 12 | V2 legacy header bytes |
| `GP_MATRIX_PAYLOAD_MAX` | 255 | Max payload bytes per packet |
| `GP_MATRIX_CHUNK_SIZE` | 64 | Max chunk payload per `FrameChunk` |
| `GP_MATRIX_MAX_ANIM_FRAMES` | 32 | Max frames per animation |
| `GP_MATRIX_MAX_LAYERS` | 16 | Max layers for static frames |
| `GP_MATRIX_MAX_ANIM_LAYERS` | 4 | Max layers per animation frame |

### Key Structs

| Struct | Size | Fields |
|---|---|---|
| `GpMatrixPacketHeader` | 6B | magic, flags, sequence, command, payload_length, header_crc8 |
| `GpMatrixActionPayload` | 28B | content_type, effect_id, rgb_color[3], brightness, scroll_step, anim_step, frame_interval_ms, flags, etc. |
| `GpMatrixFrameStartPayload` | 5B | format, width, height (2B each) |
| `GpMatrixChunkHeader` | 3B | offset(2B), chunk_len(1B) |
| `GpMatrixLayerHeader` | 1B | total_layers[4], seq[4] |
| `GpMatrixTimePayload` | 6B | HH, MM, SS, YY, MM, DD |

### Command ID Reference

| Command | ID | Category |
|---|---|---|
| Ping | 0x01 | Diagnostic |
| SetBrightness | 0x02 | Parameter |
| SetMode | 0x03 | Parameter |
| StateHint | 0x04 | Parameter |
| SetAction | 0x05 | Control |
| SetDebugLed / SetDebugLedFlow | 0x06 / 0x07 | Debug |
| SetTime | 0x08 | Clock |
| RequestCachedBitmap | 0x09 | Recovery |
| FrameStart / FrameChunk / FrameCommit | 0x10 / 0x11 / 0x12 | Full Image |
| AnimationStart / AnimationFrame / AnimationEnd | 0x13 / 0x14 / 0x15 | Animation |
| LayeredFrame | 0x18 | Lightweight Image |
| LayeredAnimFrame | 0x19 | Lightweight Animation |
| ScrollGlyphStart / Chunk / Commit | 0x20 / 0x21 / 0x22 | Glyph |
| Heartbeat | 0x30 | Keep-alive |
