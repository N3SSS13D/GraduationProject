---
name: bluetooth-protocol
description: 'Modify or review the shared Bluetooth communication protocol. Use when changing packet layout, command fields, ACK/status flow, chunk sizing, matrix_pattern_request contracts, shared protocol headers, or AI/LED protocol consistency under Project/Protocols.'
user-invocable: true
disable-model-invocation: false
---

# Bluetooth Communication Protocol

## Category

`蓝牙通信协议`

## Scope

Use this skill for protocol tasks under `Project/Protocols/`:

- `Project/Protocols/gp_led_matrix_protocol.h`
- `Project/Protocols/gp_led_matrix_protocol_spec.md`
- `Project/Protocols/gp_matrix_pattern_protocol.md`

## Read only what is needed

- Start with `Project/Protocols/` files.
- Read `AI端` or `LED端` implementation files only when verifying how a protocol field is consumed.
- Avoid scanning unrelated script or UI files unless the protocol boundary directly touches them.

## Artifact quick map

- `Project/Protocols/gp_led_matrix_protocol.h`
  - Shared single source of truth for wire constants, command IDs, payload structs, and transfer limits.
- `Project/Protocols/gp_led_matrix_protocol_spec.md`
  - Packet-level behavior: layout, staged frame transfer, animation transfer, and ACK/status expectations.
- `Project/Protocols/gp_matrix_pattern_protocol.md`
  - Host/script-facing drawing contract that feeds AI-side preview and Bluetooth upload.

## Common protocol flow

- Host drawing contract -> `matrix_pattern_request` / `matrix_pattern_result`
- AI-side sender -> `gp_led_matrix_esp32.cc`
- Wire packet layout and limits -> `gp_led_matrix_protocol.h`
- LED-side parser/executor -> `Sources/drv/gp_led_matrix_ai8051u.c`

## Common read bundles

- `Wire format / packet field changes`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/Protocols/gp_led_matrix_protocol_spec.md`
- `Host drawing contract changes`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
- `Consumer verification`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
  - `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`

## Problem-solving workflow

For protocol changes and protocol bug fixes:

1. Summarize the current contract and affected producer/consumer path first.
2. State compatibility risks, failure modes, and candidate solutions before editing.
3. Prefer the smallest feasible change that can be validated independently.
4. Validate consumer alignment after each focused protocol slice.
5. Sync docs and prompt/skill guidance when protocol semantics or workflow expectations change.

## Requirements

1. Keep AI-side and LED-side field definitions aligned.
2. Treat `Project/Protocols/gp_led_matrix_protocol.h` as the shared single source of truth for active constants and
   payload structs.
3. If protocol behavior changes, update both the shared header and the matching protocol docs in `Project/Protocols/`.
4. If a protocol change affects script payload formats or MCP expectations, update the matching docs under
   `Project/Script/`.
5. When choosing a protocol shape, prioritize unambiguous framing, integrity checks, extensibility, parsing efficiency,
  and reliable interaction.
6. For the active V2 wire format, prefer header-first validation (`header_size` + CRC8), packet-type replies matched by
  `reply_to_sequence`, and explicit byte offsets for staged frame or glyph chunks.
