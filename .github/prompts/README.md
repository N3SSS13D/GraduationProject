# GraduationProject Prompt Catalog

## Current Structure Mapping

- App layer: `STC51/Project/ws2812_driver/Sources/app/`
- Mid layer: `STC51/Project/ws2812_driver/Sources/mid/`
- Driver layer: `STC51/Project/ws2812_driver/Sources/drv/`
- Shared headers and config: `STC51/Project/ws2812_driver/Sources/inc/`
- MCU glue / generated peripheral entry: `STC51/Project/ws2812_driver/Sources/*.c` and `STC51/Project/ws2812_driver/Sources/lib/`
- XiaoZhi integration assets: `External/xiaozhi-esp32/GP_Port/`

Preferred dependency direction:

- STC side: `app -> mid -> drv -> MCU glue`
- Cross-project bridge: XiaoZhi AI result -> `GP_Port` action/protocol layer -> AI8051U execution layer

## Current Repository Status

- The STC side already has a stable WS2812 scan/output path with PWM + DMA, 74HC595 row selection, and USB debug commands.
- The XiaoZhi snapshot already includes `GP_Port` protocol notes, an ESP32 driver skeleton, an AI8051U interface design, and MCP debug tooling.
- The next milestone is a complete voice-to-LED bridge over I2C and a custom protocol.

## Prompt Families

### Core Development

- [ws2812-led-system-dev.prompt.md](./ws2812-led-system-dev.prompt.md)
- [ws2812-led-system-dev.zh-CN.prompt.md](./ws2812-led-system-dev.zh-CN.prompt.md)

### Display Driver

- [ws2812-display-driver.prompt.md](./ws2812-display-driver.prompt.md)
- [ws2812-display-driver.zh-CN.prompt.md](./ws2812-display-driver.zh-CN.prompt.md)

### Animation Effects

- [ws2812-animation-effects.prompt.md](./ws2812-animation-effects.prompt.md)
- [ws2812-animation-effects.zh-CN.prompt.md](./ws2812-animation-effects.zh-CN.prompt.md)

### AI Control Bridge

- [ws2812-ai-control.prompt.md](./ws2812-ai-control.prompt.md)
- [ws2812-ai-control.zh-CN.prompt.md](./ws2812-ai-control.zh-CN.prompt.md)

### Code Review

- [ws2812-code-review.prompt.md](./ws2812-code-review.prompt.md)
- [ws2812-code-review.zh-CN.prompt.md](./ws2812-code-review.zh-CN.prompt.md)

## GP_Port Prompt Set

The `External/xiaozhi-esp32/GP_Port/` directory keeps the phase-by-phase prompts for the ESP32 <-> AI8051U bridge. They are aligned to the current next step: I2C custom protocol + voice-controlled LED display.

## Usage

1. Open Chat input and type `/`.
2. Choose the prompt that matches one narrow task.
3. Keep each iteration focused on a single driver, protocol, integration, or review step.
4. When the task touches voice control, also read `Doc/项目文档/project_status_summary_2026-04-12.md` first.
