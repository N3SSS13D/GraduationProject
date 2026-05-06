# GP_Port Integration Overview

## 中文

本目录用于承载 MCP 与矩阵绘图桥接脚本，当前重点是稳定 `AI端 -> 蓝牙 -> LED端` 的矩阵控制链路。

### 关键文档（仅保留）

1. `gp_port_project_overview.md`
用途：目录总览与最小入口。

2. `gp_matrix_drawing_mcp_usage.md`
用途：LLM 侧 MCP 调用主文档（工具选择、输入约束、标准模板、失败修复）。

3. `gp_matrix_pattern_protocol.md`
用途：`AI端` 与主机间图案/动画请求和调试 websocket 交互约束。

4. `gp_led_matrix_protocol_spec.md`
用途：`AI端` 与 `LED端` 协议规范（包结构、命令、时序、错误码）。

### 非关键文档处理策略

- 重复或相近内容统一并入上述关键文档。
- 阶段性 prompt、临时说明、旧版并行说明不再保留在旧 `GP_Port/`。

### 快速路径

- AI端板级接入：`Project/xiaozhi-esp32/main/boards/lichuang-dev/`
- AI端矩阵驱动：`Project/xiaozhi-esp32/main/gp_port/`
- 联调脚本：`Project/Script/tools/ws2812_auto_debug.py`
- 问题说明：`Doc/Instructions/problem_tracking.md`

## English

This folder keeps the MCP-side bridge scripts and reference docs for the matrix Bluetooth pipeline.

Only four key docs are retained here:

1. `gp_port_project_overview.md`
2. `gp_matrix_drawing_mcp_usage.md`
3. `gp_matrix_pattern_protocol.md`
4. `gp_led_matrix_protocol_spec.md`
