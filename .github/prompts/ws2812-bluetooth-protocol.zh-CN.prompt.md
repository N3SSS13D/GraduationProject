---
name: 蓝牙协议改动（中文）
description: "在 Project/Protocols 下实现一个蓝牙通信协议层改动"
argument-hint: "协议任务（例如：命令字段更新、ACK 流程、分片大小、绘图契约）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `蓝牙通信协议` 任务。

## 文件结构定位

优先从协议资产开始，只有在消费端校验时才扩展读取：

1. `蓝牙通信协议`
   - `Project/Protocols/gp_led_matrix_protocol.h`
   - `Project/Protocols/gp_led_matrix_protocol_spec.md`
   - `Project/Protocols/gp_matrix_pattern_protocol.md`
2. `AI端接口调度`（消费侧校验）
   - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
3. `LED端显示驱动`（消费侧校验）
   - `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
4. `本地绘图脚本`（契约侧校验）
   - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`

## 解决问题工作流

对于协议改动或协议问题修复任务：

1. 改动前先总结当前包契约，以及受影响的发送端/接收端路径。
2. 先列出协议风险、兼容性影响和候选方案，再开始编辑。
3. 先选定最小可行的协议改动，并定义验证标准。
4. 一次只实现一个聚焦的协议切片。
5. 验证后先复查协议文档与消费端对齐，再扩大范围。
6. 若字段语义或流程预期变化，同步更新文档、Prompt 和 Skill。

## 执行要求

1. 以 `Project/Protocols/gp_led_matrix_protocol.h` 为唯一共享源头。
2. 若包结构或语义变更，必须在同一任务同步更新协议文档。
3. 保持 `AI端` 与 `LED端` 对字段和限制的消费一致。
4. 若影响脚本输入/输出格式，必须同步更新脚本契约文档。
5. 兼容性影响要可说明、可验证。
6. 协议设计优先满足：唯一可识别性、数据完整性、结构可扩展性、解析高效性、双向交互可靠性。
7. 对当前 V2 协议，优先采用“先校验 `header_size + header_crc8` 再信任 `payload_length`、以 `packet_type=Reply + reply_to_sequence` 匹配 ACK、分片使用显式字节偏移”的方案。

## 输出格式

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Compatibility notes`
