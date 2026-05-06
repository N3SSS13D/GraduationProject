# WS2812 LED端显示驱动

## 分类

`LED端显示驱动`

## 适用范围

`Project/STC51/` 下的 LED 端任务，特别是：

- `Project/STC51/ws2812_driver/Sources/app/`
- `Project/STC51/ws2812_driver/Sources/drv/`
- `Project/STC51/ws2812_driver/Sources/mid/`
- `Project/STC51/ws2812_driver/Sources/inc/`
- `Project/STC51/ws2812_driver/ws2812_driver.uvproj`

## 文件定位

- 扫描时序和渲染逻辑：`Sources/drv/`、`Sources/mid/` 及对应头文件
- 构建路径问题：`ws2812_driver.uvproj` 和相关 Keil 元数据文件
- 除非 LED 端接口边界需要，否则不读 `AI端接口调度` 或 `本地绘图脚本` 文件

## 模块快速图

- `Sources/app/app.c` — 运行时入口：初始化 WS2812、绘制驱动、协议接收路径和协作任务
- `Sources/mid/mid_task.c` — 1ms 协作调度器，供 app 任务图使用
- `Sources/mid/gp_led_action.c` — 本地绘制内容与远程控制动作/帧/动画执行之间的决策层
- `Sources/mid/draw_drv.c` — 本地/离线渲染：纯色、字形、图案和动画更新
- `Sources/drv/gp_led_matrix_ai8051u.c` — UART2 字节流组装、封包解析、命令分发、ACK/应答生成
- `Sources/drv/ws2812_drv.c` — 物理 16x16 WS2812 扫描驱动和底层显示输出

## 常见执行流程

- 启动路径：`main.c -> Test_Init() -> app.c init`
- 运行循环：`GpLedMatrixAi8051u_Poll() -> MidTask_Process()`
- 远程帧路径：`gp_led_matrix_ai8051u.c -> gp_led_action.c -> ws2812 / draw layers`
- 离线动画路径：远程帧模式未激活时 `mid_task tick -> draw_drv.c` 更新

## 常用阅读组合

- `封包 / ACK / 命令 bug` → `Project/Protocols/gp_led_matrix_protocol.h` + `Sources/drv/gp_led_matrix_ai8051u.c` + `Sources/mid/gp_led_action.c`
- `渲染 / 显示 bug` → `Sources/app/app.c` + `Sources/mid/draw_drv.c` + `Sources/drv/ws2812_drv.c`
- `构建 / 路径 bug` → `ws2812_driver.uvproj` + 匹配的 `.uvopt`/`.uvgui.*` 文件

## 优化工作流

对于 LED端刷新、动画或调度器优化工作：

1. 先总结当前实现、任务节奏和热路径
2. 陈述当前瓶颈、风险和候选优化，再开始编辑
3. 优先选择可独立验证的最小变更
4. 验证每个聚焦切片后再进行下一个优化
5. 验证后进行二次审查；若变更更新了时序或工作流预期，同步文档和 prompt/skill 指导

## 要求

1. 保持稳定的 WS2812 扫描/输出路径，除非任务明确要求更改
2. 硬件驱动变更保持在 LED端分类内
3. 若封包字段或共享常量更改，同步更新 `Project/Protocols/`
4. 若任务行为改变了预期的调试或验证流程，更新 `Doc/Instructions/` 或 `Project/STC51/` 下的文档
5. 源码修改后，重新构建 `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
