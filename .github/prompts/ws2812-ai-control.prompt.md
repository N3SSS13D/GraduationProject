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

Current structure to use:
- App: `Sources/app/` for business decisions from ASR/AI results
- Mid: `Sources/mid/` for action normalization, arbitration, and reusable state logic
- Drv: `Sources/drv/` for display-driver invocation and timing-safe execution hooks
- Shared: `Sources/inc/` for common action definitions
- Peripheral glue: `Sources/*.c` and `Sources/lib/` for low-level access only

Do not do in this prompt:
- Large UI/host-side application work
- Rewriting scan driver timing internals
- Multi-feature AI platform design in one step

Execution requirements:
1. Inspect `Sources/app`, `Sources/mid`, `Sources/drv`, and `External/xiaozhi-esp32/GP_Port/` first.
2. Define a stable API between AI and display modules, preferably aligned with `voice_color_result` and `gp_led_matrix_protocol.h`.
3. Add strict bounds checks and invalid-command handling.
4. Keep deterministic display refresh regardless of command burst.
5. Document command examples and integration points.
6. Keep protocol parsing and arbitration in `mid/`, and keep App focused on decision flow.

Current phase emphasis:
- The next target is not a generic AI platform layer. It is a narrow bridge from XiaoZhi voice output to AI8051U-executable LED actions over I2C.
- Reuse the existing `GP_Port` protocol and interface assets before inventing new message shapes.

Output format:
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
