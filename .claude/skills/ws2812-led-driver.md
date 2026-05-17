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
- Do not read `AI-side interface orchestration` or `Local drawing scripts` files unless the LED-side interface boundary requires it.

## Module quick map

- `Sources/app/app.c` — Runtime entry: init WS2812, draw driver, protocol receive path, cooperative tasks.
- `Sources/mid/mid_task.c` — 1 ms cooperative scheduler used by the app task graph.
- `Sources/mid/gp_led_action.c` — Decision layer between local draw content and remote action/frame/animation execution.
- `Sources/mid/draw_drv.c` — Local/offline rendering: solid, glyph, pattern, and animation updates.
- `Sources/mid/local_display_scheme.c` — Local startup carousel + offline P32/P33 button behavior.
- `Sources/mid/offline_pattern.c` — 6 offline patterns stored as `36B` single-layer bitmap resources.
- `Sources/mid/rtc_clock.c` — 3-zone software RTC clock module with 3x5 digit font.
- `Sources/drv/gp_led_matrix_ai8051u.c` — UART2 byte-stream assembly, packet parsing, command dispatch, ACK/reply generation.
- `Sources/drv/ws2812_drv.c` — Physical 16x16 WS2812 scan driver and low-level display output.
- `Sources/drv/hc595_drv.c` — 74HC595 row selector shift register driver.

## Common execution flow

- Boot path: `main.c -> APP_Init() -> app.c init`
- Runtime loop: `GpLedMatrixAi8051u_Poll() -> MidTask_Process()`
- Remote frame path: `gp_led_matrix_ai8051u.c -> gp_led_action.c -> ws2812 / draw layers`
- Offline animation path: `mid_task tick -> draw_drv.c` when remote frame mode is not active
- Keep `Timer1 -> WS2812DRV_RefreshStep()` scan timing separate from local `DrawDrv` cadence.
- If you add a local display effect, keep it local to `DrawDrv` / `local_display_scheme.c`; only extend `Project/Protocols/` when the AI side must trigger it.

## Common read bundles

- `Packet / ACK / command bugs` → `Project/Protocols/gp_led_matrix_protocol.h` + `Sources/drv/gp_led_matrix_ai8051u.c` + `Sources/mid/gp_led_action.c`
- `Render / display bugs` → `Sources/app/app.c` + `Sources/mid/draw_drv.c` + `Sources/drv/ws2812_drv.c`
- `Build / path bugs` → `ws2812_driver.uvproj` + matching `.uvopt`/`.uvgui.*` files
- `Display parameter structure` → `Doc/Instructions/led_display_profile_structure.md`

## Optimization workflow

1. Summarize the current implementation, task cadence, and hot path first.
2. State the current bottlenecks, risks, and candidate optimizations before editing.
3. Prefer the smallest feasible change that can be validated independently.
4. Validate each focused slice before moving to the next optimization.
5. Do a second-pass review after validation, then sync docs if the change updates timing or workflow expectations.

## Requirements

1. Preserve the stable WS2812 scan/output path unless the task explicitly changes it.
2. Keep hardware-driver changes inside the LED-side category.
3. If packet fields or shared constants change, update `Project/Protocols/` as part of the same task.
4. If task behavior changes expected debugging or verification flow, update docs under `Doc/Instructions/` or `Project/STC51/`.
5. After source-code changes, rebuild `Project/STC51/ws2812_driver/ws2812_driver.uvproj`.
