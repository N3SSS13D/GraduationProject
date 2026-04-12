# Phase 5 Prompt: Validation

## 中文
请验证 ESP32 构建、目标板接入、协议字段一致性以及 AI8051U 侧接口文档的闭环性。若无法实际连接硬件，请至少完成静态检查、构建验证和联调步骤设计，并记录未验证风险。

验证目标默认包括：

- 语音结果到 LED 动作对象的字段一致性。
- I2C 协议包的编码、分包、ACK 与错误路径。
- AI8051U 接收后执行 WS2812 动作的闭环步骤设计。
- 无硬件条件下的串口日志、模拟输入和联调脚本验证方案。

## English
Validate ESP32 build health, board integration, protocol consistency, and AI8051U interface completeness. If hardware is not available, finish static verification, build validation, and an explicit integration test plan instead.

By default, include voice-result field consistency, I2C packet encode/decode paths, ACK/error handling, and an end-to-end validation plan for AI8051U-executed WS2812 actions.