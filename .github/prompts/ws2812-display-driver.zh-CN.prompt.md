---
name: WS2812 显示驱动开发（中文）
description: "为 STC AI8051U WS2812 复用扫描系统实现一个底层驱动功能"
argument-hint: "驱动功能需求（例如：行扫描时序修复、74HC595 更新序列、可配置分辨率映射）"
agent: agent
model: "GPT-5 (copilot)"
---
基于参数，仅实现一个“驱动层功能”。

硬件约束：
- STC AI8051U，WS2812 通过 PWM + DMA 输出
- 两颗级联 74HC595 控制 16 路 PMOS 高侧开关
- 奇偶行分别复用两根 PWM 信号线P10 P12
- 相邻两行同时扫描并执行互补渐变切换

范围（仅驱动层）：
- io/hal 时序配置
- 74HC595 行开关控制
- PMOS 上电/断电时序
- 扫描调度挂钩（不实现完整动画）
- 驱动层分辨率映射

分层目录结构（落位要求）：
- App：`Sources/app/`（禁止放置驱动实现）
- Mdl/Mid：`Sources/fml/`（仅硬件无关工具/协议）
- Drv：`Sources/lib/`，显示/输出驱动可使用 `Sources/output/`
- HAL：`Sources/hal/`（MCU 寄存器/厂家库访问）

本 prompt 不做：
- 复杂动画库设计
- AI 命令解析
- 大范围无关重构

执行要求：
1. 先阅读并分析现有代码：[STC51/Project/ws2812_driver/Sources](../../STC51/Project/ws2812_driver/Sources)
2. 复用当前架构与命名风格。
3. 保持时序关键路径确定性。
4. 优先定长缓冲与编译期配置。
5. 默认 16x8 兼容，并支持可配置分辨率。
6. 保持 Drv -> HAL 单向依赖，避免在 Drv 文件中混入应用业务逻辑。

输出格式：
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
