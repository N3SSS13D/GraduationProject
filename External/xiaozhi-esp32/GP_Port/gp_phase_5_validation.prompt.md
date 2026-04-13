# Phase 5 Prompt: Validation

## 中文
请验证 ESP32 构建、目标板接入、协议字段一致性以及 AI8051U 侧接口文档的闭环性。若无法实际连接硬件，请至少完成静态检查、构建验证和联调步骤设计，并记录未验证风险。

验证目标默认包括：

- 语音结果到 LED 动作对象的字段一致性。
- I2C 协议包的编码、分包、ACK 与错误路径。
- 活跃板型 `lichuang-dev` 的矩阵链路引脚与地址一致性：`GPIO1/GPIO2` 对 `P2.4/P2.3`，地址 `0x31`。
- AI8051U `P2.4/P2.3` 是否已配置为开漏且未启用内部上拉，是否与外接 `3.3V` 上拉约束一致。
- 是否符合“仅显式图像更新时通信”的当前行为边界，以及 AI8051U 未被远程占用时渐变流动默认图案是否按预期回退。
- 语音测试圆点与 LED 矩阵的颜色、动画是否同步，且 `solid/pulse/gradient` 能映射到矩阵动作。
- AI8051U 调试日志是否区分 `payload_stop`、`restart_flush`、`empty_stop` 与 `[I2C_PKT]`，并能据此判断“总线有活动”与“协议已成功执行”的差异。
- 小智左侧连通性面板是否显示最近命令、序号、负载长度、回包状态与内容摘要，且能区分 `INIT/ONLINE/NO-REPLY/ERROR` 状态。
- AI8051U 接收后执行 WS2812 动作的闭环步骤设计。
- 无硬件条件下的串口日志、模拟输入和联调脚本验证方案。

## English
Validate ESP32 build health, board integration, protocol consistency, and AI8051U interface completeness. If hardware is not available, finish static verification, build validation, and an explicit integration test plan instead.

By default, include voice-result field consistency, active-board pin/address checks for the `lichuang-dev` matrix path, explicit open-drain validation for AI8051U P2.4/P2.3, the current explicit-image-update-only behavior boundary and gradient fallback behavior, synchronized voice-debug-dot to matrix action checks, I2C packet encode/decode paths, ACK/error handling, link-status panel validation, and an end-to-end validation plan for AI8051U-executed WS2812 actions.