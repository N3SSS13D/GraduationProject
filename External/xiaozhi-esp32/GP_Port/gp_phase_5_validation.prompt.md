# Phase 5 Prompt: Validation And Performance

## 中文
请验证当前 `AI端 -> 经典蓝牙链路 -> LED端` 主线的构建、日志、协议一致性和性能边界。

优先验证：

- `AI端` 动作对象与 `LED端` 协议字段是否一致。
- `LED端` UART2 收包、ACK 回包、超时清包是否正常。
- 蓝牙日志是否足以区分“链路有字节”和“协议已执行”。
- 调试界面、截图工具和联调脚本是否仍可用。
- 性能优化后是否影响刷新稳定性或链路可追踪性。

验证入口：

- `tools/ws2812_dev_cycle.ps1`
- `-RunAi8051BtDebug`
- `External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`

## English
Validate the current `AI side -> classic Bluetooth transport -> LED side` workflow, including build health, protocol consistency, logs, and performance boundaries.