# Local Drawing Scripts

## Category

`本地绘图脚本`

## Active paths

- 联调工具：`Project/Script/tools/`
- MCP 桥接：`Project/Script/mcp/gp_matrix/`
- 媒体工具：`Project/Script/media_tools/`

## Key docs

- `Project/Script/tools/ws2812_auto_debug.md`
- `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`

## Scope

本分类用于保存 MCP 接口、本地绘图脚本、联调脚本和辅助工具说明。

## Prompt / Skill 入口

- Prompt：`.github/prompts/ws2812-local-scripts*.prompt.md`
- Skill：`.github/skills/local-drawing-scripts/SKILL.md`

## Module quick map

- `mcp/gp_matrix/gp_display_mcp_bridge.py`
  - 主机侧绘图桥，负责把 LLM/工具请求转换为当前项目使用的预览与绘图输出格式。
- `mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - MCP 工具、参数、动画输入和失败规避规则说明。
- `mcp/gp_matrix/gp_mcp_endpoint_client.py`
  - 兼容入口，主要用于启动桥接服务，并统一分发 bitmap/animation/native action 三类 MCP 工具输出。
  - 主机桥对外工具保持 `self.screen.matrix_16x16.*`；`AI端` 本地直连调试工具统一保留在 `self.screen.matrix_16x16.local.*`。
- `media_tools/led_image_converter_gui.py`
  - 图片/文字到 16x16 显示素材的手工转换工具。
- `tools/ws2812_auto_debug.py`
  - `AI端 + LED端` 自动联调：先 Keil 编译，再延时串口监视，最后 ESP-IDF build/flash/monitor。

## Current host flow

1. 本地绘图请求进入 `gp_display_mcp_bridge.py`，主机侧 LLM 只使用 `self.screen.matrix_16x16.*` 工具
2. 脚本输出当前约定的绘图结果、动画序列或原生效果动作对象
3. 结果交给 `AI端` 预览、动作转发或蓝牙上传
4. `LED端` 只作为下游显示执行方，不直接暴露给脚本处理原始串口细节

## Common read bundles

- `绘图 / MCP`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
- `素材处理`
  - `Project/Script/media_tools/led_image_converter_gui.py`
- `联调自动化`
  - `Project/Script/tools/ws2812_auto_debug.py`
  - `Project/Script/tools/ws2812_auto_debug.md`

## Cross-category boundary

- 绘图脚本的输出必须兼容 `Project/Protocols/` 中的契约
- 如果需求是“原始文本 + 横向滚动参数 + 图像序列传输”，优先使用 `show_scroll_subtitle`；若只需 `LED端` 原生滚动字幕，则优先 `show_effect`
- 若联调流程变化，要同步更新本目录说明和 `Doc/Instructions/` 中的当前约束
