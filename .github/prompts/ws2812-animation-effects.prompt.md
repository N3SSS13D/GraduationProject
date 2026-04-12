---
name: WS2812 Animation Effects
description: "Implement one display animation or visual effect on top of the existing WS2812 driver"
argument-hint: "Animation request (e.g., text scroll, breathing, ripple, waveform, transition)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one animation/effect feature based on the argument.

Assume base driver exists with row scanning (PWM + DMA + 74HC595 PMOS switching).

Scope (effects layer):
- framebuffer updates
- effect state machine
- timing policy compatible with scan refresh
- optional double-buffer integration

Current structure to use:
- App: `Sources/app/` for scene/app orchestration
- Mid: `Sources/mid/` for hardware-independent effect logic
- Drv: `Sources/drv/` for pixel push and scan output drivers
- Shared: `Sources/inc/`
- Peripheral glue: `Sources/*.c` and `Sources/lib/`

Do not do in this prompt:
- Rewriting low-level timing-critical driver internals unless required for correctness
- AI protocol integration
- Multi-feature bundles

Execution requirements:
1. Inspect project structure and reuse effect-related code patterns.
2. Keep refresh stability first; avoid visual tearing/flicker.
3. Support resolution-aware effect behavior (default 16x16).
4. Keep CPU and memory cost suitable for 8051 constraints.
5. Add concise usage notes for triggering the effect.
6. Place effect algorithms in `mid/` and keep hardware operations in `drv/` and the low-level glue layer.

Current phase emphasis:
- Prefer effect parameters that can later be carried over the XiaoZhi-to-AI8051U I2C bridge.
- Avoid effect designs that depend on direct high-rate AI message timing.

Output format:
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
