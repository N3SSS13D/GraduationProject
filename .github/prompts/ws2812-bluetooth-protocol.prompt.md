---
name: Bluetooth Protocol Change
description: "Implement one protocol-level change under Project/Protocols"
argument-hint: "Protocol task (for example: command field update, ACK flow, chunk size, drawing contract)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one `Bluetooth communication protocol` task.

Apply `.claude/skills/bluetooth-protocol.md` for module context, file targeting, protocol flow, read bundles, and constraints.
Key files: `Project/Protocols/gp_led_matrix_protocol.h` (single source of truth), `*_spec.md`, `*_pattern_protocol.md`.

## Output format

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Compatibility notes`
