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
  - 远程动作、紧凑位图帧和动画缓冲的执行层；决定何时接管本地离线渲染，并把缓冲播放统一规范化到 `32 x 36B` 单层位图帧。
- `Sources/mid/draw_drv.c`
  - 本地 `solid / glyph / pattern / animation` 渲染层。
- `Sources/mid/rtc_clock.c`
  - 本地三行时钟内容模块；内部保存 `3x5` 数码管风格数字，缓存 `AI端` 通过蓝牙同步的时间，并按本地节拍软递增；当前布局为“年 / 月.日 / 时:分 + 底行进度灯 + 右侧列动效”，其中日期点分隔常亮、时间冒号按秒闪烁。
- `Sources/mid/local_display_scheme.c`
  - 本地启动轮播方案、离线按键动作和默认离线显示编排；自动播放项已统一成“本地图像编号 + 效果命令描述符”。
- `Sources/mid/offline_pattern.c`
  - 本地离线图案像素查询与离线图案资源存放位置；当前离线图案统一使用 `36B` 单层位图资源。
- `Sources/drv/gp_led_matrix_ai8051u.c`
  - `UART2` 收包、协议校验、命令分发与 ACK 回包。
- `Sources/drv/ws2812_drv.c`
  - 16x16 `WS2812` 底层扫描输出。

## Current execution flow

1. `main.c -> APP_Init() -> app.c` 初始化
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
  - `Project/STC51/ws2812_driver/Sources/mid/rtc_clock.c`
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

## Local startup scheme and keys

- 启动后的本地默认显示方案由 `Sources/mid/local_display_scheme.c` 统一编排。
- 本地默认主题现统一使用黑色背景；渐变和颜色切换只改变点亮像素的前景色，不再引入彩色底色。
- 本地图像现在按本地编号统一管理：离线图案直接复用 `OFFLINE_PATTERN_IDX_*`，并额外预留一个“最近 AI 位图”图案槽，文字页再补充为本地文本图像编号，播放列表只引用图像编号和效果命令参数。
- 本地启动顺序：先以 `2 s` 间隔轮播离线图案纯色显示，最后一幅为蓝色 `JLU` 校徽；随后固定 `JLU` 校徽轮流播放本地效果，再进入同步时钟的本地效果演示，最终停留在“年 / 月日 / 时分”三分区时钟显示。
- 当前本地效果集合已扩展到 `row reveal`、`row hide` 和 `gradient reveal`；这三个效果当前只在 LED 端本地编排里可用，尚未加入共享协议枚举。
- 本地同步时钟内容当前以独立 `3x5` 数码管字模模块保存，并复用 `draw_drv.c` 的亮度、渐变、呼吸、淡入和逐行显示等本地效果；年、月.日、时:分、底行进度和右侧动效使用固定分区颜色区分，其中日期点分隔常亮、时间冒号按秒闪烁。
- 当前离线按键定义：
  - `P32` 短按切换图案；一旦 AI 端下发过单帧位图，该图也会作为新的本地图案槽参与轮换，切换到该槽时会主动向 AI 端请求重发最近位图
  - `P32` 长按在 `吉林大学` 文字滚动字幕和同步时钟之间切换
  - `P33` 短按切换效果
  - `P33` 长按切换颜色主题

## Current compact storage rule

- 本地图案资源统一使用 `32B bitmap + 3B RGB + 1B seq_total = 36B` 单层位图格式。
- LED 端缓冲动画统一使用 `32` 幅 `36B` 单层位图帧存储。
- 本地启动播放列表不再分散存成“图案数组 + 颜色数组 + 效果数组”；现在统一使用“本地图像编号 + 效果命令描述符”存储自动播放项。
- 效果命令描述符当前包含 `effectId / scrollStep / animStep / frameIntervalMs / gradientSpan / flags / timelineId`，便于新增本地效果时复用同一套存储格式。
- `BITMAP_RGB888` 与“黑底 + 前景”两层 `BITMAP_LAYERED` 输入在上传阶段会被规范化后再存入缓冲。
- 无法无损压缩为单层黑底帧的多层输入，保留直接渲染路径，但不进入紧凑动画缓冲。
