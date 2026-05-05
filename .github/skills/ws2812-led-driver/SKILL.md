---
name: ws2812-led-driver
description: 'Modify or review the LED-side display driver. Use when changing AI8051U WS2812 scan timing, Keil project files, action execution, protocol receive/ACK handling, rendering flow, or STC51 driver layout under Project/STC51.'
user-invocable: true
disable-model-invocation: false
---

# WS2812 LED-side Display Driver

## Category

`LED端显示驱动`

## Scope

Use this skill for LED-side tasks under `Project/STC51/`, especially:

- `Project/STC51/ws2812_driver/Sources/app/`
- `Project/STC51/ws2812_driver/Sources/drv/`
- `Project/STC51/ws2812_driver/Sources/mid/`
- `Project/STC51/ws2812_driver/Sources/inc/`
- `Project/STC51/ws2812_driver/ws2812_driver.uvproj`

## Read only what is needed

- For scan timing and render logic, prefer `Sources/drv/`, `Sources/mid/`, and the matching headers.
- For build path issues, prefer `ws2812_driver.uvproj` and related Keil metadata files.
- Do not read `AI-side interface orchestration` or `Local drawing scripts` files unless the LED-side interface boundary
  requires it.

## Module quick map

- `Sources/app/app.c`
  - Runtime entry that initializes WS2812, draw driver, protocol receive path, and cooperative tasks.
- `Sources/mid/mid_task.c`
  - 1 ms cooperative scheduler used by the app task graph.
- `Sources/mid/gp_led_action.c`
  - Decision layer between local draw content and remotely controlled action/frame/animation execution.
- `Sources/mid/draw_drv.c`
  - Local/offline rendering for solid, glyph, pattern, and animation updates.
- `Sources/drv/gp_led_matrix_ai8051u.c`
  - UART2 byte-stream assembly, packet parsing, command dispatch, and ACK/reply generation.
- `Sources/drv/ws2812_drv.c`
  - Physical 16x16 WS2812 scan driver and low-level display output.

## Common execution flow

- Boot path: `main.c -> Test_Init() -> app.c init`
- Runtime loop: `GpLedMatrixAi8051u_Poll() -> MidTask_Process()`
- Remote frame path: `gp_led_matrix_ai8051u.c -> gp_led_action.c -> ws2812 / draw layers`
- Offline animation path: `mid_task tick -> draw_drv.c` updates when direct remote frame mode is not active

## Common read bundles

- `Packet / ACK / command bugs`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
  - `Project/STC51/ws2812_driver/Sources/mid/gp_led_action.c`
- `Render / display bugs`
  - `Project/STC51/ws2812_driver/Sources/app/app.c`
  - `Project/STC51/ws2812_driver/Sources/mid/draw_drv.c`
  - `Project/STC51/ws2812_driver/Sources/drv/ws2812_drv.c`
- `Build / path bugs`
  - `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
  - matching `.uvopt` / `.uvgui.*` files

## Optimization workflow

For LED-side refresh, animation, or scheduler optimization work:

1. Summarize the current implementation, task cadence, and hot path first.
2. State the current bottlenecks, risks, and candidate optimizations before editing.
3. Prefer the smallest feasible change that can be validated independently.
4. Validate each focused slice before moving to the next optimization.
5. Do a second-pass review after validation, then sync docs and prompt/skill guidance if the change updates timing or workflow expectations.

## Requirements

1. Preserve the stable WS2812 scan/output path unless the task explicitly changes it.
2. Keep hardware-driver changes inside the LED-side category.
3. If packet fields or shared constants change, update `Project/Protocols/` as part of the same task.
4. If task behavior changes the expected debugging or verification flow, update the matching current doc under
   `Doc/Instructions/` or `Project/STC51/`.
5. After source-code changes, rebuild `Project/STC51/ws2812_driver/ws2812_driver.uvproj`.
