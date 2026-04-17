---
name: WS2812 LED System Developer
description: "Design and implement one incremental feature for the STC AI8051U + WS2812 multiplexed display system"
argument-hint: "Feature to implement (e.g., 16x16 text scroll, 8x32 mode, low-jitter scheduler, XiaoZhi AI pattern bridge)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one incremental feature for this WS2812 LED display project, based on the user argument.

Project hardware architecture (fixed constraints):
- MCU: STC AI8051U
- Power switching: 16 PMOS high-side switches
- Shift register control: 2 cascaded 74HC595 controlling PMOS on/off states
- Data lines: odd rows share one PWM signal line, even rows share one PWM signal line
- Drive method: PWM + DMA
- Scan strategy: power two adjacent rows at a time; line A fades out pixel-by-pixel while line B fades in pixel-by-pixel; then move switch window by one row and repeat

Source context to inspect first:
- Main driver sources: [STC51/Project/ws2812_driver/Sources](../../STC51/Project/ws2812_driver/Sources)
- Existing low-level project: [STC51/Project/PWMA-DMA/AI8051U-PWMA-DMA-24灯环-1通道.c](../../STC51/Project/PWMA-DMA/AI8051U-PWMA-DMA/WS2812B-PWMA-DMA-24灯环-1通道.c)

Current structure to follow first:
- App layer: `Sources/app/`
- Mid layer: `Sources/mid/`
- Driver layer: `Sources/drv/`
- Shared headers and config: `Sources/inc/`
- Peripheral glue and vendor support: `Sources/*.c` and `Sources/lib/`

Placement rule:
- Reuse the current repository layout rather than forcing a broad folder migration.
- Place business orchestration in `app/`, reusable state/effect logic in `mid/`, and timing/hardware code in `drv/`.
- If the task touches XiaoZhi integration, inspect `External/xiaozhi-esp32/GP_Port/` first for reusable protocol and interface assets.

Execution requirements:
1. Analyze existing code and summarize what can be reused.
2. Propose a minimal implementation plan for only the requested feature.
3. Implement code changes directly in the workspace using existing style and naming conventions.
4. Preserve backward compatibility with default resolution 16x16, while allowing configurable resolutions.
5. If task scheduling is needed, implement a lightweight cooperative mini-OS style scheduler (non-preemptive) suitable for 8051 constraints.
6. If the feature touches XiaoZhi AI control, prefer mapping `voice_color_result` or an equivalent action object into display actions while staying compatible with `GP_Port/gp_led_matrix_protocol.h`.
7. Add or update concise technical documentation for integration and usage.

Constraints:
- Keep timing-critical paths deterministic.
- Avoid dynamic memory allocation unless there is a clear reason.
- Prefer fixed-size buffers and explicit bounds checks.
- Keep dependency direction explicit: `app -> mid -> drv -> peripheral glue/vendor support`.
- Do not rewrite unrelated modules.

Current repository stage:
- The STC side already has a stable PWM + DMA scan/output path, 74HC595/PMOS row switching, and USB debug commands.
- `External/xiaozhi-esp32/GP_Port/` already contains a shared protocol header, an ESP32 matrix-driver skeleton, an AI8051U interface design, MCP debug tooling, and endpoint validation assets.
- The next milestone is an end-to-end bridge from XiaoZhi AI to WS2812 actions over a custom I2C protocol.

Output format:
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification" (build/test or static checks actually run; if not run, state why)
- "Next steps"

If the request is ambiguous, ask only the minimum clarifying questions needed to continue.

---

## Iteration Issue Summary (append-only)

### 2026-04-08

- Issue: occasional channel-group disorder under dual-channel PWM+DMA output.
- Root causes: pair-length alignment, DMA source address alignment, tail-boundary sensitivity, and weak fault recovery.
- Applied fixes:
	- enforce even TX length,
	- enforce aligned DMA source pointer,
	- append a CH1/CH2 zero tail guard pair,
	- add DMA timeout + recovery,
	- keep fixed even-row->CH1 and odd-row->CH2 mapping.
- Architecture update: image buffering and PWM+DMA pipeline moved into `ws2812_drv`; `test.c` now focuses on scheduling and fixed test pattern generation.

### 2026-04-12

- Issue: XiaoZhi AI assets and MCP-based debug tooling have been imported into this repository as references for the graduation-project bridge.
- Current conclusion:
	- `External/xiaozhi-esp32/GP_Port/` now contains protocol notes, an ESP32 driver skeleton, an AI8051U interface header, debug-dot tooling, and endpoint validation scripts.
	- The official MCP bridge actively sends `initialize`, so test tools must behave as an MCP server.
	- The next practical target is to map XiaoZhi voice output into AI8051U-executable WS2812 actions over I2C.

### 2026-04-17

- Issue: the AI8051U matrix slave needed an I2C DMA backend without regressing the stable refresh path.
- Current conclusion:
	- The active implementation is a hybrid backend: interrupt-driven slave RX remains the stable default, while reply TX can use I2C TX DMA.
	- RX DMA is available behind `GpLedMatrixAi8051u_SetDmaMode()` but is not the default path until hardware validation confirms it does not perturb LED brightness or scan timing.
	- Validation now needs to cover both protocol correctness and DMA-side fault signals such as `RXLOSS`, `TXOVW`, and reply-length mismatches.
