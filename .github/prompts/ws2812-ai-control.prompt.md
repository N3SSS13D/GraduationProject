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
2. Keep the AI-side output consistent with the LED-side protocol fields.
3. Keep action delivery bounded, traceable, and validation-friendly.
4. Touch MCP or snapshot tooling only when directly required by the task.
5. Run the available build or integration validation after changes.
6. For HC-05 tasks, assume all AT commands and queries finish at `38400` in strict set-then-query order, use `AT+BIND` with fixed slave address `98:D3:02:96:A2:B1` on the AI side, then make `AT+RESET` and the local switch to `115200` the final two steps, with reply logging visible in monitor.

Useful tools:

- `tools/ws2812_dev_cycle.py`
- `External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`

Output format:

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
