---
name: WS2812 AI 控制桥接（中文）
description: "实现一个小智 AI 控制功能，将 AI 指令映射为显示动作"
argument-hint: "AI 功能需求（例如：命令解析、动作映射、优先级策略、异常兜底）"
agent: agent
model: "GPT-5 (copilot)"
---
基于参数，仅实现一个“AI 控制功能”。

目标：
- 构建小智 AI 输入与显示动作之间的清晰接口层。
- 保证显示刷新时序不受 AI 消息抖动影响。

范围（AI 接口层）：
- 输入命令格式与解析
- 命令到显示动作映射
- 参数校验与降级兜底
- 与本地效果任务的队列/仲裁策略

当前结构（落位要求）：
- App：`Sources/app/`（根据语音/AI结果做业务决策）
- Mid：`Sources/mid/`（动作标准化、仲裁、状态机）
- Drv：`Sources/drv/`（调用显示驱动执行动作）
- Shared：`Sources/inc/`（共享动作定义与声明）
- 外设入口：`Sources/*.c`、`Sources/lib/`（仅底层 glue 逻辑）

本 prompt 不做：
- 大型上位机或 UI 工程
- 重写扫描驱动时序核心
- 一次实现过多 AI 平台能力

执行要求：
1. 先分析 `Sources/app`、`Sources/mid`、`Sources/drv` 与 `External/xiaozhi-esp32/GP_Port/` 的可复用边界。
2. 定义稳定 AI-显示 API 边界，优先对齐 `voice_color_result` 与 `gp_led_matrix_protocol.h`。
3. 增加严格边界检查与非法命令处理。
4. 在命令突发时保持显示刷新确定性。
5. 补充命令示例与集成说明。
6. 协议解析与动作仲裁放在 `mid/`，App 仅负责业务编排与策略。

当前阶段重点：
- 现有目标不是直接把 AI 消息塞进 8051 驱动，而是先形成“语音动作对象 -> I2C 自定义协议 -> 8051 显示动作”的稳定桥接层。
- 小智侧现有 `GP_Port` 资产已提供协议、驱动骨架和 MCP 联调参考，应优先复用。

输出格式：
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
