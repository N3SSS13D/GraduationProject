---
name: WS2812 动画效果开发（中文）
description: "围绕当前蓝牙主线，实现一个适合 AI端 到 LED端 传输的效果或预设"
argument-hint: "效果需求（例如：文字滚动、呼吸、涟漪、转场、预设整理）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个效果或预设相关改动。

执行要求：

1. 效果算法优先放在 `Sources/mid/`。
2. 优先保证刷新稳定，避免闪烁和撕裂。
3. CPU 和内存开销需适配 8051 资源约束。
4. 参数设计应适配当前 `AI端 -> LED端` 协议传输。
5. 修改后执行可用验证。

输出格式：

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
