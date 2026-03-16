---
name: WS2812 动画效果开发（中文）
description: "在现有 WS2812 驱动基础上实现一个显示动画或视觉效果"
argument-hint: "动画需求（例如：文字滚动、呼吸、涟漪、波形、转场）"
agent: agent
model: "GPT-5 (copilot)"
---
基于参数，仅实现一个“动画/效果功能”。

默认前提：底层扫描驱动已可用（PWM + DMA + 74HC595 PMOS 切换）。

范围（效果层）：
- framebuffer 更新
- 效果状态机
- 与扫描刷新兼容的时序策略
- 可选双缓冲集成

分层目录结构（落位要求）：
- App：`Sources/app/`（场景与业务编排）
- Mdl/Mid：`Sources/fml/`（硬件无关动画算法）
- Drv：`Sources/lib/` 与 `Sources/output/`（像素输出与扫描驱动）
- HAL：`Sources/hal/`（仅硬件访问）

本 prompt 不做：
- 非必要情况下改写底层时序关键驱动
- AI 协议接入
- 一次实现多个效果功能

执行要求：
1. 先分析现有项目结构与效果代码模式。
2. 优先保证刷新稳定，避免撕裂/闪烁。
3. 效果行为需支持分辨率配置（默认 16x16）。
4. CPU 与内存开销需适配 8051 资源约束。
5. 补充简明触发与使用说明。
6. 动画算法优先放在 Mdl/Mid，硬件访问保留在 Drv/HAL。

输出格式：
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
