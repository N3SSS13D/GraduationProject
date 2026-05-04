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

- `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
  - Canonical host-side drawing bridge used by LLM-facing tools and local preview/upload flows.
- `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - Current usage contract for tool inputs, allowed formats, animation constraints, and failure avoidance.
- `Project/Script/mcp/gp_matrix/gp_mcp_endpoint_client.py`
  - Compatibility launcher for the MCP bridge entry.
- `Project/Script/media_tools/led_image_converter_gui.py`
  - Manual image/text conversion helper for 16x16 assets and firmware-friendly exports.
- `Project/Script/tools/ws2812_auto_debug.py`
  - Unified AI + LED auto-debug workflow automation.

## Common host flow

- LLM / tool request -> `gp_display_mcp_bridge.py`
- bridge output -> AI-side preview/upload interfaces
- AI-side board/orchestrator -> Bluetooth upload
- LED-side render is downstream; scripts should not assume direct LED-side packet injection

## Common read bundles

- `MCP drawing / animation rules`
  - `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
  - `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
  - `Project/Protocols/gp_matrix_pattern_protocol.md`
- `Manual asset conversion`
  - `Project/Script/media_tools/led_image_converter_gui.py`
- `Build / monitor automation`
  - `Project/Script/tools/ws2812_auto_debug.py`
  - `Project/Script/tools/ws2812_auto_debug.md`

## Requirements

1. Keep LLM-facing tool names and argument names self-descriptive.
2. Keep payload and animation docs consistent with the active protocol docs under `Project/Protocols/`.
3. Do not move script-specific rules into the top-level prompt; keep them in this category or the matching script docs.
4. If script workflow changes affect current usage guidance, update `Project/Script/README.md` and the nearest script
   doc.
5. Keep the default auto-debug chain strictly ordered as:
  - Keil rebuild only for `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
  - delay `20s`, then open AI8051U serial monitor (`COM15` default)
  - ESP-IDF `build flash monitor` for `Project/xiaozhi-esp32`
6. Validate tool roots before execution and expose configurable overrides:
  - `S:\Embedded\Keil`
  - `S:\Embedded\ESP\v5.4.3\esp-idf`
7. Remove legacy script references whenever automation entry changes to avoid dead docs and stale task bindings.
