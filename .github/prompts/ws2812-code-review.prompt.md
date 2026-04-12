---
name: WS2812 Display Code Review
description: "Review WS2812 project changes with focus on bugs, regressions, timing risks, and missing tests"
argument-hint: "Review target (commit, files, module, or feature description)"
agent: agent
model: "GPT-5 (copilot)"
---
Perform a code review for the specified target in this WS2812 project.

Review priorities (highest first):
1. Functional bugs and regressions
2. Timing risks in PWM + DMA + row scanning path
3. Power-switch sequencing risks (74HC595 + PMOS)
4. Buffer bounds and memory safety issues
5. Concurrency/scheduling issues in mini-OS cooperative tasks
6. Layering violations and wrong file placement across `app/mid/drv/peripheral glue`
7. Missing tests or validation gaps

Layered directory mapping to validate:
- App: `Sources/app/`
- Mid: `Sources/mid/`
- Drv: `Sources/drv/`
- Shared: `Sources/inc/`
- Peripheral glue: `Sources/*.c` and `Sources/lib/`

Execution requirements:
1. Read only the relevant files first; avoid broad unrelated commentary.
2. Report findings first, ordered by severity.
3. For each finding include: impact, evidence, and a concrete fix suggestion.
4. If no findings, explicitly state that and list residual risks.
5. Explicitly check dependency direction: `app -> mid -> drv -> peripheral glue/vendor support`.
6. Keep summary brief after findings.

If the review target touches `External/xiaozhi-esp32/GP_Port/`, also verify:
- protocol compatibility between the XiaoZhi bridge layer and the STC-side action layer,
- semantic consistency across `voice_color_result`, protocol fields, and LED action parameters.

Output format:
- "Findings" (severity-ordered)
- "Open questions / assumptions"
- "Change summary"
- "Test and validation gaps"
