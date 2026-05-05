# Active Instructions Index

当前仓库的有效说明文档按四类组织：

1. `LED端显示驱动`
   - 代码：`Project/STC51/`
   - 入口：`Project/STC51/README.md`
2. `AI端接口调度`
   - 代码：`Project/xiaozhi-esp32/main/gp_port/`
   - 入口：`Project/xiaozhi-esp32/main/gp_port/README.md`
3. `蓝牙通信协议`
   - 协议资产：`Project/Protocols/`
   - 入口：`Project/Protocols/README.md`
4. `本地绘图脚本`
   - 脚本资产：`Project/Script/`
   - 入口：`Project/Script/README.md`

各分类 README 现在同时承担以下作用：

- 说明当前分类下的主模块和职责分工
- 记录当前主链路或数据流
- 给出常见任务的最小阅读集，减少无关文件进入上下文

公共当前文档：

- `./project_structure.md`
- `./problem_tracking.md`
- `./bt_version_hc05_uart2_architecture.md`
- `./led_display_profile_structure.md`
- `./led_refresh_optimization.md`

## Prompt / Skill 入口矩阵

1. `LED端显示驱动`
   - Prompt: `.github/prompts/ws2812-led-driver*.prompt.md`
   - Skill: `.github/skills/ws2812-led-driver/SKILL.md`
2. `AI端接口调度`
   - Prompt: `.github/prompts/ws2812-ai-control*.prompt.md`
   - Skill: `.github/skills/karpathy-guidelines/SKILL.md`
3. `蓝牙通信协议`
   - Prompt: `.github/prompts/ws2812-bluetooth-protocol*.prompt.md`
   - Skill: `.github/skills/bluetooth-protocol/SKILL.md`
4. `本地绘图脚本`
   - Prompt: `.github/prompts/ws2812-local-scripts*.prompt.md`
   - Skill: `.github/skills/local-drawing-scripts/SKILL.md`

## 推荐调用顺序

1. 先读对应分类 README，确定边界与最小阅读集
2. 再进入该分类 Prompt，约束本次任务目标与输出
3. 最后按需使用对应 Skill，补齐实现细则与验证要点

历史方案统一放入：`../History/`
