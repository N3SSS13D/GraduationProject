---
name: LED端驱动改动（中文）
description: "在 Project/STC51 下实现一个 LED端 显示驱动功能或修复"
argument-hint: "需求描述（例如：收包修复、动作执行、ws2812 时序、渲染链路）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `LED端` 功能或问题修复。

## 文件结构定位

只读取本次任务需要的最窄文件集合：

1. `LED端显示驱动`
   - `Project/STC51/ws2812_driver/Sources/app/`
   - `Project/STC51/ws2812_driver/Sources/mid/`
   - `Project/STC51/ws2812_driver/Sources/drv/`
   - `Project/STC51/ws2812_driver/Sources/inc/`
   - `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
2. `蓝牙通信协议`（仅在接口字段联动时）
   - `Project/Protocols/gp_led_matrix_protocol.h`
   - `Project/Protocols/gp_led_matrix_protocol_spec.md`

## 模块速览

- `Sources/app/app.c`
  - 启动初始化与运行主循环（`GpLedMatrixAi8051u_Poll -> MidTask_Process`）。
- `Sources/mid/gp_led_action.c`
  - 远程动作/帧/动画执行，以及本地/远程控制切换。
- `Sources/drv/gp_led_matrix_ai8051u.c`
  - `UART2` 组包、命令分发与 ACK/回包处理。
- `Sources/drv/ws2812_drv.c`
  - 16x16 `WS2812` 物理扫描输出。

## 优化工作流

对于 `LED端` 刷新、动画或调度优化任务：

1. 改动前先总结当前实现和热点路径。
2. 先列出当前问题、风险和候选优化方案。
3. 先选定最小可行改动，并明确验证标准。
4. 一次只实现一个聚焦切片。
5. 验证通过后再回看该链路，确认无误后再扩大范围。
6. 若改动影响时序规则或执行流程预期，同步更新文档、Prompt 和 Skill。

## 执行要求

1. 除协议边界联动外，改动范围限定在 `Project/STC51/`。
2. 共享字段和常量必须与 `Project/Protocols/gp_led_matrix_protocol.h` 保持一致。
3. 保持事件驱动主链路，不要在主循环引入阻塞式等待。
4. 改动要最小化，并能直接映射到需求。
5. 修改源码后执行 `Project/STC51/ws2812_driver/ws2812_driver.uvproj` 的 Keil rebuild。
6. 若改动影响时序或输出规则，同步更新 `Doc/Instructions/` 与 `Project/STC51/README.md`。

## 输出格式

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
