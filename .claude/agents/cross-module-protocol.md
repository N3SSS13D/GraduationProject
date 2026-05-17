---
name: cross-module-protocol
description: Cross-module protocol change agent. Use when modifying Project/Protocols/gp_led_matrix_protocol.h or protocol behavior that affects AI-side AND LED-side.
---

# Cross-Module Protocol Change Agent

## Trigger
When a task changes `Project/Protocols/gp_led_matrix_protocol.h` or protocol behavior.

## Workflow
1. Read `Project/Protocols/README.md` to identify all consumer files
2. Plan the change in the protocol header first
3. Update protocol spec docs (`gp_led_matrix_protocol_spec.md`, `gp_matrix_pattern_protocol.md`)
4. Trace AI-side consumer: `gp_led_matrix_esp32.cc`, `gp_led_matrix_transport.cc`
5. Trace LED-side consumer: `gp_led_matrix_ai8051u.c`, `gp_led_action.c`
6. Verify script contracts: `gp_matrix_drawing_mcp_usage.md`
7. Verify alignment: all consumers use same field sizes, types, semantics
8. Update affected skills/docs

## Key Files
- Protocol header: `Project/Protocols/gp_led_matrix_protocol.h`
- AI-side: `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
- LED-side: `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- Script: `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
