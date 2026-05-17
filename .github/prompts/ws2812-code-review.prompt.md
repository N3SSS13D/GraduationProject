---
name: WS2812 structured code review
description: "Review changes through the four project categories: LED, AI, protocol, and local scripts"
argument-hint: "Review target (commit, files, module, or feature description)"
agent: agent
model: "GPT-5 (copilot)"
---
Review the specified target across the four project categories.

Apply `.claude/skills/karpathy-guidelines.md` for review discipline.
Read `Doc/Instructions/project_structure.md` for category layout and common read bundles.

Priorities: functional bugs → timing risks → protocol consistency → buffer bounds → validation gaps.

Category entry points:
- `LED-side`: `app.c` → `gp_led_action.c` → `gp_led_matrix_ai8051u.c` → `ws2812_drv.c`
- `AI-side`: `board` → `ui/debug state` → `gp_led_matrix_esp32` → `transport`
- `Protocol`: `gp_led_matrix_protocol.h` + `*_spec.md` field alignment
- `Scripts`: host drawing targets AI-side preview/upload interfaces

## Output format

- `Findings`
- `Open questions / assumptions`
- `Change summary`
- `Test and validation gaps`
