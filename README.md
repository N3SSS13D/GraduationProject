# GraduationProject

## 项目概述

本仓库用于毕业设计开发，当前主线已经从“方案设计”推进到“小智 ESP32 <-> AI8051U <-> WS2812”本地闭环联调：

1. `STC51/Project/ws2812_driver/`
   - 基于 STC AI8051U 的 WS2812 复用扫描显示系统。
   - 已形成稳定的 PWM + DMA、74HC595 行选、PMOS 高侧切换和 USB 调试链路。
   - 已落地 AI8051U 侧 I2C 从机、协议解析、动作分发与远程显示接管逻辑。
2. `External/xiaozhi-esp32/`
   - 引入的小智 AI 参考快照，并在 `GP_Port/` 中补充项目扩展。
   - 已实现语音颜色/预设识别、矩阵动作对象下发、ACK 读回校验和屏幕侧链路状态显示。

当前已完成的关键闭环为：

`小智语音/调试圆点状态 -> ESP32 动作映射 -> 本地 I2C 自定义协议 -> AI8051U 接收执行 -> WS2812 LED 矩阵显示`

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
- 已补充并接通 ESP32 侧矩阵驱动 `gp_led_matrix_esp32.h/.cc`。
- 已补充并落地 AI8051U 接口层 `gp_led_matrix_ai8051u.h/.c` 与动作执行层 `gp_led_action.c`。
- 已支持 `SetAction`、RGB332 帧、字模滚动、状态/错误回包。
- 已支持语音颜色结果与调试圆点状态同步到 LED 矩阵。
- 已支持矩阵预设调用：`diamond`、`cross`、`python_demo`、`scroll_subtitle`。
- 已支持“未指定预设时默认纯色满屏”策略，以及“仅在显式图像更新时通信”策略。
- 已支持图案背景色独立语音控制，且指定预设后不再自动轮播其他图案。
- 已支持小智屏幕左侧链路状态与最近一次矩阵命令摘要显示。

## 当前文档入口

- 当前项目总览与阶段计划：`Doc/项目文档/project_status_summary_2026-04-12.md`
- WS2812 驱动实现说明：`Doc/项目文档/ws2812_driver_current_implementation.md`
- 小智移植与联调总结：`Doc/项目文档/xiaozhi_esp32_porting_summary.md`
- 小智与 AI8051U I2C 协议说明：`Doc/项目文档/xiaozhi_ai8051u_i2c_interface_protocol.md`
- Prompt 索引：`.github/prompts/README.md`

## 下一阶段计划

1. 完成硬件联调闭环
   - 验证纯色、预设、滚动字幕、链路断开和异常回复路径。
2. 完善断链恢复策略
   - 补充心跳超时、自动释放远程模式和显式重连策略。
3. 扩展素材与文本能力
   - 在保持现有预设稳定的前提下，补充更高层素材管理与自由文本下发能力。
4. 收敛提交边界
   - 整理最终纳入版本控制的文档、prompt 和工程文件，剔除纯本地 IDE 产物。

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


