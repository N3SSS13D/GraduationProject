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
2. If the task involves debug-menu input or preset-based color/effect updates, reuse the existing `voice_color_analyze -> voice_color_result` analysis path and allow `source` to be either `stt` or `touch`.
3. For freeform `16x16` draw requests from `stt` that do not map cleanly to an existing local preset, route them through the existing `matrix_pattern_request` path so voice-only drawing does not depend on a prior debug-menu `Draw` tap.
4. Keep the AI-side output consistent with the LED-side protocol fields.
5. Keep action delivery bounded, traceable, and validation-friendly.
6. Touch MCP or snapshot tooling only when directly required by the task.
7. Run the available build or integration validation after changes. After an `AI端` build succeeds, continue with flash by default unless the user explicitly requests build-only or the hardware path is unavailable.
8. For HC-05 tasks, assume all AT commands and queries finish at `38400` in strict set-then-query order, use `AT+BIND` with fixed slave address `98:D3:02:96:A2:B1` on the AI side, then make `AT+RESET` and the local switch to `460800` the final two steps, with reply logging visible in monitor.
9. Preserve the existing touch-to-Bluetooth path for direct LED-side state updates, but do not route debug-menu touch actions through `SendColorDebugAnalyze(..., "touch")` or other MCP/LLM-only paths when a local debug transport is required. For touch-triggered preview/data exchange, prefer a board-local debug websocket client and keep the path bounded and monitor-visible.
10. Preserve the event-driven UART receive model in `GP_Port/transport/`; do not reintroduce per-call polling reads in `ReadPacket()`.
11. For `16x16` pattern preview tasks, prefer the host-side debug websocket path: the local Python script should run a dedicated websocket server for data transport, the `AI端` should connect as client and send touch-triggered requests, and the host should respond with compact preview payloads such as `bitmap_rows_hex + primary_rgb888`. Keep the Preview card layout fixed with the dot on the left and the preview slot on the right. Keep MCP focused on LLM-facing control/tool integration; use HTTP preview only as a secondary fallback for snapshots or generic PNG/JPEG preview debugging.
12. For LLM-facing MCP bridge scripts, use self-descriptive filenames, tool names, and argument names so the intent is obvious without reading the implementation. Prefer names like `gp_display_mcp_bridge.py`, `draw_python`, `show_text`, `python_source`, `eval_source`, `frame_interval_ms`, and `text`.
13. When the `AI端` needs to forward a `16x16` pattern to the `LED端` over Bluetooth, prefer compact `bitmap_rows + RGB888` transfer instead of expanding to a full `256`-byte RGB332 frame. Keep `FrameChunk` sizing consistent on both sides with the shared protocol constant `64` bytes.
14. For Bluetooth transport debugging, treat LED-side protocol logs such as `[GP_TX]`, `[GP_RX]`, `[GP_DROP]`, and `[GP_SYNC]` as authoritative. The raw `[BT_MON]` line is only a bounded UART sniff window and may clip long packets.
15. If a task needs to light the LED-side debug LEDs for transport verification, use the dedicated GP protocol debug-LED command instead of sending raw `LED n` text over HC-05.
16. If a task needs to verify sustained packet reception on the LED side, prefer an AI-side `1s` task that sends the GP debug-LED command only during link idle windows; make that background probe best-effort and avoid waiting for ACK so it does not interfere with foreground frame transfers.
17. When diagnosing why `SetAction` works but `FrameChunk` uploads fail, first inspect the LED-side UART receive cadence and packet assembly path before changing bitmap rendering logic.
18. After the LED side renders a remotely transferred frame, keep the last frame displayed until an explicit remote release or local control-mode switch occurs; do not clear it solely because the communication-active timeout expires.

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
