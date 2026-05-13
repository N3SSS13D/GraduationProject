---
name: AI端动作映射（中文）
description: "实现一个 AI端 功能，把语音或调试结果映射为可发送到 LED端 的动作对象"
argument-hint: "功能需求（例如：命令解析、动作映射、优先级、截图联动）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `AI端` 相关功能。

## 文件结构定位

按以下四类结构定位相关文件，只读本次任务需要的最窄范围：

1. `AI端接口调度`
   - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.h/.cc`
   - `Project/xiaozhi-esp32/main/gp_port/transport/`
   - `Project/xiaozhi-esp32/main/gp_port/ui/`
   - `Project/xiaozhi-esp32/main/boards/lichuang-dev/`
2. `蓝牙通信协议`
   - `Project/Protocols/gp_led_matrix_protocol.h`
   - `Project/Protocols/gp_led_matrix_protocol_spec.md`
   - `Project/Protocols/gp_matrix_pattern_protocol.md`
3. `本地绘图脚本`
   - `Project/Script/mcp/gp_matrix/`
  - `Project/Script/tools/ws2812_auto_debug.py`
4. `LED端显示驱动`
   - 仅在接口边界必须联动时读取 `Project/STC51/ws2812_driver/`

## 模块速览

- `main/gp_port/gp_led_matrix_esp32.h/.cc`
  - `AI端` 矩阵编排核心，负责动作对象到协议包的转换、命令/帧发送以及 ACK/状态回读。
- `main/gp_port/transport/`
  - `HC-05 / UART` 传输后端，负责发包、后台收包和从字节流中组装完整协议包。
- `main/gp_port/ui/`
  - 本地触摸调试界面与预览缓冲。涉及预览卡片、触摸回调或本地可视化状态时优先看这一层。
- `main/boards/lichuang-dev/`
  - 板级接入层，负责把传输层、矩阵编排、调试界面、websocket 转发、MCP 接口和启动蓝牙检查接起来。
- `Project/Protocols/gp_led_matrix_protocol.h`
  - `AI端` 组包和字段对齐的唯一共享协议来源。

## 解决问题工作流

对于实现、联动或问题修复任务：

1. 改动前先总结当前实现切片，以及精确的控制/数据路径。
2. 先列出当前问题、风险和候选方案，再开始编辑。
3. 先选定最小可行改动，并提前定义验证标准。
4. 一次只实现一个聚焦切片，不要立刻扩大范围。
5. 每个切片验证后先回看受影响链路，再进入下一步。
6. 若改动影响流程、假设或行为预期，同步更新文档、Prompt 和 Skill。

## 目标

- 将 `AI端` 的语音结果、调试结果或截图控制请求映射为稳定动作对象
- 保持 `AI端` 输出与 `LED端` 协议字段一致
- 优先复用现有 `voice_color_result`、矩阵驱动和调试界面路径
- 若任务涉及调试菜单输入或预设型颜色/效果更新，优先复用
  `voice_color_analyze -> voice_color_result` 这条统一分析链，允许 `source` 为 `stt` 或 `touch`
- 若任务涉及本地触摸切换或本地图案语音直达，保持 `AI端` 预设名与 `LED端` 离线图案集合对齐：
  `diamond / cross / checker / border / diagonal_x / jlu_emblem`
- 若任务涉及本地触摸调试页，保持 `Effect` 循环与 `LED端` 本地手动效果集合对齐，并保留 `R/G/B`
  三个 slider 对主颜色的直接调节：
  `solid / pulse / gradient / scroll_left / scroll_right / fade_in / fade_out / color_cycle / row_reveal / row_hide / gradient_reveal`
- 保留 `LED端` 本地/离线动作的显式入口：`next_pattern / show_text_scroll / show_clock / toggle_text_clock /
  next_effect / next_color`。这类显式本地动作在 `AI端` 触摸或本地语音里应直接走本地矩阵传输链，不要重新进入
  `voice_color_analyze`、`voice_color_result` 或 `matrix_pattern_request`
- 若 `stt` 文本表达的是自由 `16x16` 绘图，且不能直接落到本地现有 `solid / pattern / glyph` 预设上，
  优先复用已有 `matrix_pattern_request` 路径，避免语音绘图依赖先点一次 `Draw`
- 若任务涉及 HC-05，默认配置流程为先在 `38400` 下发送 `AT` 探测；若连续 `3` 次无应答，则直接切本地串口到
  `460800` 并跳过后续设置；若探测成功，再按“设置一条、查询一条”完成全部 AT 指令，`AI端` 使用固定地址
  `98:D3:02:96:A2:B1` 对应的 `AT+BIND` 绑定 `LED端`，最后两步固定为 `AT+RESET` 和本地切到 `460800`
  数据模式，并把每一步回包打印到 monitor

## 执行要求

1. 只分析 `AI端接口调度` 相关目录和必要的 `蓝牙通信协议` 文件；不要默认扫描无关分类
2. 中文说明统一使用 `AI端` 和 `LED端` 命名
3. 优先改动动作映射、协议拼包、调试工具接入，不重写无关 UI 或底层驱动
4. 命令突发时保持动作下发有边界、可追踪、可验证
5. 若涉及截图或 MCP，说明脚本路径和调用路径
6. 修改后执行可用的构建或联调验证。若 `AI端` 构建成功且具备硬件连接条件，默认继续执行 flash；只有在用户明确要求只编译或当前环境无法访问硬件时才跳过
7. 保持现有触摸控制主链：`GpDebugLcdDisplay -> QueueMatrixDebugState -> ShowDebugState -> SetAction`；若任务是显式本地/离线动作按钮，则允许使用
  `GpDebugLcdDisplay -> QueueLocalControlAction -> SendLocalControlAction -> SetAction` 这条并行但受控的本地链路。若需要进入大模型，优先复用现有路径，不要为同一类触摸输入新增无边界协议
8. 保持 `main/gp_port/transport/` 中基于后台任务的 UART 收包模型，不要把 `ReadPacket()` 改回调用时轮询读串口
9. 若任务涉及 Wi-Fi 图片预览链路，先从 `AI端` monitor 日志 `WiFi STA IP: ...` 获取设备地址，再优先使用主机脚本
   `/control/device_preview` 将本地 PNG/JPEG 发送到 `http://<device_ip>:8781/debug/preview_image`；设备侧应复用现有预览路径，并同步显示到调试二级菜单预览卡片中
