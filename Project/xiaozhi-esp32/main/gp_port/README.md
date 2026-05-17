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
3. 本地触摸 `Pattern` 切换和本地图案语音关键字应与 `LED端` 离线图案集合保持一致：`diamond / cross / checker / border / diagonal_x / jlu_emblem`；自由 `16x16` 绘图继续走 `matrix_pattern_request`
4. 本地触摸 `Effect` 循环复用 `LED端` 现有效果集合：`solid / pulse / gradient / scroll_left / scroll_right / fade_in / fade_out / color_cycle / row_reveal / row_hide / gradient_reveal`；`Color` 改为 `R/G/B` 三个 slider，实时更新主颜色并沿现有本地下发链立即生效
5. 本地触摸 `Local` 区和显式 `本地/离线` 语音短语可直接触发 `LED端` 本地动作：`next_pattern / show_text_scroll / show_clock / toggle_text_clock / next_effect / next_color`；这条链路绕过 `matrix_pattern_request` 与 `voice_color_analyze`
6. 共享协议继续复用 `SetAction(28 bytes)`；当 `content=state` 且 `animation_flags=local_action_id` 时，`LED端` 直接分发到本地方案动作，避免新增串口命令和长度变更
7. 上游矩阵结果可走专用 `debug websocket`，也可走主 websocket `type:"custom" + payload`
8. `Application` 先处理本地颜色调试和显式本地动作语音，再把矩阵 `custom.payload` 交给 `Board::HandleCustomPayload()`
9. `lichuang_dev_board.cc` 统一把 `payload.type` 或 `payload.action` 里的 `matrix_*` 结果规范化到现有解析器，完成预览、字模上传和蓝牙转发
10. `gp_led_matrix_esp32.cc` 生成共享协议包，`transport` 负责发包和后台接收 ACK/状态

## Runtime safeguards

- 语音触发 `matrix_pattern_request` 后，`Application` 会保留一个短暂 pending 窗口；若 TTS 先结束而矩阵结果仍在飞行中，设备会提示 `Matrix command pending` 并回到空闲态，而不是立即重新进入 `listening`。
- 主 websocket 与 debug websocket 可以并存；当前 websocket 客户端要求按连接实例维护分片接收状态，并在原连接内直接回复 `Ping/Pong`，避免共享静态组包状态或为每个 `Ping` 再派生线程。
- `AFE` 停止后会丢弃晚到的 `Feed()` 数据并清空旧 PCM 缓冲，避免模式切换时再次触发 `AFE(FEED) ringbuffer full`。
- 语音 uplink 链路当前保留一个较保守的实时缓冲窗口：`4` 帧 PCM encode 缓冲和约 `3 s` 的 Opus send 缓冲，并提高 `AFE` 内部任务与外层 `fetch` / detection 任务优先级；`connecting -> listening` 阶段的短时发送回堵应优先由这些缓冲吸收。
- 若发送回堵持续超过该缓冲窗口，系统会丢弃最旧的实时 PCM/Opus 上行帧并打印累计告警，而不是继续阻塞 `AFE fetch` 直到再次触发 `AFE(FEED)` ringbuffer full。
- `lichuang-dev` 的 debug preview HTTP server、debug websocket 和 debug command worker 已改为按需启动；Wi-Fi 建链后不会再默认常驻拉起这些调试资源，以减少 listening 切换期的内部 SRAM 压力。
- 当 `tools/list` 进入最低预算档位时，MCP 会先隐藏项目额外增加的 `self.screen.matrix_16x16.local.*` 和 `preview/snapshot` 调试工具，优先保住主控制面响应。
- `AfeAudioProcessor` 与 `AfeWakeWord` 的外层 ESP-SR 任务已改为 `PSRAM` 栈静态任务；若现场仍然出现 `AFE(FEED)` 连续打满，应先检查新的任务创建失败日志，而不是默认回退到协议层排查。

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
- 对于语音里的“字幕 / 跑马灯 / scroll”请求，`AI端` 应优先把它们路由到 `matrix_pattern_request`，再让主机根据传输需求选择 `show_scroll_subtitle` 或 `show_effect`；只有显式带 `本地/离线` 限定的字幕、时钟、图案、效果、颜色切换短语，才走 LED 端本地动作直发链。

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

## Key Functions Reference

| Function | File | Role | Called By |
|---|---|---|---|
| `GpLedMatrixEsp32::ShowAction()` | gp_led_matrix_esp32.cc | Send 28B action payload; dedup vs `last_action_` | debug UI / board |
| `GpLedMatrixEsp32::ShowRgb332Frame()` | gp_led_matrix_esp32.cc | Send 256B RGB332 frame via chunked transfer | board / application |
| `GpLedMatrixEsp32::ShowBitmapFrame()` | gp_led_matrix_esp32.cc | Route to `LayeredFrame` (≤4 layers) or chunked (>4) | board / HandleCustomPayload |
| `GpLedMatrixEsp32::ShowLayeredFrame()` | gp_led_matrix_esp32.cc | Serialize layers `[hdr:1][bmp:32][RGB:3]` per layer, send cmd 0x18 | board |
| `GpLedMatrixEsp32::ShowLayeredAnimation()` | gp_led_matrix_esp32.cc | Upload frames as `LayeredAnimFrame` (0x19), commit | board |
| `GpLedMatrixEsp32::SendLocalControlAction()` | gp_led_matrix_esp32.cc | Send local-only action (next_pattern, show_clock, etc.) | debug UI / voice |
| `GpLedMatrixEsp32::SyncClockTime()` | gp_led_matrix_esp32.cc | Forward Wi-Fi NTP time via `SetTime` cmd (6B) | application periodic |
| `GpLedMatrixEsp32::ReadReply()` | gp_led_matrix_esp32.cc | Poll reply queue: 12 retries × 8ms. Match by seq+cmd. | after each send |
| `GpLedMatrixEsp32::PollIncomingRequest()` | gp_led_matrix_esp32.cc | Handle LED-initiated requests (cache bitmap resend) | main loop |
| `GpLedMatrixEsp32::RunStartupLinkTest()` | gp_led_matrix_esp32.cc | Send R→G→B test sequence to verify BT link | board startup |
| `GpMatrixBtUartTransport::WritePacket()` | gp_led_matrix_transport.cc | TX over UART (mutex-protected) | GpLedMatrixEsp32 |
| `GpMatrixBtUartTransport::ReadPacket()` | gp_led_matrix_transport.cc | Dequeue validated packet from FreeRTOS RX queue | GpLedMatrixEsp32 |
| `gp_bt_uart_rx_task()` | gp_led_matrix_transport.cc | Background FreeRTOS task: UART byte→packet state machine | FreeRTOS scheduler |
| `GpDebugLcdDisplay::ProcessTouch()` | gp_debug_display.cc | Handle touch events: color sliders, presets, effects | LVGL event loop |
| `LichuangDevBoard::HandleCustomPayload()` | lichuang_dev_board.cc | Dispatch WS `type:"custom"` → `matrix_pattern_result` etc. | application.cc |
| `ConfigureBluetoothModule()` | lichuang_dev_board.cc | HC-05 AT config: name, PIN, role, baud, bind address | board startup |
