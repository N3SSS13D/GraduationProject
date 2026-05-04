---
name: WS2812 structured code review
description: "Review changes through the four project categories: LED, AI, protocol, and local scripts"
argument-hint: "Review target (commit, files, module, or feature description)"
agent: agent
model: "GPT-5 (copilot)"
---
Review the specified target.

## Structure-based file targeting

Check changes against these four categories and read only the relevant files:

1. `LED-side display driver`: `Project/STC51/`
2. `AI-side interface orchestration`: `Project/xiaozhi-esp32/main/gp_port/`
3. `Bluetooth communication protocol`: `Project/Protocols/`
4. `Local drawing scripts`: `Project/Script/`

Category review entry points:

- `LED-side`
  - `Project/STC51/ws2812_driver/Sources/app/app.c`
  - `Project/STC51/ws2812_driver/Sources/mid/gp_led_action.c`
  - `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
  - `Project/STC51/ws2812_driver/Sources/drv/ws2812_drv.c`
- `AI-side`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
  - `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.cc`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
- `Protocol`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/Protocols/gp_led_matrix_protocol_spec.md`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
- `Scripts`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
  - `Project/Script/tools/ws2812_auto_debug.py`

Priorities:

1. Functional bugs and regressions
2. Timing risks in the LED-side execution path
3. Bluetooth transport and protocol consistency risks
4. Buffer bounds and memory safety issues
5. Validation gaps

Category-specific checks:

- `LED-side`: verify the runtime path `app.c -> protocol poll / scheduler -> gp_led_action -> draw/ws2812` stays
  coherent.
- `AI-side`: verify `board -> ui/debug state -> gp_led_matrix_esp32 -> transport` stays event-driven and bounded.
- `Protocol`: verify shared constants, payload sizes, and command docs remain aligned.
- `Scripts`: verify host drawing still targets AI-side preview/upload interfaces instead of bypassing into raw LED-side
  packet assumptions.

If the review touches `Project/xiaozhi-esp32/main/gp_port/`, also verify:

- field consistency between AI-side action objects and LED-side protocol fields
- transport, ACK, and status-reporting coherence

Output format:

- `Findings`
- `Open questions / assumptions`
- `Change summary`
- `Test and validation gaps`
