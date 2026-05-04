---
name: Local Script and MCP Change
description: "Implement one local drawing script or MCP tooling change under Project/Script"
argument-hint: "Script task (for example: MCP bridge behavior, payload normalization, auto-debug tooling)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one `Local drawing scripts` task.

## Structure-based file targeting

Read only relevant script assets first:

1. `Local drawing scripts`
   - `Project/Script/mcp/gp_matrix/`
   - `Project/Script/tools/`
   - `Project/Script/media_tools/`
2. `Bluetooth communication protocol` (contract alignment)
   - `Project/Protocols/gp_matrix_pattern_protocol.md`
   - `Project/Protocols/gp_led_matrix_protocol.h`
3. `AI-side interface orchestration` (integration boundary)
   - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
   - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`

## Requirements

1. Keep MCP tool names and argument names self-descriptive.
2. Keep script payload formats aligned with active protocol docs.
3. Keep host flow bounded to AI-side preview/upload interfaces.
4. Do not introduce assumptions that bypass AI-side orchestration and directly depend on LED-side raw serial details.
5. If script workflow changes, sync `Project/Script/README.md` and `Doc/Instructions/problem_tracking.md`.
6. Treat `Project/Script/tools/ws2812_auto_debug.py` as the default automation entry.
7. For the auto-debug chain, enforce this order by default:
   - Keil rebuild for `Project/STC51/ws2812_driver/ws2812_driver.uvproj` only
   - wait `20s` and open AI8051U serial monitor (`COM15` default, adjustable)
   - run ESP-IDF `build flash monitor` for `Project/xiaozhi-esp32`
8. Keep tool roots configurable and validated before execution:
   - Keil root: `S:\Embedded\Keil`
   - ESP-IDF root: `S:\Embedded\ESP\v5.4.3\esp-idf`
9. Remove or update stale references when the automation entry changes; do not keep dead links to legacy scripts.

## Output format

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Operational notes`
