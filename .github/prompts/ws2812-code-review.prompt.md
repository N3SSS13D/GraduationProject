---
name: WS2812 Display Code Review
description: "Review project changes with focus on bugs, regressions, Bluetooth transport risks, and missing validation"
argument-hint: "Review target (commit, files, module, or feature description)"
agent: agent
model: "GPT-5 (copilot)"
---
Review the specified target.

Priorities:

1. Functional bugs and regressions
2. Timing risks in the LED-side execution path
3. Bluetooth transport and protocol consistency risks
4. Buffer bounds and memory safety issues
5. Missing validation gaps

If the review touches `External/xiaozhi-esp32/GP_Port/`, also verify:

- field consistency between AI-side action objects and LED-side protocol fields
- transport, ACK, and status-reporting coherence

Output format:

- `Findings`
- `Open questions / assumptions`
- `Change summary`
- `Test and validation gaps`
