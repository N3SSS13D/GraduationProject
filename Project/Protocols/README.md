# Bluetooth Communication Protocol

## Category

`蓝牙通信协议`

## Active files

- 共享协议头：`Project/Protocols/gp_led_matrix_protocol.h`
- 主协议说明：`Project/Protocols/gp_led_matrix_protocol_spec.md`
- 图案请求说明：`Project/Protocols/gp_matrix_pattern_protocol.md`

## Scope

本分类用于保存 `AI端` 与 `LED端` 共用的蓝牙通信协议定义、字段约束和请求契约。

## Prompt / Skill 入口

- Prompt：`.github/prompts/ws2812-bluetooth-protocol*.prompt.md`
- Skill：`.github/skills/bluetooth-protocol/SKILL.md`

两端业务实现请分别查看：

- `Project/xiaozhi-esp32/main/gp_port/`
- `Project/STC51/ws2812_driver/`

## Artifact quick map

- `gp_led_matrix_protocol.h`
  - 当前共享协议唯一源头：包头常量、命令字、负载结构、分片限制和动画上限都以此为准。
- `gp_led_matrix_protocol_spec.md`
  - 描述线上的包结构、命令流程、ACK/状态语义和推荐传输顺序。
- `gp_matrix_pattern_protocol.md`
  - 描述主机绘图请求、`bitmap_rows_hex` / `matrix_action_result` 表达方式，以及主机 -> `AI端` -> `LED端` 的契约边界。

## Current protocol flow

1. 主机绘图、原生效果命令或 `AI端` 本地动作生成协议负载
2. `AI端` 按 `gp_led_matrix_protocol.h` 拼包发送
3. `LED端` 解析相同字段并执行
4. 若协议语义变化，文档与两端实现必须同步更新

当前 `SetAction` 附加约定：当 `content=state` 且 `animation_flags` 携带 `GpMatrixLocalControlAction` 时，`LED端` 直接执行
离线本地方案动作（`next_pattern / show_text_scroll / show_clock / toggle_text_clock / next_effect / next_color`），不新增命令字，也不改变 `28` 字节动作负载长度。

## Consumer entry points

- `AI端` 发送侧：`Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
- `LED端` 接收侧：`Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- 主机绘图契约：`Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
- 主机 websocket 转发入口：`Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`
