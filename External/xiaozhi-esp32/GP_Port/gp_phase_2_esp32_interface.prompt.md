# Phase 2 Prompt: AI Side Interface

## 中文
请完善 `AI端` 的动作映射与板级接入。优先修改：

- `GP_Port/gp_led_matrix_esp32.h/.cc`
- `main/boards/lichuang-dev/`
- 必要时修改 `GP_Port/ui/`

本阶段目标：

- 将 `voice_color_result` 或等价结果映射为稳定动作对象。
- 保持调试界面、动作下发和链路状态的一致性。
- 不扩展无关功能。

## English
Refine the AI-side action mapping and board integration with minimal changes, focusing on the matrix driver, board wiring, and related UI hooks.