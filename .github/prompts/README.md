# Prompt Catalog

当前 prompt 和 skill 统一按四类结构组织。

## 文件定位

1. 先读 `Doc/Instructions/project_structure.md` 了解完整布局
2. 再读对应分类的 README 获取模块细节
3. Skills 统一在 `.claude/skills/`，Prompts 为 Copilot 专用 Agent 入口

## Prompts

### LED端显示驱动
- [ws2812-led-driver.prompt.md](./ws2812-led-driver.prompt.md) | [zh-CN](./ws2812-led-driver.zh-CN.prompt.md)
- Skill: `.claude/skills/ws2812-led-driver.md`

### AI端接口调度
- [ws2812-ai-control.prompt.md](./ws2812-ai-control.prompt.md) | [zh-CN](./ws2812-ai-control.zh-CN.prompt.md)
- Skill: `.claude/skills/karpathy-guidelines.md`

### 蓝牙通信协议
- [ws2812-bluetooth-protocol.prompt.md](./ws2812-bluetooth-protocol.prompt.md) | [zh-CN](./ws2812-bluetooth-protocol.zh-CN.prompt.md)
- Skill: `.claude/skills/bluetooth-protocol.md`

### 本地绘图脚本
- [ws2812-local-scripts.prompt.md](./ws2812-local-scripts.prompt.md) | [zh-CN](./ws2812-local-scripts.zh-CN.prompt.md)
- Skill: `.claude/skills/local-drawing-scripts.md`

### 跨分类代码审查
- [ws2812-code-review.prompt.md](./ws2812-code-review.prompt.md) | [zh-CN](./ws2812-code-review.zh-CN.prompt.md)

## Skills

- `karpathy-guidelines` — 通用编码行为准则
- `ws2812-led-driver` — LED端显示驱动
- `bluetooth-protocol` — 蓝牙通信协议
- `local-drawing-scripts` — 本地绘图脚本
- `ai8051u-i2c-dma` — AI8051U I2C DMA 专项参考

入口结构：`分类 README → Skill → Prompt`
