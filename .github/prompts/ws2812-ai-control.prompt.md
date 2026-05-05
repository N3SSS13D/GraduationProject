---
name: AI Side Action Mapping
description: "Implement one AI-side feature that maps voice or debug results into LED-side actions"
argument-hint: "Feature request (for example: command parser, action mapping, priority policy, snapshot control)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one `AI-side` integration feature.

## Structure-based file targeting

Use the four active categories to read only the narrow files needed for the task:

1. `AI-side interface orchestration`
   - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.h/.cc`
   - `Project/xiaozhi-esp32/main/gp_port/transport/`
   - `Project/xiaozhi-esp32/main/gp_port/ui/`
   - `Project/xiaozhi-esp32/main/boards/lichuang-dev/`
2. `Bluetooth communication protocol`
   - `Project/Protocols/gp_led_matrix_protocol.h`
   - `Project/Protocols/gp_led_matrix_protocol_spec.md`
   - `Project/Protocols/gp_matrix_pattern_protocol.md`
3. `Local drawing scripts`
   - `Project/Script/mcp/gp_matrix/`
  - `Project/Script/tools/ws2812_auto_debug.py`
4. `LED-side display driver`
   - Only read `Project/STC51/ws2812_driver/` when the interface boundary requires it

## Module quick map

- `main/gp_port/gp_led_matrix_esp32.h/.cc`
  - AI-side matrix orchestrator. Builds packets, sends commands and frames, waits for ACK/status, and maps debug-state
    objects into protocol payloads.
- `main/gp_port/transport/`
  - HC-05 / UART transport backend. Owns packet write/read abstraction and background packet assembly from the UART byte
    stream.
- `main/gp_port/ui/`
  - Local touch/debug UI and preview buffer. Use this layer when the task touches preview cards, touch callbacks, or
    local visualization state.
- `main/boards/lichuang-dev/`
  - Board-level integration. Wires the transport, matrix orchestrator, debug UI, websocket relay, MCP-facing hooks,
    and startup Bluetooth checks.
- `Project/Protocols/gp_led_matrix_protocol.h`
  - Shared single source of truth for packet fields, limits, and payload structs used by AI-side code.

## Problem-solving workflow

For implementation, integration, or bug-fix tasks:

1. Summarize the current implementation slice and the exact control/data path first.
2. List the current issues, risks, and candidate solutions before editing.
3. Choose the smallest feasible change and define validation criteria up front.
4. Implement one focused slice at a time instead of widening scope immediately.
5. Re-check the touched path after validation before moving to the next slice.
6. Sync docs, prompts, and skills when workflow, assumptions, or behavior expectations change.

## Requirements

1. Reuse existing action objects such as `voice_color_result` when possible.
2. If the task involves debug-menu input or preset-based color/effect updates, reuse the existing
   `voice_color_analyze -> voice_color_result` analysis path and allow `source` to be either `stt` or `touch`.
3. For freeform `16x16` draw requests from `stt` that do not map cleanly to an existing local preset, route them
   through the existing `matrix_pattern_request` path so voice-only drawing does not depend on a prior debug-menu
   `Draw` tap.
4. Keep the AI-side output consistent with the LED-side protocol fields.
5. Keep action delivery bounded, traceable, and validation-friendly.
6. Touch MCP or snapshot tooling only when directly required by the task.
7. Run the available build or integration validation after changes. After an `AI端` build succeeds, continue with
   flash by default unless the user explicitly requests build-only or the hardware path is unavailable.
8. For HC-05 tasks, assume all AT commands and queries finish at `38400` in strict set-then-query order, use
   `AT+BIND` with fixed slave address `98:D3:02:96:A2:B1` on the AI side, then make `AT+RESET` and the local switch
   to `460800` the final two steps, with reply logging visible in monitor.
9. Preserve the existing touch-to-Bluetooth path for direct LED-side state updates, but do not route debug-menu touch
   actions through `SendColorDebugAnalyze(..., "touch")` or other MCP/LLM-only paths when a local debug transport is
   required. For touch-triggered preview/data exchange, prefer a board-local debug websocket client and keep the path
   bounded and monitor-visible.
10. Preserve the event-driven UART receive model in `main/gp_port/transport/`; do not reintroduce per-call polling
    reads in `ReadPacket()`.
11. For `16x16` pattern preview tasks, prefer the host-side debug websocket path: the local Python script should run
    a dedicated websocket server for data transport, the `AI端` should connect as client and send touch-triggered
    requests, and the host should respond with compact preview payloads such as `bitmap_rows_hex + primary_rgb888`.
    Keep the Preview card layout fixed with the dot on the left and the preview slot on the right. Keep MCP focused
    on LLM-facing control/tool integration; use HTTP preview only as a secondary fallback for snapshots or generic
    PNG/JPEG preview debugging.
12. For LLM-facing MCP bridge scripts, use self-descriptive filenames, tool names, and argument names so the intent
    is obvious without reading the implementation. Prefer names like `gp_display_mcp_bridge.py`, `draw_python`,
    `show_text`, `python_source`, `eval_source`, `frame_interval_ms`, and `text`.
13. When the `AI端` needs to forward a `16x16` pattern to the `LED端` over Bluetooth, prefer compact `bitmap_rows +
    RGB888` transfer instead of expanding to a full `256`-byte RGB332 frame. Keep `FrameChunk` sizing consistent on
    both sides with the shared protocol constant `64` bytes.
14. When the task is a compact `16x16` animation sequence, prefer a dedicated MCP tool that emits
    `matrix_frame_sequence_v1` with LED-ready `bitmap_rows_hex` frames. Prefer `frames[]` as the LLM-facing input
    envelope (each frame uses exactly one of `bitmap_rows_hex`, `bitmap_rows`, or `python_source/eval_source`) or
    use `bitmap_rows_hex_list` with full-frame entries. Treat `bitmap_rows_hex` as the `32-byte` bitmap only: the
    canonical form is exactly `64` hex characters, `16` rows, `16` bits per row, `top row first`,
    `MSB = leftmost LED`, `1 = lit`, and `0 = dark`; tolerant bridge-side normalization may also accept common
    `16`-row hex-token forms. The full compact LED-ready frame is
    `bitmap_rows_hex + primary_rgb888 + background_rgb888 = 38 bytes`. Keep the LED-side buffer at `24` frames, use
    `frame_interval_ms` in the range `1..65535`, default to `42 ms`, and if the host receives more than `24` input
    frames, resample them to `24` while scaling the interval to preserve total duration. Route buffered playback
    through `matrix_animation_start -> indexed matrix_pattern_result -> matrix_animation_end` so the `AI端` can
    buffer preview frames before forwarding the full batch. Never rely on `yield_frame`, `time.sleep`, or
    import-based timing inside drawing statements.
15. For compact bitmap frame uploads that fit within one `64`-byte chunk, prefer waiting for ACK only on
    `FrameCommit`; avoid per-stage ACK waits on `FrameStart` and `FrameChunk` unless the task explicitly needs maximum
    transfer diagnostics.
16. For Bluetooth transport debugging, treat LED-side protocol logs such as `[GP_TX]`, `[GP_RX]`, `[GP_DROP]`, and
    `[GP_SYNC]` as authoritative. The raw `[BT_MON]` line is only a bounded UART sniff window and may clip long
    packets.
17. If a task needs to light the LED-side debug LEDs for transport verification, use the dedicated GP protocol
    debug-LED command instead of sending raw `LED n` text over HC-05.
18. If a task needs to verify sustained packet reception on the LED side, prefer an AI-side `1s` task that sends the
    GP debug-LED command only during link idle windows; make that background probe best-effort and avoid waiting for
    ACK so it does not interfere with foreground frame transfers.
19. When diagnosing why `SetAction` works but `FrameChunk` uploads fail, first inspect the LED-side UART receive
    cadence and packet assembly path before changing bitmap rendering logic.
20. After the LED side renders a remotely transferred frame, keep the last frame displayed until an explicit remote
    release or local control-mode switch occurs; do not clear it solely because the communication-active timeout
    expires.

## Common read bundles

- `Send / ACK / packet bugs`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
  - `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc`
- `Touch UI / preview bugs`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.h`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.cc`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
- `Host drawing / websocket / MCP relay`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`

## Useful tools

- `Project/Script/tools/ws2812_auto_debug.py`
- `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`

## Integration notes

- `Project/Script/tools/ws2812_auto_debug.py` is the default chain entry: Keil rebuild first, then delayed STC serial
  monitor, then ESP-IDF `build flash monitor`.
- Keil output is recorded in `Project/Debug/build/keil_build.log` for quick failure triage.

## Output format

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
