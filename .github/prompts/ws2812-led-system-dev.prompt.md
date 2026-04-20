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
0. Before analysis, coding, review, or refactoring, read and apply `.github/skills/karpathy-guidelines/SKILL.md` so assumptions, minimal edits, and verification criteria are explicit.
1. Analyze existing code and summarize what can be reused.
2. Propose a minimal implementation plan for only the requested feature.
3. Implement code changes directly in the workspace using existing style and naming conventions.
4. Preserve backward compatibility with default resolution 16x16, while allowing configurable resolutions.
5. If task scheduling is needed, implement a lightweight cooperative mini-OS style scheduler (non-preemptive) suitable for 8051 constraints.
6. If the feature touches XiaoZhi AI control, prefer mapping `voice_color_result` or an equivalent action object into display actions while staying compatible with `GP_Port/gp_led_matrix_protocol.h`.
7. Add or update concise technical documentation for integration and usage.
8. After each code change, automatically run the validation flow that matches the task; when the repository integration flow applies, use `tools/ws2812_dev_cycle.ps1` as the default automation entry.
9. When the change touches the `BT_Version` AI8051U Bluetooth debug path, prefer validating with `-RunAi8051BtDebug` so the AI8051 serial monitor opens automatically, sends `BT AT`, `BT AT+UART?`, and captures the returned logs.

Constraints:
- Keep timing-critical paths deterministic.
- Avoid dynamic memory allocation unless there is a clear reason.
- Prefer fixed-size buffers and explicit bounds checks.
- Keep dependency direction explicit: `app -> mid -> drv -> peripheral glue/vendor support`.
- Do not rewrite unrelated modules.

Current repository stage:
- The STC side already has a stable PWM + DMA scan/output path, 74HC595/PMOS row switching, and USB debug commands.
- `External/xiaozhi-esp32/GP_Port/` already contains a shared protocol header, an ESP32 matrix-driver skeleton, an AI8051U interface design, MCP debug tooling, and endpoint validation assets.
- The stable milestone is the end-to-end bridge from XiaoZhi AI to WS2812 actions over a custom I2C protocol. On `BT_Version`, the active local-debug milestone is the restored AI8051U `UART2(P4.2/P4.3) + HC-05` path with `P4.1` driving HC-05 `PIO11` AT mode, booting at `9600 8N1`, and using USB only to forward the structured `BT SEND/BT STATUS` debug command set plus UART2 send/receive logs.
- On `BT_Version`, keep the active runtime path on `gp_led_matrix_ai8051u -> UART2 -> HC-05`, run the AI8051U main clock at `33.1776MHz`, keep Timer2 reserved exclusively for UART2 baud generation, and let the local UART2 baudrate follow successful `AT+UART=...` reconfiguration requests.

Output format:
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification" (build/test or static checks actually run; if not run, state why)
- "Next steps"

Verification addendum:

- Do not stop at suggesting tests when source files changed; run the available automated validation directly.
- After STC51 source changes, run at least one Keil rebuild of `STC51/Project/ws2812_driver/ws2812_driver.uvproj` until it succeeds.
- If the change affects XiaoZhi, MCP, or AI8051U integration boundaries, also run the matching `tools/ws2812_dev_cycle.ps1` flow and record the actual result in "Verification".

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
	- The stable target is already mapped over I2C; on `BT_Version`, the immediate target is to keep the restored UART2 HC-05 transport aligned with the existing AI8051U packet parser and action-execution path.

### 2026-04-17

- Issue: the AI8051U matrix slave needed an I2C DMA backend without regressing the stable refresh path.
- Current conclusion:
	- The stable branch still uses bidirectional I2C DMA, while `BT_Version` restores the AI8051U protocol path over UART2 byte-stream assembly on `P4.2/P4.3`.
	- `GpLedMatrixAi8051u_SetDmaMode()` remains only as a compatibility stub on `BT_Version`.
	- Validation on `BT_Version` now needs to cover the `33.1776MHz` UART2 timing base, boot-time `9600 8N1`, the `BT SEND/BT STATUS` debug command surface, `P4.1 -> HC-05 PIO11` AT-mode control, local baud follow-up after `AT+UART=...`, and the verified target limit that the classic Bluetooth SPP backend cannot currently be linked on `ESP-IDF v5.4.3 + esp32s3`.
	- On `lichuang-dev`, the XiaoZhi-side debug UI now keeps the original main screen as default, exposes a `DBG` entry into a secondary debug menu, uses a fixed header (`Back / Debug Menu / S`), and keeps a stable single-page layout with touch controls, dot preview, link status, and summary information.
	- The current bidirectional link strategy is: AI8051U prepares an ACK reply immediately after a complete protocol packet is captured, while the main poll loop performs the actual LED action execution outside the ISR.
