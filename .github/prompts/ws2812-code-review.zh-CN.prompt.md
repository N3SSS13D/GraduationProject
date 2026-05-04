---
name: WS2812 结构化代码审查（中文）
description: "按四类结构审查项目改动，聚焦 LED端、AI端、协议和脚本边界"
argument-hint: "审查目标（commit、文件、模块或功能描述）"
agent: agent
model: "GPT-5 (copilot)"
---
对指定目标执行代码审查。

## 文件结构定位

按以下四类结构检查改动边界，并只读取相关分类：

1. `LED端显示驱动`：`Project/STC51/`
2. `AI端接口调度`：`Project/xiaozhi-esp32/main/gp_port/`
3. `蓝牙通信协议`：`Project/Protocols/`
4. `本地绘图脚本`：`Project/Script/`

分类审查入口：

- `LED端`
  - `Project/STC51/ws2812_driver/Sources/app/app.c`
  - `Project/STC51/ws2812_driver/Sources/mid/gp_led_action.c`
  - `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
  - `Project/STC51/ws2812_driver/Sources/drv/ws2812_drv.c`
- `AI端`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
  - `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.cc`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
- `协议`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/Protocols/gp_led_matrix_protocol_spec.md`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
- `脚本`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
  - `Project/Script/tools/ws2812_auto_debug.py`

审查优先级：

1. 功能缺陷与行为回归
2. `LED端` 执行路径的时序风险
3. 蓝牙传输与协议一致性风险
4. 缓冲区越界与内存安全问题
5. 验证缺口

分类专项检查：

- `LED端`：确认 `app.c -> 协议轮询/调度 -> gp_led_action -> draw/ws2812` 主链路保持一致。
- `AI端`：确认 `board -> ui/debug state -> gp_led_matrix_esp32 -> transport` 仍是事件驱动且有边界。
- `协议`：确认共享常量、负载大小和命令文档没有漂移。
- `脚本`：确认主机绘图仍然走 `AI端` 预览/转发接口，而不是假定可以直接绕过到 `LED端` 原始发包。

若审查对象涉及 `Project/xiaozhi-esp32/main/gp_port/`，额外检查：

- `AI端` 动作对象与 `LED端` 协议字段是否一致
- 传输、ACK 和状态日志是否一致

输出格式：

- `Findings`
- `Open questions / assumptions`
- `Change summary`
- `Test and validation gaps`
