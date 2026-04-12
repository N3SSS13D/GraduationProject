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

- 目标板型固定为 `main/boards/lichuang-dev`。
- 新增 `gp_led_matrix_protocol.h` 作为 ESP32 与 AI8051U 共享的协议头。
- 新增 `gp_led_matrix_esp32.h/.cc` 作为 ESP32 侧矩阵驱动骨架，并在目标板上通过 `GetLed()` 接入。
- 新增 `gp_led_matrix_ai8051u.h` 作为 AI8051U 侧接口层头文件骨架。

### 已实现内容

1. ESP32 侧状态驱动矩阵骨架已经接入构建系统。
2. 目标板已经声明矩阵 I2C 地址和默认亮度。
3. 设备状态会被映射成简单的 RGB332 帧图案并通过 I2C 协议分包发送。

### 未完成内容

1. AI8051U 侧 `.c` 实现尚未落地，目前提供的是接口边界和设计文档。
2. 现阶段矩阵图案为代码生成图案，尚未直接复用 `test_image.h` 的静态资产。
3. 协议未加入读回 ACK、错误恢复和心跳联调代码，仅完成帧结构与下行发送骨架。

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

1. Add the AI8051U `.c` implementation.
2. Replace generated demo patterns with asset-backed frames where needed.
3. Add ACK handling, error recovery, and heartbeat validation.
