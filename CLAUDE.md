# GraduationProject — Claude 工作指南

## 项目概述

基于蓝牙的 WS2812 LED 矩阵显示系统。四个活跃分类：

| 分类 | 路径 | 说明 |
|---|---|---|
| LED端显示驱动 | `Project/STC51/` | AI8051U MCU，WS2812 矩阵扫描，UART2 协议接收 |
| AI端接口调度 | `Project/xiaozhi-esp32/main/gp_port/` | ESP32-S3，语音/调试动作映射，蓝牙传输 |
| 蓝牙通信协议 | `Project/Protocols/` | 共享协议头与规范文档 |
| 本地绘图脚本 | `Project/Script/` | MCP 桥接、图像转换、自动调试工具 |

## 文件定位

1. 先根据任务归属分类，只读该分类的相关文件
2. 先读 `Doc/Instructions/project_structure.md` 了解完整布局和入口矩阵
3. 再读对应分类的 README 获取模块图、执行流程、常用阅读组合
4. 当前约束和热点见 `Doc/Instructions/problem_tracking.md`
5. 跨模块理解见 `Doc/Instructions/end_to_end_data_flow.md`
6. 技术参考见 `Doc/Instructions/led_driver_tech_ref.md` 等

代码风格、命名规范、代码质量规则见 `.github/copilot-instructions.md`。

## 构建与验证

- STC51 源码修改后，对 `Project/STC51/ws2812_driver/ws2812_driver.uvproj` 运行 Keil 重新构建
- AI端 (ESP32) 修改后，运行 ESP-IDF `build flash monitor`
- 默认自动调试链顺序：
  1. 仅 Keil 重新构建 STC51 工程
  2. 等待 20s，打开 AI8051U 串口监视器（默认 `COM15`）
  3. ESP-IDF `build flash monitor` for `Project/xiaozhi-esp32`
- 工具路径：Keil `S:\Embedded\Keil`，ESP-IDF `S:\Embedded\ESP\v5.4.3\esp-idf`

## 项目 Skills

Skills 定义在 `.github/skills/`，Claude Code 和 GitHub Copilot 共享使用。通过 `/skill-name` 调用：

| Skill | 用途 |
|---|---|
| `karpathy-guidelines` | 通用编码行为准则（写、审阅、重构时自动应用） |
| `ws2812-led-driver` | LED端显示驱动变更 |
| `bluetooth-protocol` | 蓝牙通信协议变更 |
| `local-drawing-scripts` | 本地绘图脚本与 MCP 工具变更 |
| `ai8051u-i2c-dma` | AI8051U I2C DMA 支持 |

任务分类 → Skill 映射：
- `LED端显示驱动` → `/ws2812-led-driver`
- `AI端接口调度` → 入口: `gp_led_matrix_esp32.cc`, `transport/`, `ui/`, `boards/lichuang-dev/`
- `蓝牙通信协议` → `/bluetooth-protocol`
- `本地绘图脚本` → `/local-drawing-scripts`

## 输出格式

每个任务完成后提供简洁总结：变更概述、涉及文件、验证状态。
