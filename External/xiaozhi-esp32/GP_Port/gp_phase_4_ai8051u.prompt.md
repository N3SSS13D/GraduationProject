# Phase 4 Prompt: AI8051U Interface Layer

## 中文
请围绕 `GP_Port/gp_led_matrix_ai8051u.h` 继续推进 AI8051U 侧接口层设计或模板实现。重点覆盖 I2C 从机收发、包解析、帧缓存和矩阵刷新，避免一开始就把范围扩展成完整固件工程，除非有明确要求。

本阶段应优先落地：

- I2C 从机接收缓存与事件标志。
- 协议包解析、校验和命令分发。
- 颜色/动画/文本动作与 WS2812 执行路径的映射。
- ACK 回包、错误码和超时恢复的接口边界。

## English
Continue the AI8051U-side interface layer from `GP_Port/gp_led_matrix_ai8051u.h`, focusing on I2C slave handling, packet parsing, frame buffering, and refresh logic without expanding into a full firmware project unless explicitly requested.

Prioritize receive buffering, packet parsing, color/animation/text action mapping, and the interface boundary for ACK, error reporting, and timeout recovery.