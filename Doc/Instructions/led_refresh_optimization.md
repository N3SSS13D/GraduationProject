# LED Refresh And Scheduling Optimization

## Scope

This document tracks the current `LED-side` refresh, animation, and scheduler optimization work under:

- `Project/STC51/ws2812_driver/Sources/drv/ws2812_drv.c`
- `Project/STC51/ws2812_driver/Sources/mid/draw_drv.c`
- `Project/STC51/ws2812_driver/Sources/mid/mid_task.c`
- `Project/STC51/ws2812_driver/Sources/app/app.c`

## Current scheme summary

### 1. Bottom refresh path

- `Timer1 ISR -> WS2812DRV_RefreshStep()` drives row refresh.
- `WS2812DRV_RefreshStep()` currently performs row-pair buffer preparation, row select switching, and DMA trigger.
- PWM row data is double-buffered, but the dual-row DMA payload is still assembled on the hot path.

### 2. Offline render and animation path

- `DrawDrv_Task()` advances local effect state and rebuilds one full frame when dirty.
- Local draw cadence now comes from `DrawDrv_RenderConfig.frameIntervalMs`; the default fallback remains `32 ms`.
- `scrollStep` and `animStep` remain per-step amplitude controls, while `frameIntervalMs` is the time base.
- `DrawDrv_RebuildFrame()` writes the active image region into the WS2812 back buffer and then calls `WS2812DRV_EncodeAllRows()`.
- Offline patterns are provided by `offline_pattern.c`.

### 3. Cooperative scheduler path

- `Timer0` provides the 1 ms base tick.
- `MidTask_Tick1ms()` updates software task counters.
- `MidTask_Process()` runs due tasks from the main loop.
- `app.c` keeps the cooperative local draw task period synchronized to the active `frameIntervalMs`.
- `GpLedAction_Tick1ms()` keeps remote animation playback on raw 1 ms cadence.

## Problems found before optimization

### 1. Repeated full-buffer work in the local rebuild path

- `WS2812DRV_ClearImage()` previously performed DMA zero-fill and then iterated the full image buffer again in software.
- Full-frame local rebuilds and remote direct-frame writes also cleared the full back buffer before immediately overwriting all effective pixels.

### 2. Animation cadence mismatch

- `draw_drv.c` used `DRAWDRV_TASK_STEP_MS = 32U`.
- `app.c` registered the draw-frame task with `36U`.
- Timeline and effect progression therefore did not match the actual scheduler cadence.

### 3. Empty periodic draw task

- The registered 500 ms draw-animation task only reset an unused local counter.
- This consumed scheduler slots without changing visible behavior.

### 4. Cooperative scheduler could silently drop due ticks

- `MidTask` previously used a one-bit `run` flag.
- If the main loop was delayed, multiple elapsed periods collapsed into one execution.

## Implemented changes in this round

### 1. Optimized clear path in `ws2812_drv.c`

- Removed the redundant software full-buffer clear after successful DMA zero-fill.
- Added a software fallback path when DMA memory-to-memory clear is unavailable.

### 2. Removed redundant pre-clear before full-frame writes

- `DrawDrv_RebuildFrame()` now starts fast frame write directly.
- `GpLedAction_BeginDirectFrame()` also skips pre-clear because direct-frame paths rewrite the complete `16x16` payload.

### 3. Made offline draw cadence configurable with a 32 ms default

- `DrawDrv_RenderConfig.frameIntervalMs` is now the local effect time base.
- `app.c` stores the draw-task id and uses `MidTask_SetPeriod()` to retune the cooperative task when `frameIntervalMs`
  changes.
- `draw_drv.c` now advances timeline-style effects by the active configured interval instead of a hard-coded `32 ms`.
- `Timer1` row-scan timing remains independent; changing local effect speed must not change the physical scan interval.

### 4. Removed the no-op 500 ms draw task

- Deleted the empty `DrawDrv_Task500ms()` path and its scheduler registration.
- Removed the dead `g_drawImageTick` state.

### 5. Upgraded `MidTask` pending semantics

- Replaced the single `run` flag with a saturating `pendingCount`.
- Each scheduler pass still executes at most one instance per task, preserving fairness while avoiding silent tick collapse.

### 6. Rolled back the cached normal-pair DMA payload experiment

- The previous `normal pair` cache experiment was removed from the active refresh path after board-side output corruption was observed.
- The current stable scheme again builds the dual-row DMA payload on demand inside `WS2812DRV_RefreshStep()` for both stable row-pair output and simpler diagnosis.
- Analysis of the reverted code path showed that the cached DMA source buffers did not carry the same explicit even-address alignment guarantee as the legacy shared DMA buffer.

### 7. Added DMA alignment guard and later removed always-on runtime diagnostics

- `WS2812DRV_TriggerDualRowDma()` now records the last DMA source address and rejects odd-address sources instead of silently streaming corrupted PWM data.
- The driver now keeps refresh-side counters for refresh-step executions, DMA triggers, DMA completions, DMA wait timeouts, and odd-address source rejects.
- The earlier always-on USB scan baseline (`[LED_SCAN]` / `[LED_LAST]`) and the 50 ms BT sniff task were removed from the default playback path after they proved unnecessary for steady-state local playback and added avoidable cooperative-scheduler and `printf` overhead.
- Runtime diagnosis is now expected to be enabled explicitly when needed, rather than running continuously during local effect playback.

## Remaining hotspot

The largest remaining refresh-side hotspot is again the active dual-row assembly and row-switch work around the refresh path:

- `normal pair` mode has returned to on-demand dual-row DMA assembly inside `WS2812DRV_RefreshStep()`.
- `legacy shift` mode still assembles the dual-row DMA payload on the refresh hot path.
- Row-select blanking, settle delay, and DMA trigger setup are still paid on every refresh step.
- Always-on debug task polling, baseline USB logging, and no-op row-switch debug hooks have been removed, so the remaining hotspot is now closer to the actual render/scan work instead of instrumentation overhead.
- If later measurement shows these dominate scan time, the next optimization should focus on a properly aligned cache design, legacy-path reuse, row-switch policy, or further setup reduction inside `WS2812DRV_RefreshStep()`.

## Required workflow for future optimization tasks

When continuing LED-side refresh, animation, or scheduler optimization, use this sequence:

1. Summarize the current implementation and identify the hot path.
2. List problems, risks, and candidate optimizations.
3. Choose the smallest feasible change and define validation criteria.
4. Implement one focused slice.
5. Perform a second-pass review after validation.
6. Sync docs, prompts, and skills if assumptions, timing rules, or workflow expectations changed.

## Verification status

This round was validated by Keil rebuilds:

- `Project/Debug/build/keil_build_led_opt_v1.log`
- `Project/Debug/build/keil_build_led_opt_v2.log`
- `Project/Debug/build/keil_build_led_opt_v3.log`
- `Project/Debug/build/keil_build_led_opt_v4.log`
- `Project/Debug/build/keil_build_led_dma_revert_v1.log`
- `Project/Debug/build/keil_build_led_dma_revert_v2.log`
- `Project/Debug/build/keil_build_led_dma_revert_v3.log`

Additional validation for the configurable local draw cadence:

- Keil rebuild of `Project/STC51/ws2812_driver/ws2812_driver.uvproj` on `2026-05-11`

Current result: `0 Error(s), 0 Warning(s)`.
