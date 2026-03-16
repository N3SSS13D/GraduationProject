# WS2812 Prompt Catalog

## Layered Directory Mapping
- App layer (application/business logic): `STC51/Project/ws2812_driver/Sources/app/`
- Mdl/Mid layer (hardware-independent algorithms/protocols): `STC51/Project/ws2812_driver/Sources/fml/`
- Drv layer (device/peripheral drivers): `STC51/Project/ws2812_driver/Sources/lib/` and `STC51/Project/ws2812_driver/Sources/output/`
- HAL layer (MCU register/vendor library access): `STC51/Project/ws2812_driver/Sources/hal/`

Dependency direction to keep: App -> Mdl/Mid and Drv -> HAL

## Core (General)
- [ws2812-led-system-dev.prompt.md](./ws2812-led-system-dev.prompt.md) (EN)
- [ws2812-led-system-dev.zh-CN.prompt.md](./ws2812-led-system-dev.zh-CN.prompt.md) (ZH)

## 1) Display Driver
- [ws2812-display-driver.prompt.md](./ws2812-display-driver.prompt.md) (EN)
- [ws2812-display-driver.zh-CN.prompt.md](./ws2812-display-driver.zh-CN.prompt.md) (ZH)

## 2) Animation Effects
- [ws2812-animation-effects.prompt.md](./ws2812-animation-effects.prompt.md) (EN)
- [ws2812-animation-effects.zh-CN.prompt.md](./ws2812-animation-effects.zh-CN.prompt.md) (ZH)

## 3) AI Control
- [ws2812-ai-control.prompt.md](./ws2812-ai-control.prompt.md) (EN)
- [ws2812-ai-control.zh-CN.prompt.md](./ws2812-ai-control.zh-CN.prompt.md) (ZH)

## 4) Code Review
- [ws2812-code-review.prompt.md](./ws2812-code-review.prompt.md) (EN)
- [ws2812-code-review.zh-CN.prompt.md](./ws2812-code-review.zh-CN.prompt.md) (ZH)

## Usage
1. Open Chat input and type `/`.
2. Select one prompt by name.
3. Enter one focused argument (single task).
4. Run and iterate in small increments.
