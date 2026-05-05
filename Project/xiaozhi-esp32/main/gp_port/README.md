# AI-side Interface Orchestration

## Category

`AI端接口调度`

## Active paths

- AI端矩阵扩展：`Project/xiaozhi-esp32/main/gp_port/`
- 传输层：`Project/xiaozhi-esp32/main/gp_port/transport/`
- 调试界面：`Project/xiaozhi-esp32/main/gp_port/ui/`
- 板级接入：`Project/xiaozhi-esp32/main/boards/lichuang-dev/`
- 共享协议头：`Project/Protocols/gp_led_matrix_protocol.h`

## Scope

本分类只负责语音结果映射、蓝牙传输接入、调试界面、预览路径和板级初始化。

协议规范请查看：`Project/Protocols/`
本地绘图工具请查看：`Project/Script/`

## Prompt / Skill 入口

- Prompt：`.github/prompts/ws2812-ai-control*.prompt.md`
- Skill：`.github/skills/karpathy-guidelines/SKILL.md`

## Module quick map

- `gp_led_matrix_esp32.h/.cc`
  - 矩阵编排核心，负责动作对象、单帧、动画批次到协议包的转换，以及 ACK/状态处理。
- `transport/gp_led_matrix_transport.cc`
  - `HC-05 / UART` 传输层，负责发包、后台收包和完整包提取。
- `ui/gp_debug_display.h/.cc`
  - 本地调试界面、触摸输入和预览缓冲。
- `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
  - 板级接入层，把 UI、传输、矩阵控制、websocket/MCP 转发和启动检查串起来。

## Current execution flow

1. 板级初始化创建 `transport + GpLedMatrixEsp32`
2. `UI` 或主机绘图结果进入板级回调
3. `gp_led_matrix_esp32.cc` 生成共享协议包
4. `transport` 负责发包和后台接收 ACK/状态
5. 主机绘图结果先本地预览，再按协议转发到 `LED端`

## Common read bundles

- `发包 / ACK / 协议一致性`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
  - `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc`
- `触摸 / 预览 / 调试界面`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.h`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.cc`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
- `主机绘图转发`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`

## Cross-category boundary

- 协议格式由 `Project/Protocols/` 定义，不在本目录内复制一份
- 主机绘图脚本应对接本分类的预览/转发入口，而不是直接假定 `LED端` 发送细节
