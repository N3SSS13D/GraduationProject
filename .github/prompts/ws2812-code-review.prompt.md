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
6. Layering violations and wrong file placement across App/Mdl-Mid/Drv/HAL
7. Missing tests or validation gaps

Layered directory mapping to validate:
- App: `Sources/app/`
- Mdl/Mid: `Sources/fml/`
- Drv: `Sources/lib/` and `Sources/output/`
- HAL: `Sources/hal/`

Execution requirements:
1. Read only the relevant files first; avoid broad unrelated commentary.
2. Report findings first, ordered by severity.
3. For each finding include: impact, evidence, and a concrete fix suggestion.
4. If no findings, explicitly state that and list residual risks.
5. Explicitly check dependency direction: App -> Mdl/Mid/Drv -> HAL.
6. Keep summary brief after findings.

Output format:
- "Findings" (severity-ordered)
- "Open questions / assumptions"
- "Change summary"
- "Test and validation gaps"
