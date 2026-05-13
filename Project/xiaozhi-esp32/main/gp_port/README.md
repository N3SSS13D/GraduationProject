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
- `Project/xiaozhi-esp32/main/application.cc`
  - 主状态机与主 websocket 路由，负责 `stt` 语义识别、`matrix_pattern_request` 生成，以及把主通道 `custom.payload` 里的矩阵结果优先交给板级消费。
- `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
  - 板级接入层，把 UI、传输、矩阵控制、`matrix_pattern_result` / `matrix_action_result` 回包复用、预览缓存和启动检查串起来。

## Current execution flow

1. 板级初始化创建 `transport + GpLedMatrixEsp32`
2. `Application` 根据 `stt` / `touch` 生成 `matrix_pattern_request`，字幕/跑马灯类请求会额外提示上游优先使用 `show_scroll_subtitle`
3. 上游矩阵结果可走专用 `debug websocket`，也可走主 websocket `type:"custom" + payload`
4. `Application` 先处理本地颜色调试，再把矩阵 `custom.payload` 交给 `Board::HandleCustomPayload()`
5. `lichuang_dev_board.cc` 统一把 `payload.type` 或 `payload.action` 里的 `matrix_*` 结果规范化到现有解析器，完成预览、字模上传和蓝牙转发
6. `gp_led_matrix_esp32.cc` 生成共享协议包，`transport` 负责发包和后台接收 ACK/状态

## Runtime safeguards

- 语音触发 `matrix_pattern_request` 后，`Application` 会保留一个短暂 pending 窗口；若 TTS 先结束而矩阵结果仍在飞行中，设备会提示 `Matrix command pending` 并回到空闲态，而不是立即重新进入 `listening`。
- 主 websocket 与 debug websocket 可以并存；当前 websocket 客户端要求按连接实例维护分片接收状态，并在原连接内直接回复 `Ping/Pong`，避免共享静态组包状态或为每个 `Ping` 再派生线程。
- `AFE` 停止后会丢弃晚到的 `Feed()` 数据并清空旧 PCM 缓冲，避免模式切换时再次触发 `AFE(FEED) ringbuffer full`。

## Runtime endpoint override

`lichuang-dev` 板级已支持在 `AI端` 串口监视器里直接改主机侧 MCP 服务地址，避免 PC 端 Wi-Fi IP 变化后还要重新改代码：

- `mcp_host set <ip_or_host>`
  - 同时重写 `debug_ws` 为 `ws://<host>:8766/debug`
  - 同时重写 `snap_url` 为 `http://<host>:8765/snapshot`
- `mcp_host get` / `mcp_host status`
  - 打印当前 `debug_ws` 与 `snap_url` 生效状态
- `mcp_host reset`
  - 恢复编译期默认地址
- 若端口或路径也要改，继续使用 `debug_ws set <url>` 与 `snap_url set <url>` 覆盖完整 URL

## MCP tool namespace boundary

- `AI端` 本地调试工具统一使用 `self.screen.matrix_16x16.local.*`，例如 `local.draw`、`local.draw_frame`、`local.pattern`。
- 主机脚本桥接工具继续使用 `self.screen.matrix_16x16.*`，例如 `draw_python`、`show_text`、`show_scroll_subtitle`、`show_effect`、`draw_animation`。
- 对于语音里的“字幕 / 跑马灯 / scroll”请求，`AI端` 应优先把它们路由到 `matrix_pattern_request`，再让主机根据传输需求选择 `show_scroll_subtitle` 或 `show_effect`。

## Common read bundles

- `发包 / ACK / 协议一致性`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
  - `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc`
- `主 websocket / 语音矩阵请求`
  - `Project/xiaozhi-esp32/main/application.cc`
  - `Project/xiaozhi-esp32/main/boards/common/board.h`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
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
