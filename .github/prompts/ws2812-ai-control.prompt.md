---
name: WS2812 AI Control Bridge
description: "Implement one AI-control integration feature that maps XiaoZhi commands to display actions"
argument-hint: "AI feature (e.g., command parser, action mapping, priority policy, fail-safe behavior)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one AI-control feature based on the argument.

Goal:
- Build a clean interface layer between XiaoZhi AI inputs and display actions.
- Keep display timing independent from unpredictable AI message timing.

Scope (AI interface layer):
- input command format and parser
- command-to-action mapping
- validation and fallback behavior
- queue/policy for command arbitration with local effects

Layered directory structure (placement requirement):
- App: `Sources/app/` for business decisions from ASR/AI results
- Mdl/Mid: `Sources/fml/` for protocol parsing and command normalization
- Drv: `Sources/lib/` and `Sources/output/` for display driver invocation
- HAL: `Sources/hal/` for hardware register/vendor-lib access only

Do not do in this prompt:
- Large UI/host-side application work
- Rewriting scan driver timing internals
- Multi-feature AI platform design in one step

Execution requirements:
1. Inspect existing app/fml/lib/output/hal layering first.
2. Define a stable API between AI and display modules.
3. Add strict bounds checks and invalid-command handling.
4. Keep deterministic display refresh regardless of command burst.
5. Document command examples and integration points.
6. Keep protocol parsing in Mdl/Mid and keep App focused on decision flow.

Output format:
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
