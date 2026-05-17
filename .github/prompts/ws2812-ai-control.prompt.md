---
name: AI Side Action Mapping
description: "Implement one AI-side feature that maps voice or debug results into LED-side actions"
argument-hint: "Feature request (for example: command parser, action mapping, priority policy, snapshot control)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one `AI-side` integration feature.

Apply `.claude/skills/karpathy-guidelines.md` for behavioral guidelines.
Read `Project/xiaozhi-esp32/main/gp_port/README.md` for module context, file targeting, execution flow, read bundles, and constraints.
Key files: `gp_led_matrix_esp32.cc`, `transport/`, `ui/`, `boards/lichuang-dev/lichuang_dev_board.cc`.

## Output format

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
