---
name: WS2812 Display Driver
description: "Implement one low-level display driver feature for STC AI8051U WS2812 multiplex scanning"
argument-hint: "Driver feature (e.g., row scan timing fix, 74HC595 update sequence, configurable resolution mapping)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one driver-layer feature based on the argument.

Hardware constraints:
- STC AI8051U + PWM + DMA for WS2812 signaling
- Two cascaded 74HC595 control 16 PMOS high-side switches
- Odd/even rows share two PWM lines
- Two adjacent rows are scanned together with complementary fade transition

Scope (driver only):
- io/hal timing setup
- row switch control by 74HC595
- PMOS power gating sequence
- scan scheduler hook points (not full animation logic)
- resolution-aware row/column mapping in driver layer

Layered directory structure (placement requirement):
- App: `Sources/app/` (do not place driver code here)
- Mdl/Mid: `Sources/fml/` (hardware-independent utility/protocol only)
- Drv: `Sources/lib/` and `Sources/output/` for display/output driver modules
- HAL: `Sources/hal/` (MCU register/vendor-lib access)

Do not do in this prompt:
- Complex animation library design
- AI command parsing
- Broad codebase refactors

Execution requirements:
1. Inspect existing code first: [STC51/Project/ws2812_driver/Sources](../../STC51/Project/ws2812_driver/Sources)
2. Reuse current architecture and naming style.
3. Keep deterministic timing in critical path.
4. Prefer fixed-size buffers and compile-time configuration.
5. Preserve default 16x16 behavior while supporting configurable resolution.
6. Keep driver-to-HAL dependency one-way; avoid app logic in Drv files.

Output format:
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"
- "Next steps"
