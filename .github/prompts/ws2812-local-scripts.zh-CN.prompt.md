---
name: 本地脚本与 MCP 改动（中文）
description: "在 Project/Script 下实现一个本地绘图脚本或 MCP 工具改动"
argument-hint: "脚本任务（例如：MCP 桥行为、payload 归一化、自动联调工具流程）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `本地绘图脚本` 任务。

模块上下文、文件定位、主机流程、阅读组合和约束详见：`.claude/skills/local-drawing-scripts.md`
关键文件：`gp_display_mcp_bridge.py`、`gp_mcp_endpoint_client.py`、`gp_matrix_drawing_mcp_usage.md`、`ws2812_auto_debug.py`

## 输出格式

- `假设`
- `计划`
- `涉及文件`
- `验证`
- `运维说明`
