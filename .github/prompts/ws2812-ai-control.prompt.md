---
name: WS2812 AI Control Bridge
description: "Implement one AI-control integration feature that maps XiaoZhi commands to display actions"
argument-hint: "AI feature (e.g., command parser, action mapping, priority policy, fail-safe behavior)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one AI-control feature based on the argument.

Goal:
- Build a clean interface layer between XiaoZhi AI inputs and display actions.
- Keep display timing independent from unpredictable AI message timing.

Scope (AI interface layer):
- input command format and parser
- command-to-action mapping
- validation and fallback behavior
- queue/policy for command arbitration with local effects

Current structure to use:
- App: `Sources/app/` for business decisions from ASR/AI results
- Mid: `Sources/mid/` for action normalization, arbitration, and reusable state logic
- Drv: `Sources/drv/` for display-driver invocation and timing-safe execution hooks
- Shared: `Sources/inc/` for common action definitions
- Peripheral glue: `Sources/*.c` and `Sources/lib/` for low-level access only

Do not do in this prompt:
- Large UI/host-side application work
- Rewriting scan driver timing internals
- Multi-feature AI platform design in one step

Execution requirements:
1. Inspect `Sources/app`, `Sources/mid`, `Sources/drv`, and `External/xiaozhi-esp32/GP_Port/` first.
2. Define a stable API between AI and display modules, preferably aligned with `voice_color_result` and `gp_led_matrix_protocol.h`.
3. Add strict bounds checks and invalid-command handling.
4. Keep deterministic display refresh regardless of command burst.
5. Document command examples and integration points.
6. Keep protocol parsing and arbitration in `mid/`, and keep App focused on decision flow.

Current phase emphasis:
- The next target is not a generic AI platform layer. It is a narrow bridge from XiaoZhi voice output to AI8051U-executable LED actions over the current local transport, which is I2C on the stable branch and the restored UART2 HC-05 path on `BT_Version`.
- Prioritize the local communication loop without requiring external MCP bridging. Add MCP-facing expansion only after the local path is stable.
- Reuse the existing `GP_Port` protocol and interface assets before inventing new message shapes.
- When the existing `voice_color_result` or local voice debug-dot path changes color/effect, mirror that same state to the LED matrix instead of creating a separate test-only flow.
- Keep the active AI8051U transport electrical assumptions explicit: the stable branch uses I2C on `P2.4/P2.3`, while the current `BT_Version` path uses UART2 on `P4.2/P4.3` at `9600 8N1` with HC-05 text forwarding and Timer2 reserved only for UART2 baud generation.
- Treat former demo animations as callable matrix presets such as `diamond`, `cross`, `JLU_emblem`, and `scroll_subtitle`, and let voice commands select them by name.
- When no preset name is requested, prefer a pure solid-color matrix display rather than expressing the state only through background-color changes.
- Once a preset is selected, keep rendering only that preset instead of rotating through multiple legacy demo patterns.
- Change pattern background color only through an explicit background-color command or field; solid-color display commands must not rewrite the stored background.
- Keep the XiaoZhi online LED brightness aligned with the AI8051U offline default unless the user explicitly changes brightness.
- Expose device-side UI debugging helpers through MCP, including snapshot control tools that start a device-side capture task and report execution status while the actual image payload is uploaded through the local HTTP receiver.
- Keep the local Python MCP helper script runnable without arguments by preconfiguring the endpoint and saving captured screenshots into the project folder by default.
- When the device Snap button is used, freeze that exact frame into an isolated LVGL snapshot buffer, release the LVGL-owned buffer quickly, encode PNG in the background, and upload it to a local HTTP snapshot receiver on the developer machine instead of relying on reverse MCP tools/call over the official voice-model bridge.
- Prefer serial logs for snapshot progress and failure reporting instead of consuming scarce debug-menu screen space with persistent status text.
- In addition to receiving device-side HTTP snapshot uploads, the local Python helper should expose a local HTTP control endpoint that lets the host request `self.screen.debug_snapshot.capture` without going through serial text commands.
- Keep the local Python MCP helper responsibilities explicit: it may relay a device MCP tool call only when the local host hits the HTTP control endpoint, but it should not keep the old serial-trigger path or expose local screenshot Base64 sink tools.
- After each XiaoZhi-side or MCP-script modification, run the mandatory debug flow in this order:
	1. XiaoZhi side: build, flash, and monitor the XiaoZhi firmware.
	2. Stop the old local MCP Python script.
	3. Restart the local MCP Python script and confirm that both the MCP bridge connection and the local HTTP snapshot receiver are ready.
	4. Use the local HTTP control endpoint and the on-device Snap button to validate both the host-triggered and device-triggered screenshot paths.
	5. Verify the result from the local status endpoint and the device serial logs.
	6. LED driver side: stop the serial monitor, build with Keil without manual download, wait 20 seconds, then reopen the serial port and inspect debug logs.
	7. Continue iterating based on the observed debug information until the target behavior is reached.
- Recommended automation entry for the above flow: `tools/ws2812_dev_cycle.ps1`. Use `-ValidateSnapshotControl` when you need the script to also trigger `/control/snapshot`.

Output format:
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
