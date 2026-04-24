# GraduationProject Prompt Catalog

## 当前主线

- 当前分支：`BT_Version`
- 当前目标：围绕 `AI端 -> 经典蓝牙链路 -> LED端` 做增量开发。
- 中文命名统一：`AI端` 指 `External/xiaozhi-esp32/`，`LED端` 指 `STC51/Project/ws2812_driver/`。
- 已解决问题与已知限制统一记录在：`Doc/项目文档/problem_tracking.md`

## 快速路径

- 总问题说明：`Doc/项目文档/problem_tracking.md`
- 蓝牙结构说明：`Doc/项目文档/bt_version_hc05_uart2_architecture.md`
- 蓝牙替代方案：`Doc/项目文档/bluetooth_replacement_plan.md`
- 联调脚本：`tools/ws2812_dev_cycle.py`
- AI端矩阵驱动：`External/xiaozhi-esp32/GP_Port/gp_led_matrix_esp32.h/.cc`
- AI端蓝牙传输：`External/xiaozhi-esp32/GP_Port/transport/`
- AI端调试界面：`External/xiaozhi-esp32/GP_Port/ui/`
- AI端板级接入：`External/xiaozhi-esp32/main/boards/lichuang-dev/`
- LED端协议执行：`STC51/Project/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- LED端头文件：`STC51/Project/ws2812_driver/Sources/inc/`

## 阅读规则

- AI端功能只看 `External/xiaozhi-esp32/GP_Port/` 和对应 board/build 文件。
- LED端功能只看 `STC51/Project/ws2812_driver/Sources/` 下相关目录。
- Prompt 或 README 维护只看 `.github/prompts/`、`External/xiaozhi-esp32/GP_Port/*.prompt.md` 和直接相关文档。
- 不要默认扫描无关第三方目录。

## Prompt 入口（精简后）

### AI端主开发入口

- [ws2812-ai-control.zh-CN.prompt.md](./ws2812-ai-control.zh-CN.prompt.md)
- [ws2812-ai-control.prompt.md](./ws2812-ai-control.prompt.md)

### 代码审查入口

- [ws2812-code-review.zh-CN.prompt.md](./ws2812-code-review.zh-CN.prompt.md)
- [ws2812-code-review.prompt.md](./ws2812-code-review.prompt.md)

说明：

- 其余阶段性或细分 prompt 已并入上述主入口，避免重复维护。
- `GP_Port/` 内的阶段 prompt 已下线，相关规则统一在主 prompt 与关键说明文档中维护。

## 使用要求

1. 先选一个最窄的任务入口，不要一次覆盖架构、协议、驱动和验证。
2. 已解决问题不要写回 prompt，统一写入 `Doc/项目文档/problem_tracking.md`。
3. 改动前先给出假设和验证标准；改动后直接执行可用验证。
