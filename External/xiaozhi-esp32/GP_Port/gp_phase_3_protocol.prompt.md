# Phase 3 Prompt: Bluetooth Transport And Protocol

## 中文
请围绕当前蓝牙主线细化传输与协议，优先修改：

- `GP_Port/gp_led_matrix_protocol.h`
- `GP_Port/transport/`
- 协议说明文档

本阶段重点：

- 分包、校验、ACK、错误码、状态回传
- `AI端` 与 `LED端` 字段保持一致
- 蓝牙链路下的超时、重试和日志可追踪性

## English
Refine the Bluetooth-oriented transport and protocol path, focusing on packet framing, checksum, ACK, error handling, status replies, and consistency between the AI side and the LED side.