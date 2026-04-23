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
- HC-05 默认配置流：先在 `38400` 下发送 `AT` 探测；若连续 `3` 次无应答，则直接切本地串口到 `460800` 并跳过后续设置；若探测成功，再按“设置一条、查询一条”完成全部 AT 指令，AI端 使用固定地址 `98:D3:02:96:A2:B1` 对应的 `AT+BIND` 绑定 LED端，最后两步固定为 `AT+RESET` 和本地切到 `460800` 数据模式，名称统一为 `XiaoZhi -> WS2812`。
- LED端 UART2 默认使用 DMA 收发，并由任务/主循环批量消费接收数据，不再依赖逐字节串口中断路径。

优先查看路径：

- Prompt 索引：`.github/prompts/README.md`
- 问题说明：`Doc/项目文档/problem_tracking.md`
- AI端驱动：`External/xiaozhi-esp32/GP_Port/gp_led_matrix_esp32.h/.cc`
- AI端蓝牙传输：`External/xiaozhi-esp32/GP_Port/transport/`
- AI端调试界面：`External/xiaozhi-esp32/GP_Port/ui/`
- LED端协议执行：`STC51/Project/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- LED端头文件：`STC51/Project/ws2812_driver/Sources/inc/`
- 联调脚本：`tools/ws2812_dev_cycle.py`

执行要求：

1. 开始前先读取并应用 `.github/skills/karpathy-guidelines/SKILL.md`。
2. 只查看当前任务直接相关的文件，不扫描无关目录。
3. 中文说明统一使用 `AI端` 和 `LED端` 命名。
4. 只做最小必要改动，不保留无调用的兼容壳逻辑。
5. 若改动涉及动作对象、协议字段或回包逻辑，保持 AI端 与 LED端 定义一致。
6. 若改动涉及性能，说明优化点、影响路径和验证结果。
7. 修改源码后直接执行可用验证，不停留在建议层。
8. 统一维护 `solid / pattern / glyph` 三类 `SetAction` 行为，避免 AI端 能发但 LED端 不能执行的动作组合。
9. 将 DrawDrv 视为在线动画渲染主路径；直接帧写入仅用于显式帧传输或字模行传输。

默认验证：

- 改动 LED端 源码后，执行 `STC51/Project/ws2812_driver/ws2812_driver.uvproj` 的 Keil rebuild。
- 改动 AI端 或联调边界后，优先执行 `tools/ws2812_dev_cycle.py`。
- 若 `AI端` 构建成功且具备硬件连接条件，默认继续执行 flash；只有在用户明确要求只编译或当前环境无法访问硬件时才跳过。
- 改动 LED端 蓝牙链路时，优先追加 `-RunAi8051BtDebug`。

联调补充：

- `tools/ws2812_dev_cycle.py` 会把每轮联调日志保存到 `debug_snapshots/dev_cycle_logs/`。
- 评估蓝牙链路行为时，优先检查定时保存的 `LED端` 串口抓取日志。

输出格式：

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
