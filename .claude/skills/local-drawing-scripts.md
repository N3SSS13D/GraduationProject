---
name: local-drawing-scripts
description: 'Modify or review local drawing scripts and MCP tooling. Use when changing gp_display_mcp_bridge.py, drawing payload rules, host preview flow, dev-cycle tooling, image conversion tools, or script-side animation constraints under Project/Script.'
user-invocable: true
disable-model-invocation: false
---

# Local Drawing Scripts

## Category

`本地绘图脚本`

## Scope

Use this skill for script-side and MCP tasks under `Project/Script/`:

- `Project/Script/mcp/gp_matrix/`
- `Project/Script/tools/`
- `Project/Script/media_tools/`

## Read only what is needed

- For MCP drawing tasks, prefer `Project/Script/mcp/gp_matrix/`.
- For build/flash/monitor workflow tasks, prefer `Project/Script/tools/`.
- Read `Project/Protocols/` only when script payload contracts depend on shared protocol fields.

## Module quick map

- `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py` — Canonical host-side drawing bridge for LLM tools and local preview/upload flows.
- `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md` — Tool input, allowed formats, animation constraints, and failure avoidance.
- `Project/Script/mcp/gp_matrix/gp_mcp_endpoint_client.py` — MCP bridge entry: MCP client, HTTP server, Debug WS, drawing tools.
- `Project/Script/media_tools/led_image_converter_gui.py` — Manual image/text→16x16 asset converter (Tkinter GUI).
- `Project/Script/tools/ws2812_auto_debug.py` — Unified AI + LED auto-debug workflow automation.

## Common host flow

- LLM / tool request → `gp_display_mcp_bridge.py`
- Bridge output → AI-side preview/upload interfaces (`matrix_pattern_result`, animation batches, or `matrix_action_result`)
- AI-side board/orchestrator → Bluetooth upload
- LED-side render is downstream; scripts should not assume direct LED-side packet injection

## Common read bundles

- `MCP drawing / animation rules` → `gp_matrix_drawing_mcp_usage.md` + `gp_display_mcp_bridge.py` + `Project/Protocols/gp_matrix_pattern_protocol.md`
- `Manual asset conversion` → `Project/Script/media_tools/led_image_converter_gui.py`
- `Build / monitor automation` → `Project/Script/tools/ws2812_auto_debug.py`

## Problem-solving workflow

For script, MCP, and tooling tasks:

1. Summarize the current script flow, boundaries, and entry points first.
2. State the operational risks, failure cases, and candidate fixes before editing.
3. Prefer the smallest feasible change that keeps the host flow bounded.
4. Validate one focused workflow slice before widening scope.
5. Sync docs and prompt/skill guidance when operational expectations or workflow rules change.

## Requirements

1. Keep LLM-facing tool names and argument names self-descriptive.
2. Keep payload and animation docs consistent with `Project/Protocols/`.
3. Host bridge tool namespace: `self.screen.matrix_16x16.*`. Local debug: `self.screen.matrix_16x16.local.*`. Do not mix.
4. Do not move script-specific rules into the top-level prompt; keep them in this category or matching script docs.
5. If script workflow changes, update `Project/Script/README.md` and nearest script doc.
6. When a display can use native effects, prefer `show_effect` / `matrix_action_result` over bitmap animation.
7. For subtitle text + scroll: prefer `show_scroll_subtitle`; use `draw_animation` only for explicit frame sequences.
8. Keep default auto-debug chain: Keil rebuild → delay 20s → STC serial monitor (COM15) → ESP-IDF build/flash/monitor.
9. Tool paths: Keil `S:\Embedded\Keil`, ESP-IDF `S:\Embedded\ESP\v5.4.3\esp-idf`.
