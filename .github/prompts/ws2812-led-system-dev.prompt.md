---
name: WS2812 Bluetooth Workflow
description: "Implement one incremental feature for the current AI-side to LED-side Bluetooth workflow"
argument-hint: "Feature to implement (for example: action mapping, Bluetooth packet handling, ACK path, performance optimization)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one feature that belongs to the current Bluetooth-focused workflow.

Current workflow:

- `AI side`: `External/xiaozhi-esp32/`
- `LED side`: `STC51/Project/ws2812_driver/`
- transport: `AI side -> HC-05 -> LED side UART2(P4.2/P4.3)`
- default HC-05 setup flow: first probe with `AT` at `38400`; if there is still no reply after `3` attempts, switch the local UART directly to `460800` and skip the remaining setup; if the probe succeeds, keep all AT commands and queries at `38400` in strict set-then-query order, bind the AI side to fixed slave address `98:D3:02:96:A2:B1` with `AT+BIND`, then make `AT+RESET` and the local baud switch to `460800` the final two steps, with `XiaoZhi -> WS2812`.
- LED-side UART2 should default to DMA-based TX/RX with task-driven batch consumption instead of byte-by-byte UART interrupts.

Read first:

- `.github/prompts/README.md`
- `Doc/项目文档/problem_tracking.md`
- `External/xiaozhi-esp32/GP_Port/gp_led_matrix_esp32.h/.cc`
- `External/xiaozhi-esp32/GP_Port/transport/`
- `STC51/Project/ws2812_driver/Sources/`
- `tools/ws2812_dev_cycle.py`

Requirements:

1. Read `.github/skills/karpathy-guidelines/SKILL.md` before work.
2. Inspect only the files directly related to the request.
3. Keep changes minimal and keep solved issues out of prompts.
4. Preserve protocol consistency between the AI side and the LED side.
5. If the task touches performance, explain what was optimized and how it was verified.
6. Run the available validation after code changes.
7. Keep `SetAction` behavior aligned across `solid`, `pattern`, and `glyph` content; avoid AI-side combinations that the LED-side renderer cannot execute.
8. Treat DrawDrv as the shared online render path for animated solid, pattern, and glyph actions; direct-frame mode is only for explicit frame or glyph-row uploads.

Default validation:

- Rebuild `STC51/Project/ws2812_driver/ws2812_driver.uvproj` after LED-side source changes.
- Use `tools/ws2812_dev_cycle.py` for integration work.
- After an `AI side` build succeeds, continue with flash by default unless the user explicitly requests build-only or the hardware path is unavailable.
- Use `-RunAi8051BtDebug` when the LED-side Bluetooth path changes.

Integration notes:

- `tools/ws2812_dev_cycle.py` stores per-cycle logs in `debug_snapshots/dev_cycle_logs/`.
- Prefer checking bounded LED-side serial captures when reviewing Bluetooth behavior changes.

Output format:

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
