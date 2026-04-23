---
name: AI Side Action Mapping
description: "Implement one AI-side feature that maps voice or debug results into LED-side actions"
argument-hint: "Feature request (for example: command parser, action mapping, priority policy, snapshot control)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one AI-side integration feature.

Focus paths:

- `External/xiaozhi-esp32/GP_Port/gp_led_matrix_esp32.h/.cc`
- `External/xiaozhi-esp32/GP_Port/transport/`
- `External/xiaozhi-esp32/GP_Port/ui/`
- `External/xiaozhi-esp32/main/boards/lichuang-dev/`
- `External/xiaozhi-esp32/GP_Port/gp_led_matrix_protocol.h`

Requirements:

1. Reuse existing action objects such as `voice_color_result` when possible.
2. If the task involves debug-menu input, reuse the existing `voice_color_analyze -> voice_color_result` analysis path and allow `source` to be either `stt` or `touch`.
3. Keep the AI-side output consistent with the LED-side protocol fields.
4. Keep action delivery bounded, traceable, and validation-friendly.
5. Touch MCP or snapshot tooling only when directly required by the task.
6. Run the available build or integration validation after changes.
7. For HC-05 tasks, assume all AT commands and queries finish at `38400` in strict set-then-query order, use `AT+BIND` with fixed slave address `98:D3:02:96:A2:B1` on the AI side, then make `AT+RESET` and the local switch to `460800` the final two steps, with reply logging visible in monitor.
8. Preserve the existing touch-to-Bluetooth path for direct LED-side state updates, but do not route debug-menu touch actions through `SendColorDebugAnalyze(..., "touch")` or other MCP/LLM-only paths when a local debug transport is required. For touch-triggered preview/data exchange, prefer a board-local debug websocket client and keep the path bounded and monitor-visible.
9. Preserve the event-driven UART receive model in `GP_Port/transport/`; do not reintroduce per-call polling reads in `ReadPacket()`.
10. For `16x16` pattern preview tasks, prefer the host-side debug websocket path: the local Python script should run a dedicated websocket server for data transport, the `AI端` should connect as client and send touch-triggered requests, and the host should respond with compact preview payloads such as `bitmap_rows_hex + primary_rgb888`. The `AI端` should render the returned pattern locally in the Preview area and print websocket configuration and payload logs in monitor. Keep MCP focused on LLM-facing control/tool integration; use HTTP preview only as a secondary fallback for snapshots or generic PNG/JPEG preview debugging.
11. For LLM-facing MCP bridge scripts, use self-descriptive filenames, tool names, and argument names so the intent is obvious without reading the implementation. Prefer names like `gp_display_mcp_bridge.py`, `draw_python`, `show_text`, `python_source`, `frame_interval_ms`, and `text`.

Useful tools:

- `tools/ws2812_dev_cycle.py`
- `External/xiaozhi-esp32/GP_Port/gp_display_mcp_bridge.py`

Integration notes:

- `tools/ws2812_dev_cycle.py` now creates per-cycle logs under `debug_snapshots/dev_cycle_logs/`.
- LED-side serial verification is captured for a bounded duration instead of running an unbounded foreground monitor by default.

Output format:

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
