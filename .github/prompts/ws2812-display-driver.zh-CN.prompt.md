---
name: WS2812 显示驱动开发（中文）
description: "围绕当前 UART2 与 WS2812 执行路径，实现一个 LED端 驱动功能"
argument-hint: "驱动功能需求（例如：行扫描时序、74HC595 序列、UART2 接收挂钩、分辨率映射）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `LED端` 驱动功能。

关注路径：

- `STC51/Project/ws2812_driver/Sources/drv/`
- `STC51/Project/ws2812_driver/Sources/inc/`
- `STC51/Project/ws2812_driver/Sources/*.c`

执行要求：

1. 保持时序关键路径确定性。
2. 优先使用定长缓冲区和显式边界检查。
3. 保持 `drv -> 外设入口` 单向依赖。
4. 与当前 UART2 协议执行路径保持兼容。
5. 修改源码后执行 Keil rebuild。

输出格式：

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
