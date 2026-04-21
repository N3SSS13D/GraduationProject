---
name: AI端动作映射（中文）
description: "实现一个 AI端 功能，把语音或调试结果映射为可发送到 LED端 的动作对象"
argument-hint: "功能需求（例如：命令解析、动作映射、优先级、截图联动）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `AI端` 相关功能。

关注路径：

- `External/xiaozhi-esp32/GP_Port/gp_led_matrix_esp32.h/.cc`
- `External/xiaozhi-esp32/GP_Port/transport/`
- `External/xiaozhi-esp32/GP_Port/ui/`
- `External/xiaozhi-esp32/main/boards/lichuang-dev/`
- `External/xiaozhi-esp32/GP_Port/gp_led_matrix_protocol.h`

目标：

- 将 `AI端` 的语音结果、调试结果或截图控制请求映射为稳定动作对象。
- 保持 `AI端` 输出与 `LED端` 协议字段一致。
- 优先复用现有 `voice_color_result`、矩阵驱动和调试界面路径。

执行要求：

1. 只分析 `AI端` 相关目录和必要的 `LED端` 协议头。
2. 中文说明统一使用 `AI端` 和 `LED端` 命名。
3. 优先改动动作映射、协议拼包、调试工具接入，不重写无关 UI 或底层驱动。
4. 命令突发时保持动作下发有边界、可追踪、可验证。
5. 若涉及截图或 MCP，说明脚本路径和调用路径。
6. 修改后执行可用的构建或联调验证。

验证入口：

- `tools/ws2812_dev_cycle.ps1`
- `External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`

输出格式：

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
