---
name: Local Script and MCP Change
description: "Implement one local drawing script or MCP tooling change under Project/Script"
argument-hint: "Script task (for example: MCP bridge behavior, payload normalization, auto-debug tooling)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one `Local drawing scripts` task.

Apply `.claude/skills/local-drawing-scripts.md` for module context, file targeting, host flow, read bundles, and constraints.
Key files: `gp_display_mcp_bridge.py`, `gp_mcp_endpoint_client.py`, `gp_matrix_drawing_mcp_usage.md`, `ws2812_auto_debug.py`.

## Output format

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Operational notes`
