---
name: WS2812 代码审查（中文）
description: "针对项目改动执行代码审查，聚焦缺陷、回归、蓝牙传输风险与验证缺口"
argument-hint: "审查目标（commit、文件、模块或功能描述）"
agent: agent
model: "GPT-5 (copilot)"
---
对指定目标执行代码审查。

审查优先级：

1. 功能缺陷与行为回归
2. LED端 执行路径的时序风险
3. 蓝牙传输与协议一致性风险
4. 缓冲区越界与内存安全问题
5. 验证缺口

若审查对象涉及 `External/xiaozhi-esp32/GP_Port/`，额外检查：

- `AI端` 动作对象与 `LED端` 协议字段是否一致
- 传输、ACK 和状态日志是否一致

输出格式：

- `Findings`
- `Open questions / assumptions`
- `Change summary`
- `Test and validation gaps`
