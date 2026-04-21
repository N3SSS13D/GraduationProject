---
name: WS2812 Display Driver
description: "Implement one LED-side driver feature for the current UART2 and WS2812 execution path"
argument-hint: "Driver feature (for example: row scan timing, 74HC595 sequencing, UART2 packet intake hook, resolution mapping)"
agent: agent
model: "GPT-5 (copilot)"
---
Implement exactly one LED-side driver feature.

Focus paths:

- `STC51/Project/ws2812_driver/Sources/drv/`
- `STC51/Project/ws2812_driver/Sources/inc/`
- `STC51/Project/ws2812_driver/Sources/*.c`

Requirements:

1. Keep timing-critical paths deterministic.
2. Use fixed-size buffers and explicit bounds checks.
3. Keep dependency flow one-way: `drv -> peripheral glue`.
4. Stay compatible with the current UART2 packet execution path.
5. Rebuild the Keil project after source changes.

Output format:

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Next steps`
