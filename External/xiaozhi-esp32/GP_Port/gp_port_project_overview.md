# GP_Port Integration Overview

## 中文

### 目标

本目录用于承载小智 AI 与外部 AI8051U 之间的 16x16 LED 矩阵接口开发资产，包括协议定义、ESP32 侧驱动、AI8051U 侧接口骨架、分析文档和阶段 Prompt。

### 小智 AI 项目结构摘要

- 应用入口位于 `main/main.cc`，完成 NVS 初始化后进入 `Application` 单例。
- `main/application.cc` 负责统一调度音频、网络、显示、设备状态机和 MCP 工具。
- `main/boards/common/board.h` 定义板级硬件抽象；每个板型通过覆写 `GetDisplay()`、`GetLed()`、`GetAudioCodec()` 等接口接入具体外设。
- `main/boards/common/i2c_device.h` 提供 I2C 设备抽象，适合扩展外部驱动芯片。
- `main/led/led.h` 只暴露 `OnStateChanged()` 这一最小 LED 接口，现有复杂灯效参考实现位于 `main/led/circular_strip.cc`。

### 本次实现的落点

- 当前活跃板型固定为 `main/boards/lichuang-dev`。
- `lichuang-dev` 已使用 `GPIO1/GPIO2` 作为矩阵链路复用 I2C 总线。
- 共享协议头统一使用地址 `0x31`。
- 新增 `gp_led_matrix_esp32.h/.cc` 作为 ESP32 侧矩阵驱动，并在目标板上通过 `GetLed()` 接入。
- AI8051U 侧已落地 `gp_led_matrix_ai8051u.c/.h`，负责从机接收、协议解析和 WS2812 动作分发。

### 已实现内容

1. ESP32 侧状态驱动矩阵链路已经接入活跃板型构建系统。
2. 目标板已经声明矩阵 I2C 地址、默认亮度和实际连线引脚。
3. 设备侧可将语音结果与调试圆点状态映射为矩阵动作对象并通过 I2C 协议发送。
4. 当前链路遵循“仅显式图像更新时通信”，不会因待机/聆听等状态自动覆盖上一幅图像。
5. AI8051U 侧已显式切换 I2C 到 `P2.3/P2.4`，并在中断中收包、发包。
6. AI8051U 在未被远程占用时，默认显示渐变流动图案。
7. 小智语音侧已支持将颜色结果映射为纯色满屏或图案预设调用，预设包括 `diamond`、`cross`、`python_demo`、`scroll_subtitle`。
8. ESP32 主机侧已实现 `ACK_REQUIRED` 命令的同步读回校验与链路状态摘要显示。

### 未完成内容

1. 尚未加入链路断开后的自动心跳超时恢复，当前回退依赖显式释放动作或重新切回本地图案路径。
2. 现阶段矩阵图案仍以代码生成和现有 `test_image.h` 索引为主，未建立更高层素材管理。
3. 当前滚动字幕仍依赖 AI8051U 侧内置字模序列，尚未支持自由文本下发。
4. 仍需补充更完整的实机联调记录与异常场景验证。

### 关键约束

- 不直接把 `AI8051U.H` 纳入 ESP32 编译链。
- 所有新增文件统一放在 `GP_Port`。
- 与小智 AI 现有代码的耦合保持在板级层和 `Led` 抽象层。

## English

### Goal

This directory holds the staged implementation assets for a 16x16 LED matrix link between XiaoZhi AI on ESP32 and an external AI8051U controller.

### Architecture summary

- `main/main.cc` enters the application runtime.
- `main/application.cc` orchestrates state, audio, networking, display, and tool flows.
- `main/boards/common/board.h` is the hardware abstraction boundary.
- `main/boards/common/i2c_device.h` is the current reusable I2C device layer.
- `main/led/led.h` exposes the minimal LED state callback contract.

### Current implementation status

1. A shared protocol header now exists in this directory.
2. An ESP32-side matrix driver skeleton is connected to the `lichuang-dev` board.
3. Device states now map to RGB332 matrix frames that are sent over I2C in chunks.

### Remaining work

1. Add ESP32-side readback validation for `ACK_REQUIRED` commands.
2. Replace generated demo patterns with asset-backed frames where needed.
3. Add heartbeat timeout recovery and explicit reconnection policy.
