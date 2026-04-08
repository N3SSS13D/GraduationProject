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

Layered directory structure (must follow):
- App layer (business logic): `Sources/app/`
- Mdl/Mid layer (hardware-independent algorithms/protocols): `Sources/fml/`
- Drv layer (device-specific drivers): `Sources/lib/` and `Sources/output/` when output-driver split already exists
- HAL layer (MCU register/vendor-lib access): `Sources/hal/`

Placement rule:
- Put new/changed files in the matching layer directory.
- If current codebase naming differs, keep compatibility and add the minimal mapping note in documentation instead of broad folder migration.

Execution requirements:
1. Analyze existing code and summarize what can be reused.
2. Propose a minimal implementation plan for only the requested feature.
3. Implement code changes directly in the workspace using existing style and naming conventions.
4. Preserve backward compatibility with default resolution 16x16, while allowing configurable resolutions.
5. If task scheduling is needed, implement a lightweight cooperative mini-OS style scheduler (non-preemptive) suitable for 8051 constraints.
6. If the requested feature mentions XiaoZhi AI control, add a clean input interface layer that translates AI commands to display actions.
7. Add or update concise technical documentation for integration and usage.

Constraints:
- Keep timing-critical paths deterministic.
- Avoid dynamic memory allocation unless there is a clear reason.
- Prefer fixed-size buffers and explicit bounds checks.
- Enforce strict layer boundaries: App can call Mdl/Mid and Drv APIs, Drv can call HAL, HAL must not depend on App/Mdl.
- Do not rewrite unrelated modules.

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
