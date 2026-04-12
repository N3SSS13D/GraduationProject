# Phase 1 Prompt: Architecture Review

## 中文
请梳理 xiaozhi-esp32 的应用入口、状态机、板级抽象、I2C 外设模式和显示/LED 联动路径，并把结果更新到 `GP_Port` 目录中的架构说明文档。重点说明 `Application`、`Board`、`I2cDevice`、`Led`、目标板 `lichuang-dev` 的关系。

补充要求：

- 结合当前毕业设计场景，额外梳理“小智语音结果 -> 板级 LED/I2C 接口 -> AI8051U 协处理器”的目标调用路径。
- 若现有架构中已经存在可复用的动作状态、消息模型或调试工具，需明确写入文档，避免后续重复造轮子。

## English
Review the application entry flow, state machine, board abstraction, I2C patterns, and LED/display integration for xiaozhi-esp32, then update the architecture notes inside `GP_Port`.

Also map the intended path from XiaoZhi voice results to board-level LED/I2C integration and then to the external AI8051U controller.