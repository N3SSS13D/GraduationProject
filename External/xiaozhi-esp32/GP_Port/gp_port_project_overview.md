# GP_Port Integration Overview

## 中文

本目录用于承载 `AI端` 的自建扩展，当前重点是把动作对象通过经典蓝牙链路发送到 `LED端`。

### 目录分工

- `gp_led_matrix_esp32.h/.cc`：`AI端` 矩阵驱动与动作下发入口
- `transport/`：蓝牙传输层
- `ui/`：调试界面与链路状态显示
- `gp_led_matrix_protocol.h`：`AI端` 与 `LED端` 共享协议
- `gp_project_master.prompt.md` 与 `gp_phase_*.prompt.md`：阶段任务入口

### 当前目标

1. 稳定 `AI端` 动作对象到 `LED端` 协议的映射。
2. 稳定蓝牙链路日志、回包和错误追踪。
3. 在不扩大无关范围的前提下继续做性能优化。

### 快速路径

- 板级接入：`../main/boards/lichuang-dev/`
- 联调脚本：`../../tools/ws2812_dev_cycle.ps1`
- 问题说明：`../../Doc/项目文档/problem_tracking.md`

## English

This directory contains the custom `AI side` extension. The current focus is the Bluetooth-based action path from the AI side to the LED side.
