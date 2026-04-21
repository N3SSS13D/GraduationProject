---
name: WS2812 蓝牙主线开发（中文）
description: "围绕 AI端 与 LED端 的经典蓝牙主线，实现一个当前相关的增量功能"
argument-hint: "功能需求（例如：AI端动作映射、蓝牙收发、ACK 回包、性能优化）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个与当前主线直接相关的增量功能。

当前主线：

- `AI端`：`External/xiaozhi-esp32/`
- `LED端`：`STC51/Project/ws2812_driver/`
- 通信链路：`AI端动作对象 -> AI端蓝牙传输 -> HC-05 -> LED端 UART2 -> WS2812`

优先查看路径：

- Prompt 索引：`.github/prompts/README.md`
- 问题说明：`Doc/项目文档/problem_tracking.md`
- AI端驱动：`External/xiaozhi-esp32/GP_Port/gp_led_matrix_esp32.h/.cc`
- AI端蓝牙传输：`External/xiaozhi-esp32/GP_Port/transport/`
- AI端调试界面：`External/xiaozhi-esp32/GP_Port/ui/`
- LED端协议执行：`STC51/Project/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- LED端头文件：`STC51/Project/ws2812_driver/Sources/inc/`
- 联调脚本：`tools/ws2812_dev_cycle.ps1`

执行要求：

1. 开始前先读取并应用 `.github/skills/karpathy-guidelines/SKILL.md`。
2. 只查看当前任务直接相关的文件，不扫描无关目录。
3. 中文说明统一使用 `AI端` 和 `LED端` 命名。
4. 只做最小必要改动，不保留无调用的兼容壳逻辑。
5. 若改动涉及动作对象、协议字段或回包逻辑，保持 AI端 与 LED端 定义一致。
6. 若改动涉及性能，说明优化点、影响路径和验证结果。
7. 修改源码后直接执行可用验证，不停留在建议层。

默认验证：

- 改动 LED端 源码后，执行 `STC51/Project/ws2812_driver/ws2812_driver.uvproj` 的 Keil rebuild。
- 改动 AI端、MCP 或联调边界后，优先执行 `tools/ws2812_dev_cycle.ps1`。
- 改动 LED端 蓝牙链路时，优先追加 `-RunAi8051BtDebug`。

输出格式：

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