10. 面向 LLM 的 MCP 桥接脚本、工具名和参数名必须尽量自解释，优先使用一眼可懂的命名，例如
  `gp_display_mcp_bridge.py`、`draw_python`、`show_text`、`show_scroll_subtitle`、`python_source`、`eval_source`、
    `frame_interval_ms`、`text`
11. 若任务涉及 `16x16` 图案预览，优先维持当前固定布局：左侧中心为圆点，右侧中心为预览区域；不要再让预览区域依赖首次绘制后才出现
12. 当 `AI端` 需要把 `16x16` 图案通过蓝牙转发到 `LED端` 时，优先使用紧凑 `bitmap_rows + RGB888` 传输，而不是先展开成
    `256` 字节 RGB332 整帧；`FrameChunk` 的分片基准必须在两端保持一致，统一使用共享协议里的 `64` 字节常量
13. 若任务涉及 `16x16` 动画，优先提供输出 `matrix_frame_sequence_v1` 的独立 MCP 工具，并让每帧都携带适合 `LED端` 的紧凑
    `bitmap_rows_hex`。LLM 侧优先用 `frames[]` 作为输入壳（每帧在 `bitmap_rows_hex`、`bitmap_rows`、
    `python_source/eval_source` 中三选一），或使用每项为完整帧的 `bitmap_rows_hex_list`。其中
    `bitmap_rows_hex` 只表示 `32 byte` 位图本体，规范写法是恰好 `64` 个十六进制字符、`16` 行、每行 `16 bit`、
    按 `top->bottom` 排列、高位对应最左侧 LED、`1=亮`、`0=暗`，桥接层也可兼容常见的 `16` 行逐行 `hex token`
    写法并统一归一化。再加上前景/背景 `RGB888` 各 `3 byte`，整帧紧凑格式固定为 `38 byte`。保持 `LED端`
    `24` 帧缓冲上限，将 `frame_interval_ms` 视为 `1..65535 ms` 的毫秒级字段，默认取 `42 ms`；若主机侧收到超过
    `24` 帧的输入，优先重采样到 `24` 帧并同步调整间隔以尽量保持总时长。缓冲动画继续通过
    `matrix_animation_start -> 带序号的 matrix_pattern_result -> matrix_animation_end`
    让 `AI端` 先做本地缓冲预览，再整体转发到 `LED端`。禁止在绘图语句里用 `yield_frame`、`time.sleep`、`import`
    等方式自行实现时序
