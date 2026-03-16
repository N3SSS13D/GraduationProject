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

分层目录结构（落位要求）：
- App：`Sources/app/`（根据语音/AI结果做业务决策）
- Mdl/Mid：`Sources/fml/`（协议解析、命令标准化等硬件无关逻辑）
- Drv：`Sources/lib/` 与 `Sources/output/`（调用显示驱动执行动作）
- HAL：`Sources/hal/`（仅寄存器/厂家库访问）

本 prompt 不做：
- 大型上位机或 UI 工程
- 重写扫描驱动时序核心
- 一次实现过多 AI 平台能力

执行要求：
1. 先分析 app/fml/lib/output/hal 分层结构。
2. 定义稳定 AI-显示 API 边界。
3. 增加严格边界检查与非法命令处理。
4. 在命令突发时保持显示刷新确定性。
5. 补充命令示例与集成说明。
6. 协议解析放在 Mdl/Mid，App 仅负责业务编排与策略。

输出格式：
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
