---
name: AI端动作映射（中文）
description: "实现一个 AI端 功能，把语音或调试结果映射为可发送到 LED端 的动作对象"
argument-hint: "功能需求（例如：命令解析、动作映射、优先级、截图联动）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `AI端` 相关功能。

行为准则详见：`.claude/skills/karpathy-guidelines.md`
模块上下文、文件定位、执行流程、阅读组合和约束详见：`Project/xiaozhi-esp32/main/gp_port/README.md`
关键文件：`gp_led_matrix_esp32.cc`、`transport/`、`ui/`、`boards/lichuang-dev/`

## 输出格式

- `假设`
- `计划`
- `涉及文件`
- `验证`
- `后续步骤`
