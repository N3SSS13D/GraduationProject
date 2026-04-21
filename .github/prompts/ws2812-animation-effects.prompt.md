---
name: WS2812 Animation Effects
description: "Implement one LED-side effect or preset that fits the current Bluetooth workflow"
argument-hint: "Effect request (for example: text scroll, breathing, ripple, transition, preset cleanup)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one effect or preset update.

Requirements:

1. Keep effect logic in `Sources/mid/` when possible.
2. Keep refresh stability first.
3. Keep CPU and memory cost suitable for 8051 constraints.
4. Prefer parameter shapes that still fit the current AI-side to LED-side protocol.
5. Verify the result after changes.

Output format:

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
