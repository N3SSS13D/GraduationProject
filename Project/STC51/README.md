# LED-side Display Driver

## Category

`LED端显示驱动`

## Active paths

- 主工程：`Project/STC51/ws2812_driver/`
- Keil 工程：`Project/STC51/ws2812_driver/ws2812_driver.uvproj`
- 应用入口：`Project/STC51/ws2812_driver/Sources/app/app.c`
- 驱动层：`Project/STC51/ws2812_driver/Sources/drv/`
- 头文件：`Project/STC51/ws2812_driver/Sources/inc/`
- 中间层：`Project/STC51/ws2812_driver/Sources/mid/`
- 示例工程：`Project/STC51/Examples/`

## Scope

本分类只负责 WS2812 矩阵显示驱动、动作执行、AI8051U 本地工程和 Keil 构建路径。

协议共享定义请查看：`Project/Protocols/`

## Prompt / Skill 入口

- Prompt：`.github/prompts/ws2812-led-driver*.prompt.md`
- Skill：`.github/skills/ws2812-led-driver/SKILL.md`

## Module quick map

- `Sources/app/app.c`
  - 初始化 WS2812 驱动、绘制驱动、协议接收路径和任务调度，并在主循环里执行轮询。
- `Sources/mid/mid_task.c`
  - 1 ms 协作式任务调度器。
- `Sources/mid/gp_led_action.c`
  - 远程动作、紧凑位图帧和动画缓冲的执行层；决定何时接管本地离线渲染。
- `Sources/mid/draw_drv.c`
  - 本地 `solid / glyph / pattern / animation` 渲染层。
- `Sources/mid/offline_pattern.c`
  - 本地离线图案像素查询与离线图案资源存放位置。
- `Sources/drv/gp_led_matrix_ai8051u.c`
  - `UART2` 收包、协议校验、命令分发与 ACK 回包。
- `Sources/drv/ws2812_drv.c`
  - 16x16 `WS2812` 底层扫描输出。

## Current execution flow

1. `main.c -> Test_Init() -> app.c` 初始化
2. 主循环执行 `GpLedMatrixAi8051u_Poll() -> MidTask_Process()`
3. 协议包进入 `gp_led_matrix_ai8051u.c`
4. 命令落到 `gp_led_action.c`
5. 远程帧走 `gp_led_action -> ws2812 / draw`，离线动画走 `mid_task -> draw_drv`

## Common read bundles

- `收包 / ACK / 指令执行`
  - `Project/Protocols/gp_led_matrix_protocol.h`
  - `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
  - `Project/STC51/ws2812_driver/Sources/mid/gp_led_action.c`
- `显示 / 渲染`
  - `Project/STC51/ws2812_driver/Sources/app/app.c`
  - `Project/STC51/ws2812_driver/Sources/mid/draw_drv.c`
  - `Project/STC51/ws2812_driver/Sources/mid/offline_pattern.c`
  - `Project/STC51/ws2812_driver/Sources/drv/ws2812_drv.c`
  - `Project/STC51/ws2812_driver/Sources/inc/gp_led_display_profile.h`
- `显示参数结构说明`
  - `Doc/Instructions/led_display_profile_structure.md`
- `性能 / 调度优化`
  - `Project/STC51/ws2812_driver/Sources/drv/ws2812_drv.c`
  - `Project/STC51/ws2812_driver/Sources/mid/draw_drv.c`
  - `Project/STC51/ws2812_driver/Sources/mid/mid_task.c`
  - `Project/STC51/ws2812_driver/Sources/app/app.c`
  - `Doc/Instructions/led_refresh_optimization.md`
- `工程 / 路径`
  - `Project/STC51/ws2812_driver/ws2812_driver.uvproj`

## Cross-category boundary

- 协议字段和常量只来自 `Project/Protocols/`
- 主机绘图和联调脚本只通过 `AI端` 或协议边界影响本分类，不直接替代本地驱动逻辑
