# Active Instructions Index

## Navigation (recommended reading order)

1. **First**: `project_structure.md` — understand the 4-category layout, entry points, and read bundles
2. **Second**: Your category's README — module details, execution flow, common read bundles
   - `Project/STC51/README.md` (LED端)
   - `Project/xiaozhi-esp32/main/gp_port/README.md` (AI端)
   - `Project/Protocols/README.md` (协议)
   - `Project/Script/README.md` (脚本)
3. **Third**: `problem_tracking.md` — current constraints, module hot spots, and known issues
4. **Cross-module**: `end_to_end_data_flow.md` — full data chain from MCP tool to LED pixel
5. **Reference**: `module_interface_spec.md` — interfaces exposed/consumed by each module

## Technical Reference Docs

| Doc | Covers |
|---|---|
| `led_driver_tech_ref.md` | LED-side: file map, key functions, scan architecture, task scheduling, timing (EN) |
| `led_driver_tech_ref_zh.md` | 同上，中文版：文件映射、关键函数、扫描架构、任务调度、时序 |
| `ai_interface_tech_ref.md` | AI-side: file map, classes/functions, dual control path, transport details |
| `protocol_tech_ref.md` | Protocol: packet format, command set, image formats, animation limits |
| `scripts_tech_ref.md` | Scripts: file map, MCP tools, drawing pipeline, AST whitelist, auto-debug |
| `led_display_profile_structure.md` | Display profile data structure: fields, protocol mapping, timeline executor |
| `led_refresh_optimization.md` | LED refresh optimization history and solutions |
| `bt_version_hc05_uart2_architecture.md` | End-to-end Bluetooth/UART2 architecture and link layer |

## Four Categories

1. `LED端显示驱动` — `Project/STC51/` → README: `Project/STC51/README.md`
2. `AI端接口调度` — `Project/xiaozhi-esp32/main/gp_port/` → README: `Project/xiaozhi-esp32/main/gp_port/README.md`
3. `蓝牙通信协议` — `Project/Protocols/` → README: `Project/Protocols/README.md`
4. `本地绘图脚本` — `Project/Script/` → README: `Project/Script/README.md`

## Prompt / Skill 入口

Skills 统一在 `.claude/skills/`（Claude Code 和 GitHub Copilot 共享）：

1. `LED端显示驱动` → Skill: `.claude/skills/ws2812-led-driver.md` | Prompt: `.github/prompts/ws2812-led-driver*.prompt.md`
2. `AI端接口调度` → Skill: `.claude/skills/karpathy-guidelines.md` | Prompt: `.github/prompts/ws2812-ai-control*.prompt.md`
3. `蓝牙通信协议` → Skill: `.claude/skills/bluetooth-protocol.md` | Prompt: `.github/prompts/ws2812-bluetooth-protocol*.prompt.md`
4. `本地绘图脚本` → Skill: `.claude/skills/local-drawing-scripts.md` | Prompt: `.github/prompts/ws2812-local-scripts*.prompt.md`

推荐调用顺序：先读分类 README → 再进入对应 Skill → 最后按需使用 Copilot Prompt。

历史方案统一放入：`../History/`