14. 对于能落入单个 `64` 字节 `FrameChunk` 的紧凑位图帧，优先只在 `FrameCommit` 等待 ACK；除非任务明确要求最大化传输诊断，否则不要再对 `FrameStart` 与 `FrameChunk` 逐阶段等待 ACK
15. 蓝牙联调时，以 `LED端` 的协议级 `[GP_TX]`、`[GP_RX]`、`[GP_DROP]`、`[GP_SYNC]` 日志为准；`[BT_MON]` 只是一个有上限的原始 UART 抓包窗口，可能裁剪长包，不能单独据此判断整帧是否完整到达
16. 需要确认 `LED端` ACK 是否真实返回到 `AI端` 时，优先查看 `AI端` monitor 中新增的 `[BT_RX]` 日志；它会打印通过 HC-05 收到的完整协议回包摘要和原始十六进制内容
17. 如果任务需要点亮 `LED端` 板载调试 LED 做链路验证，必须使用独立的 GP 协议调试 LED 命令，不要再向 HC-05 发送裸 `LED n` 文本
18. 如果任务需要验证 `LED端` 是否能持续稳定接收协议包，而不仅是回一条短 ACK，优先让 `AI端` 创建 `1s` 周期任务，但只在链路空闲窗口内发送 GP 协议调试 LED 命令；该后台探测应采用尽力发送且不等待 ACK，避免干扰前台整帧传输
19. 当 `SetAction` 这类短包能工作、而 `FrameChunk` 这类长包上传失败时，应先检查 `LED端` UART2 接收节奏与组包时机，再考虑修改位图渲染逻辑
20. `LED端` 显示通过蓝牙传输的图像后，应保持最后一帧显示，直到显式释放远程模式或本地切换控制模式；不能仅因为通信活动超时就自动清空
21. 若任务涉及 `lichuang-dev` 的主机侧调试 `websocket / snapshot` 地址切换，默认优先复用 `AI端` 串口命令
  `mcp_host set <ip_or_host>` 一次性改写 `debug_ws` 与 `snap_url`；只有在端口或路径也要变化时，才分别使用
  `debug_ws set <url>` 与 `snap_url set <url>`
22. 当 `stt` 文本明确描述“字幕 / 跑马灯 / scroll”时，优先把它路由到 `matrix_pattern_request`，不要先被本地颜色预设消化。
  同时在请求 payload 里把 `show_scroll_subtitle` 作为“字幕走图像序列传输”时的首选主机工具，并与现有的
  `show_effect`、`draw_animation` 提示一起暴露给上游。
23. 当矩阵结果经主会话 websocket 以 `type:"custom"` 返回时，优先把 `custom.payload` 委托给板级钩子处理，
    并把 `payload.type` 或 `payload.action` 中的 `matrix_pattern_result`、`matrix_action_result`、
    `matrix_animation_*` 统一归一化到现有板级解析器；不要在 `Application` 内再复制一套矩阵结果解析逻辑。
24. 若语音触发的矩阵请求在 TTS 结束时仍未收到结果，交互层应进入一个短暂的 pending/idle 状态并给出等待提示，
    而不是立刻回到 `listening`，避免把“结果仍在路上”误判成“命令没有执行”。
25. 当主 websocket 与 debug websocket 可能同时存在时，websocket 分片接收状态必须按连接实例保存，`Ping`
    直接在原连接回复；不要再为每个 `Ping` 派生 detached 线程，也不要复用共享静态组包状态。
26. `lichuang-dev` 上的 debug preview HTTP server、debug websocket 和 debug command worker 当前都视为按需调试设施。
  不要再次在 Wi-Fi 连上后默认常驻启动，除非任务明确要求常驻调试链路，且能说明新增的内部 SRAM 成本是可接受的。
27. 当前稳定语音链路默认维持一个较保守的实时 backlog 窗口，而不是继续无限扩队列。保留“丢弃最旧实时帧”的
  语义；如果 `AFE(FEED)` 仍然溢出，先检查 ESP-SR 外层 `fetch/detection` 任务是否成功创建，再决定是否继续放大队列。
28. 在本板型低内存场景下，`tools/list` 应优先隐藏项目额外增加的 `matrix local` 与 `preview/snapshot` 调试工具，
  不要先牺牲基线公共工具；若 AFE 外层任务需要更大栈，优先使用带显式失败日志的 `PSRAM` 静态任务栈。

## 常用阅读集

- `发包 / ACK / 组包问题`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
  - `Project/xiaozhi-esp32/main/gp_port/transport/gp_led_matrix_transport.cc`
- `触摸界面 / 预览问题`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.h`
  - `Project/xiaozhi-esp32/main/gp_port/ui/gp_debug_display.cc`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
- `主机绘图 / websocket / MCP 转发`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
  - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`

## 联调工具说明

- 默认联调链路入口为 `Project/Script/tools/ws2812_auto_debug.py`：先 Keil 编译 `LED端`，等待 `20s` 打开
  `AI8051U` 串口监视，再执行 ESP-IDF `build flash monitor`。
- Keil 编译日志默认写入 `Project/Debug/build/keil_build.log`，用于快速定位编译失败原因。

## 输出格式

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
