# Phase 5 Prompt: Validation

## 中文
请验证 ESP32 构建、目标板接入、协议字段一致性以及 AI8051U 侧接口文档的闭环性。若无法实际连接硬件，请至少完成静态检查、构建验证和联调步骤设计，并记录未验证风险。

验证目标默认包括：

- 语音结果到 LED 动作对象的字段一致性。
- I2C 协议包的编码、分包、ACK 与错误路径。
- 活跃板型 `lichuang-dev` 的矩阵链路引脚与地址一致性：`GPIO1/GPIO2` 对 `P2.4/P2.3`，地址 `0x31`。
- AI8051U `P2.4/P2.3` 是否已配置为开漏且未启用内部上拉，是否与外接 `3.3V` 上拉约束一致。
- AI8051U 在线/离线模式仲裁是否符合预期：有效协议通信进入在线，通信超时回到离线，且长按 `P33` 可人工反转当前模式。
- AI8051U I2C DMA 路径是否符合当前策略：默认 RX/TX 双向 DMA 正常工作，`RXLOSS`、`TXOVW`、DMA 完成长度与回复长度均符合协议预期，且可通过 `GpLedMatrixAi8051u_SetDmaMode()` 分方向切换。
- 是否符合“仅显式图像更新时通信”的当前行为边界，以及 AI8051U 未被远程占用时渐变流动默认图案是否按预期回退。
- 语音测试圆点与 LED 矩阵的颜色、动画是否同步，且 `solid/pulse/gradient` 能映射到矩阵动作。
- AI8051U 调试日志是否区分 `payload_stop`、`restart_flush`、`empty_stop` 与 `[I2C_PKT]`，并能据此判断“总线有活动”与“协议已成功执行”的差异。
- 小智主界面是否保持原有聊天/状态功能，且仅通过 `DBG` 入口进入次级调试菜单。
- 次级调试菜单标题栏是否固定保持 `Back / Debug Menu / S` 结构。
- 次级调试菜单是否已收敛为单页绝对布局，而不是继续依赖左右滑动分页。
- 左侧触摸控制区是否能在当前屏幕高度下完整显示 `Pattern` 与 `Effect`，无需滚动即可看到全部控制项。
- 右上 `Dot` 标题是否居中，`Link` 面板是否把状态圆点移动到 `Link` 标题后方，并把状态文本稳定显示在下方而不出现断裂错位。
- `Link` 状态区是否稳定显示最近命令、序号与回包结果，且能区分 `INIT/ONLINE/NO-REPLY/ERROR` 状态。
- 标题栏圆形 `S` 按钮是否固定在最右侧，且点击后能够触发截图而不占用控制区。
- `Pattern` 切换到 `JLU_emblem` 后，预览是否仍完整显示而不被调试菜单有效区域裁切。
- 标题栏 `S` 按钮是否会把按键当下的屏幕内容冻结到独立缓冲区，并在释放 LVGL 快照资源后于后台完成 PNG 编码。
- 标题栏 `S` 是否会在后台完成 PNG 编码后直接通过本地 HTTP 上传到开发机，而不是继续依赖官方 MCP 语音桥接的反向 `tools/call`。
- `External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py` 是否可在不传参情况下同时启动 MCP 端点接入与本地 HTTP 截图接收端，并持续显示连接/分块/HTTP 落盘状态。
- 设备侧 `self.screen.debug_snapshot.set_upload_url` / `get_upload_url` 是否能正确持久化并读取 `Snap` 使用的本地 HTTP 上传地址。
- 串口命令 `snap|snap <quality>` 是否能直接触发设备侧截图，且截图排队、上传成功/失败信息只走串口日志而不占用屏幕空间。
- 串口命令 `snap_url get|set|clear|reset` 是否能正确管理设备侧 `Snap` 的 HTTP 上传地址。
- 编译期固定 URL 是否会在设备首次启动且 NVS 尚未配置时自动写入，避免重复人工设置。
- 修改代码后是否按规定联调顺序执行：小智侧先编译/下载/监控，再运行 MCP 脚本，再通过串口逐一调用本地 MCP 接口；LED 驱动侧先关闭 serial monitor，再 Keil 编译，等待 20s 后重新打开串口读取日志。
- 小智在线控制 LED 时的默认亮度是否已与 AI8051U 离线模式对齐，不再明显偏暗。
- 次级调试菜单是否符合“左侧控制 + 右侧状态 + 下方摘要”的稳定版布局要求，且圆点、链路状态和摘要信息都可见。
- 调试菜单隐藏时，调试圆点动画与调试面板刷新是否停止或降到最小，不影响原小智功能响应速度。
- 双向通信是否符合当前策略：AI8051U 收到完整包后可立即提供 ACK 回包，小智侧写命令后延时短轮询读取状态包，不再长期停留在 `send failed`。
- AI8051U 接收后执行 WS2812 动作的闭环步骤设计。
- 无硬件条件下的串口日志、模拟输入和联调脚本验证方案。

## English
Validate ESP32 build health, board integration, protocol consistency, and AI8051U interface completeness. If hardware is not available, finish static verification, build validation, and an explicit integration test plan instead.

By default, include voice-result field consistency, active-board pin/address checks for the `lichuang-dev` matrix path, explicit open-drain validation for AI8051U P2.4/P2.3, online/offline mode arbitration checks, the current explicit-image-update-only behavior boundary and gradient fallback behavior, synchronized voice-debug-dot to matrix action checks, I2C packet encode/decode paths, ACK/error handling, link-status panel validation, and an end-to-end validation plan for AI8051U-executed WS2812 actions.