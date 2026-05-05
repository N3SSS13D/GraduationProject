---
name: Bluetooth Protocol Change
description: "Implement one protocol-level change under Project/Protocols"
argument-hint: "Protocol task (for example: command field update, ACK flow, chunk size, drawing contract)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one `Bluetooth communication protocol` task.

## Structure-based file targeting

Start from protocol artifacts and only read consumers when needed:

1. `Bluetooth communication protocol`
   - `Project/Protocols/gp_led_matrix_protocol.h`
   - `Project/Protocols/gp_led_matrix_protocol_spec.md`
   - `Project/Protocols/gp_matrix_pattern_protocol.md`
2. `AI-side interface orchestration` (consumer verification)
   - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
3. `LED-side display driver` (consumer verification)
   - `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
4. `Local drawing scripts` (contract verification)
   - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`

## Problem-solving workflow

For protocol change or protocol bug tasks:

1. Summarize the current packet contract and the affected producer/consumer path first.
2. List the protocol risks, compatibility impact, and candidate solutions before editing.
3. Choose the smallest feasible protocol change and define validation criteria.
4. Implement one focused protocol slice at a time.
5. Re-check both docs and consumer alignment after validation before widening scope.
6. Sync docs, prompts, and skills when field semantics or workflow expectations change.

## Requirements

1. Treat `Project/Protocols/gp_led_matrix_protocol.h` as the single source of truth.
2. If packet layout or semantics change, update both protocol docs in the same task.
3. Keep AI-side and LED-side consumption aligned with the same fields and limits.
4. If script payload formats are affected, sync the corresponding script docs.
5. Keep backward compatibility impact explicit and testable.
6. Prioritize these protocol design goals: unambiguous framing, integrity checking, extensibility, parsing efficiency, and reliable interaction.
7. For active V2 work, prefer packet layouts that validate `header_size` and header CRC before trusting `payload_length`, use packet-type replies matched by `reply_to_sequence`, and use explicit byte offsets for staged chunks.

## Output format

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Compatibility notes`
