---
name: WS2812 代码审查（中文）
description: "针对 WS2812 项目变更执行代码审查，聚焦缺陷、回归、时序风险与测试缺口"
argument-hint: "审查目标（commit、文件、模块或功能描述）"
agent: agent
model: "GPT-5 (copilot)"
---
对指定目标执行代码审查。

审查优先级（从高到低）：
1. 功能缺陷与行为回归
2. PWM + DMA + 行扫描路径的时序风险
3. 74HC595 + PMOS 电源切换风险
4. 缓冲区越界与内存安全问题
5. mini-OS 协作调度中的并发/调度问题
6. App/Mdl-Mid/Drv/HAL 分层越界与文件落位错误
7. 测试缺失与验证空白

分层目录映射（审查时必须核对）：
- App：`Sources/app/`
- Mdl/Mid：`Sources/fml/`
- Drv：`Sources/lib/` 与 `Sources/output/`
- HAL：`Sources/hal/`

执行要求：
1. 先阅读与目标直接相关的文件，避免无关泛化评论。
2. 先给 findings，且按严重级别排序。
3. 每个 finding 必须包含：影响、证据、可执行修复建议。
4. 若无 findings，需明确说明并给出残余风险。
5. 明确检查依赖方向：App -> Mdl/Mid/Drv -> HAL。
6. findings 之后再给简要总结。

输出格式：
- "Findings"（按严重级别）
- "Open questions / assumptions"
- "Change summary"
- "Test and validation gaps"
