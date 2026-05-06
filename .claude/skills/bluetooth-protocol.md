# 蓝牙通信协议

## 分类

`蓝牙通信协议`

## 适用范围

`Project/Protocols/` 下的协议任务：

- `Project/Protocols/gp_led_matrix_protocol.h`
- `Project/Protocols/gp_led_matrix_protocol_spec.md`
- `Project/Protocols/gp_matrix_pattern_protocol.md`

## 文件定位

- 从 `Project/Protocols/` 文件开始
- 仅在验证协议字段如何被消费时读取 `AI端` 或 `LED端` 实现文件
- 除非协议边界直接涉及，否则避免扫描无关的脚本或 UI 文件

## 文件快速图

- `Project/Protocols/gp_led_matrix_protocol.h` — 共享唯一真相源：线常量、命令 ID、负载结构体、传输限制
- `Project/Protocols/gp_led_matrix_protocol_spec.md` — 包级行为：布局、分阶段帧传输、动画传输、ACK/状态期望
- `Project/Protocols/gp_matrix_pattern_protocol.md` — 主机/脚本绘图契约：输入 AI端预览和蓝牙上传

## 常见协议流程

- 主机绘图契约 → `matrix_pattern_request` / `matrix_pattern_result`
- AI端发送方 → `gp_led_matrix_esp32.cc`
- 线封包布局和限制 → `gp_led_matrix_protocol.h`
- LED端解析器/执行器 → `Sources/drv/gp_led_matrix_ai8051u.c`

## 常用阅读组合

- `线格式 / 封包字段变更` → `gp_led_matrix_protocol.h` + `gp_led_matrix_protocol_spec.md`
- `主机绘图契约变更` → `gp_matrix_pattern_protocol.md` + `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
- `消费方验证` → `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc` + `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`

## 问题解决工作流

对于协议变更和协议 bug 修复：

1. 先总结当前契约和受影响的生产者/消费者路径
2. 陈述兼容性风险、故障模式和候选方案，再编辑
3. 优先选择可独立验证的最小协议变更
4. 每个聚焦的协议切片后验证消费方对齐
5. 当协议语义或工作流预期变化时，同步文档和 prompt/skill 指导

## 要求

1. 保持 AI端和 LED端字段定义对齐
2. 将 `Project/Protocols/gp_led_matrix_protocol.h` 视为活跃常量和负载结构体的共享唯一真相源
3. 若协议行为变更，同步更新共享头文件和 `Project/Protocols/` 中的协议文档
4. 若协议变更影响脚本负载格式或 MCP 期望，更新 `Project/Script/` 下的文档
5. 选择协议形态时，优先考虑：明确成帧、完整性校验、可扩展性、解析效率、可靠交互
6. 对于活跃的 V2 线格式，优先头优先验证（`header_size` + CRC8）、按 `reply_to_sequence` 匹配的包类型回复、分阶段帧或字形块的显式字节偏移
