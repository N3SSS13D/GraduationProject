# GraduationProject

## 项目概述

本仓库用于毕业设计开发，当前聚焦两条技术主线：

1. `STC51/Project/ws2812_driver/`
   - 基于 STC AI8051U 的 WS2812 复用扫描显示系统。
   - 当前已形成稳定的 PWM + DMA、74HC595 行选、PMOS 高侧切换和 USB 调试链路。
2. `External/xiaozhi-esp32/`
   - 引入的小智 AI 参考快照。
   - 当前已补充 `GP_Port/` 目录，用于承载 ESP32 <-> AI8051U 的 I2C 协议、驱动骨架、联调脚本和阶段 prompt。

下一阶段的核心目标是打通：

`小智 AI 语音输入 -> ESP32 动作映射 -> I2C 自定义协议 -> AI8051U 接收执行 -> WS2812 LED 显示`

## 仓库结构

```text
GraduationProject/
|-- README.md
|-- Doc/
|   `-- 项目文档/
|       |-- project_status_summary_2026-04-12.md
|       |-- usb_play_v2_guide.md
|       |-- ws2812_driver_current_implementation.md
|       `-- xiaozhi_esp32_porting_summary.md
|-- External/
|   `-- xiaozhi-esp32/
|       |-- main/                      # 小智应用、板级与设备抽象
|       `-- GP_Port/                   # 本项目扩展的协议、驱动骨架、联调资产
|-- STC51/
|   `-- Project/
|       `-- ws2812_driver/
|           |-- Sources/
|           |   |-- app/               # 扫描调度与应用层流程
|           |   |-- mid/               # 渲染、动画、按键控制
|           |   |-- drv/               # WS2812/74HC595 驱动
|           |   |-- inc/               # 共享头文件与配置
|           |   |-- timer.c            # 定时器与节拍控制
|           |   |-- usblib.c           # USB 命令入口
|           |   `-- main.c             # MCU 入口与初始化
|           `-- ws2812_driver.uvproj
`-- .github/
    `-- prompts/                       # 项目开发 prompt 集合
```

## 已实现能力

### STC51 / WS2812 侧

- 已完成 PWM + DMA 双通道输出链路整理。
- 已完成 74HC595 + PMOS 行扫描控制与复位尾波处理。
- 已完成 Timer0 one-shot 扫描节拍调度，减少关键路径中的软延时。
- 已支持 `normal_pair` 与 `legacy_shift` 两类扫描/发送模式。
- 已支持 USB 调试命令：颜色、图案、间隔、渲染模式切换。
- 已支持 16x64 / 16x8 有效列模式，以及对应的运行时 DMA 长度重建。

### XiaoZhi / GP_Port 侧

- 已引入 `xiaozhi-esp32` 参考快照用于语音交互与 MCP 架构参考。
- 已补充共享协议头 `gp_led_matrix_protocol.h`。
- 已补充 ESP32 侧矩阵驱动骨架 `gp_led_matrix_esp32.h/.cc`。
- 已补充 AI8051U 接口设计头 `gp_led_matrix_ai8051u.h`。
- 已补充 MCP 调试工具、调试圆点显示链路和官方 MCP 桥接测试脚本。

## 当前文档入口

- 当前项目总览与阶段计划：`Doc/项目文档/project_status_summary_2026-04-12.md`
- WS2812 驱动实现说明：`Doc/项目文档/ws2812_driver_current_implementation.md`
- 小智移植与联调总结：`Doc/项目文档/xiaozhi_esp32_porting_summary.md`
- Prompt 索引：`.github/prompts/README.md`

## 下一阶段计划

1. 固化语音动作对象
   - 对齐颜色、亮度、动画、文本等高层参数，统一为 ESP32 到 AI8051U 可复用的动作模型。
2. 冻结 I2C 自定义协议
   - 完善命令字、分包、校验、ACK、错误码、状态回读和心跳机制。
3. 打通 ESP32 主机发送路径
   - 在 `GP_Port/gp_led_matrix_esp32` 中接入真实 I2C 发送与状态映射。
4. 落地 AI8051U 从机执行路径
   - 在 STC51 工程中实现收包、解析、缓存和 WS2812 动作执行。
5. 完成语音控灯联调验证
   - 验证语音输入、协议传输、异常恢复和显示刷新稳定性。

## 构建与验证

### STC51 工程

1. 使用 Keil 打开 `STC51/Project/ws2812_driver/ws2812_driver.uvproj`。
2. 编译并通过 STC ISP 下载固件。
3. 使用串口或 USB 命令验证颜色、间隔、图案和渲染模式切换。

### ESP32 参考工程

1. 使用 ESP-IDF 插件打开 `External/xiaozhi-esp32/`。
2. 选择目标板并执行构建验证。
3. 在 `GP_Port/` 范围内推进协议、驱动和联调脚本开发。

## 提交边界建议

默认不提交以下本地环境或临时产物：

- `*.uvgui.*`
- `*.uvopt`
- `__pycache__/`
- `*.pyc`
- 未明确需要纳入版本控制的临时导出图片或日志


