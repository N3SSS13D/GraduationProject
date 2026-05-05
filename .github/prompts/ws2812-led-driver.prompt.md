---
name: LED Side Driver Change
description: "Implement one LED-side display driver change under Project/STC51"
argument-hint: "Task request (for example: protocol receive fix, action execution, ws2812 timing, render path)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one `LED-side` feature or fix.

## Structure-based file targeting

Read only the narrow files needed for this task:

1. `LED-side display driver`
   - `Project/STC51/ws2812_driver/Sources/app/`
   - `Project/STC51/ws2812_driver/Sources/mid/`
   - `Project/STC51/ws2812_driver/Sources/drv/`
   - `Project/STC51/ws2812_driver/Sources/inc/`
   - `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
2. `Bluetooth communication protocol` (only when interface fields are involved)
   - `Project/Protocols/gp_led_matrix_protocol.h`
   - `Project/Protocols/gp_led_matrix_protocol_spec.md`

## Module quick map

- `Sources/app/app.c`
  - Boot init and runtime loop (`GpLedMatrixAi8051u_Poll -> MidTask_Process`).
- `Sources/mid/gp_led_action.c`
  - Remote action/frame/animation execution and local/remote control switching.
- `Sources/drv/gp_led_matrix_ai8051u.c`
  - UART2 packet assembly, command dispatch, ACK/reply behavior.
- `Sources/drv/ws2812_drv.c`
  - Physical 16x16 WS2812 scan output.

## Optimization workflow

For `LED-side` refresh, animation, or scheduler optimization tasks:

1. Summarize the current implementation and hot path before editing.
2. List current issues, risks, and candidate optimizations.
3. Choose the smallest feasible change and define validation criteria first.
4. Implement one focused slice at a time.
5. Re-check the touched path after validation before widening scope.
6. Sync docs, prompt, and skill content when timing rules or workflow expectations change.

## Requirements

1. Keep edits inside `Project/STC51/` unless protocol boundary requires shared updates.
2. Keep packet fields and constants aligned with `Project/Protocols/gp_led_matrix_protocol.h`.
3. Preserve event-driven runtime path; avoid introducing blocking loops in the main execution flow.
4. Keep changes minimal and traceable to the requested behavior.
5. After source changes, run a Keil rebuild for `Project/STC51/ws2812_driver/ws2812_driver.uvproj`.
6. If behavior changes timing or output rules, sync docs in `Doc/Instructions/` and `Project/STC51/README.md`.

## Output format

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
