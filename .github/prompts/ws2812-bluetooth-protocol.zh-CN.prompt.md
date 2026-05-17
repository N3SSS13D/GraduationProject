---
name: 蓝牙协议改动（中文）
description: "在 Project/Protocols 下实现一个蓝牙通信协议层改动"
argument-hint: "协议任务（例如：命令字段更新、ACK 流程、分片大小、绘图契约）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `蓝牙通信协议` 任务。

模块上下文、文件定位、协议流程、阅读组合和约束详见：`.claude/skills/bluetooth-protocol.md`
关键文件：`Project/Protocols/gp_led_matrix_protocol.h`（唯一真相源）、`*_spec.md`、`*_pattern_protocol.md`

## 输出格式

- `假设`
- `计划`
- `涉及文件`
- `验证`
- `兼容性说明`
