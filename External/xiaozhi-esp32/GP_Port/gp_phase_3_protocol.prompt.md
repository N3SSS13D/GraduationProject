# Phase 3 Prompt: Protocol Refinement

## 中文
请细化 `GP_Port/gp_led_matrix_protocol.h` 和协议规范文档，确保命令集合、分包策略、校验、错误码、状态回传与 `test_image.h` 的数据格式映射保持一致。如果协议变更影响 ESP32 驱动或 AI8051U 接口文档，请同步更新。

本阶段必须额外覆盖：

- 语音控灯场景下的颜色、亮度、动画、文本或图案参数表达。
- ACK、超时重试、状态回读、序号管理和心跳机制。
- 与 AI8051U 显示动作接口的最小稳定边界。

## English
Refine `GP_Port/gp_led_matrix_protocol.h` and the protocol spec so that command set, chunking strategy, checksum rules, error handling, and payload mapping stay aligned with `test_image.h`.

Also cover the minimal stable action model for voice-controlled color, brightness, animation, text/pattern delivery, plus ACK, retry, status-readback, sequence, and heartbeat handling.