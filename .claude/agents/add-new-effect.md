---
name: add-new-effect
description: New display effect agent. Use when adding a new local LED animation effect or protocol-visible effect.
---

# Add New Display Effect Agent

## Trigger
When adding a new display effect (animation, pattern, text effect, color transition) to the LED matrix system.

## Decision Tree
1. **Local-only or remote?**
   - Local-only (offline button/key flow) → add to `draw_drv.c` / `local_display_scheme.c` only
   - Remote (AI-side can trigger) → extend `Project/Protocols/gp_led_matrix_protocol.h`

2. **If local-only:**
   - Add effect function to `Sources/mid/draw_drv.c`
   - Register in `local_display_scheme.c` with effect-command descriptor
   - Add button mapping in `key_ctrl.c` if user-triggerable
   - Update `Doc/Instructions/led_driver_tech_ref.md`

3. **If protocol-visible:**
   - Add command/enum to `Project/Protocols/gp_led_matrix_protocol.h`
   - Implement handler in `gp_led_matrix_ai8051u.c` + `gp_led_action.c`
   - Add AI-side trigger in `gp_led_matrix_esp32.cc`
   - Update protocol specs and MCP tools if needed
   - Update both AI-side and LED-side docs

## Key Files
- LED render: `Project/STC51/ws2812_driver/Sources/mid/draw_drv.c`
- Local scheme: `Project/STC51/ws2812_driver/Sources/mid/local_display_scheme.c`
- Protocol: `Project/Protocols/gp_led_matrix_protocol.h`
- AI-side: `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
