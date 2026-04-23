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
- 若任务涉及调试菜单输入，优先复用 `voice_color_analyze -> voice_color_result` 这条统一分析链，允许 `source` 为 `stt` 或 `touch`。
- 若任务涉及 HC-05，默认配置流程为先在 `38400` 下发送 `AT` 探测；若连续 `3` 次无应答，则直接切本地串口到 `460800` 并跳过后续设置；若探测成功，再按“设置一条、查询一条”完成全部 AT 指令，AI端 使用固定地址 `98:D3:02:96:A2:B1` 对应的 `AT+BIND` 绑定 LED端，最后两步固定为 `AT+RESET` 和本地切到 `460800` 数据模式，并把每一步回包打印到 monitor。

执行要求：

1. 只分析 `AI端` 相关目录和必要的 `LED端` 协议头。
2. 中文说明统一使用 `AI端` 和 `LED端` 命名。
3. 优先改动动作映射、协议拼包、调试工具接入，不重写无关 UI 或底层驱动。
4. 命令突发时保持动作下发有边界、可追踪、可验证。
5. 若涉及截图或 MCP，说明脚本路径和调用路径。
6. 修改后执行可用的构建或联调验证。
7. 保持现有触摸控制主链：`GpDebugLcdDisplay -> QueueMatrixDebugState -> ShowDebugState -> SetAction`；若需要进入大模型，复用 `GpDebugLcdDisplay -> SendColorDebugAnalyze(..., "touch")`，不要新增并行协议。
8. 保持 `GP_Port/transport/` 中基于后台任务的 UART 收包模型，不要把 `ReadPacket()` 改回调用时轮询读串口。
9. 若任务涉及 Wi-Fi 图片预览链路，先从 `AI端` monitor 日志 `WiFi STA IP: ...` 获取设备地址，再优先使用主机脚本 `/control/device_preview` 将本地 PNG/JPEG 发送到 `http://<device_ip>:8781/debug/preview_image`；设备侧应复用现有预览路径，并同步显示到调试二级菜单预览卡片中。
10. 面向 LLM 的 MCP 桥接脚本、工具名和参数名必须尽量自解释，优先使用一眼可懂的命名，例如 `gp_display_mcp_bridge.py`、`draw_python`、`show_text`、`python_source`、`frame_interval_ms`、`text`。
11. 当 `AI端` 需要把 `16x16` 图案通过蓝牙转发到 `LED端` 时，优先使用紧凑 `bitmap_rows + RGB888` 传输，而不是先展开成 `256` 字节 RGB332 整帧；`FrameChunk` 的分片基准必须在两端保持一致，统一使用共享协议里的 `64` 字节常量。
12. 蓝牙联调时，以 `LED端` 的协议级 `[GP_TX]`、`[GP_RX]`、`[GP_DROP]`、`[GP_SYNC]` 日志为准；`[BT_MON]` 只是一个有上限的原始 UART 抓包窗口，可能裁剪长包，不能单独据此判断整帧是否完整到达。
13. 需要确认 `LED端` ACK 是否真实返回到 `AI端` 时，优先查看 `AI端` monitor 中新增的 `[BT_RX]` 日志；它会打印通过 HC-05 收到的完整协议回包摘要和原始十六进制内容。
14. 如果任务需要点亮 `LED端` 板载调试 LED 做链路验证，必须使用独立的 GP 协议调试 LED 命令，不要再向 HC-05 发送裸 `LED n` 文本。
15. 如果任务需要验证 `LED端` 是否能持续稳定接收协议包，而不仅是回一条短 ACK，优先让 `AI端` 创建 `1s` 周期任务，持续发送 GP 协议调试 LED 命令，让 `LED端` P2 调试 LED 按正常 ACK 流程做流水灯。
16. 当 `SetAction` 这类短包能工作、而 `FrameChunk` 这类长包上传失败时，应先检查 `LED端` UART2 接收节奏与组包时机，再考虑修改位图渲染逻辑。
17. `LED端` 显示通过蓝牙传输的图像后，应保持最后一帧显示，直到显式释放远程模式或本地切换控制模式；不能仅因为通信活动超时就自动清空。

验证入口：

- `tools/ws2812_dev_cycle.py`
- `External/xiaozhi-esp32/GP_Port/gp_display_mcp_bridge.py`

联调补充：

- `tools/ws2812_dev_cycle.py` 会把每轮联调日志落到 `debug_snapshots/dev_cycle_logs/`。
- 默认用定时串口抓取保存 `LED端` 日志，而不是无限前台串口监听。

输出格式：

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
