# Phase 2 Prompt: ESP32 Interface

## 中文
请在不破坏现有小智架构的前提下完善 ESP32 侧 LED 矩阵驱动。优先扩展 `GP_Port/gp_led_matrix_esp32.h/.cc`，必要时只修改目标板文件和构建脚本。保持接口清晰、状态映射稳定，并补充必要的构建验证。

本阶段默认目标：

- 将设备状态或 `voice_color_result` 类动作对象映射成稳定的 LED 动作结构。
- 优先准备本地 I2C 自定义协议发送所需的分包、缓存与错误处理挂钩。
- 如果目标板带触摸屏，优先复用现有屏幕调试层，为 LED 图案和效果增加轻量触摸切换控件。
- 不要求本阶段接入外部 MCP 桥接，先保证本地动作对象能够稳定下发到 AI8051U。

## English
Extend the ESP32-side LED matrix driver with minimal impact to the existing XiaoZhi architecture. Prefer changes in `GP_Port/gp_led_matrix_esp32.h/.cc`, and only touch the target board or build files when integration requires it.

Default focus for this phase: map device state or `voice_color_result`-style action objects into stable LED actions, expose lightweight touch controls when the board supports it, and prepare the local I2C transport hooks before any MCP bridge integration.