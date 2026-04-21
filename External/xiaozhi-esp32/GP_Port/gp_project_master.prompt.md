# Project Master Prompt

## 中文
你正在维护 `AI端` 的自建扩展，目标板型固定为 `lichuang-dev`。当前主线只聚焦 `AI端 -> 经典蓝牙链路 -> LED端`。

核心路径：

- `AI端` 主工程：`External/xiaozhi-esp32/`
- 矩阵驱动：`GP_Port/gp_led_matrix_esp32.h/.cc`
- 蓝牙传输：`GP_Port/transport/`
- 调试界面：`GP_Port/ui/`
- 板级接入：`main/boards/lichuang-dev/`
- 问题说明：`Doc/项目文档/problem_tracking.md`
- 联调工具：`tools/ws2812_dev_cycle.ps1`

查看规则：

- 传输问题，只看 `GP_Port/transport/`、矩阵驱动和相关 board/build 文件。
- 界面问题，只看 `GP_Port/ui/` 和相关 board 文件。
- 不要默认展开无关上游模块。

工作要求：

1. 先读取 `d:\GraduationProject\.github\skills\karpathy-guidelines\SKILL.md`。
2. 中文命名统一使用 `AI端` 和 `LED端`。
3. 只保留当前主线所需信息；已解决问题统一写入 `Doc/项目文档/problem_tracking.md`。
4. 优先处理动作映射、蓝牙收发、协议一致性、调试工具接入和性能优化。
5. 保持 `GP_Port/transport/`、`GP_Port/ui/`、矩阵驱动三者职责清晰。
6. 修改后直接执行可用构建或联调验证。

输出要求：

1. 说明改动路径和验证状态。
2. 若有阻塞，明确写出限制和下一步建议。

## English
You are maintaining the custom `AI side` extension for the `lichuang-dev` board. The current branch focuses on `AI side -> classic Bluetooth transport -> LED side`.

Core paths:

- `External/xiaozhi-esp32/`
- `GP_Port/gp_led_matrix_esp32.h/.cc`
- `GP_Port/transport/`
- `GP_Port/ui/`
- `main/boards/lichuang-dev/`
- `Doc/项目文档/problem_tracking.md`
- `tools/ws2812_dev_cycle.ps1`

Rules:

1. Read `d:\GraduationProject\.github\skills\karpathy-guidelines\SKILL.md` first.
2. Keep changes focused on action mapping, Bluetooth transport, protocol consistency, tooling, and performance.
3. Move solved issues to the shared problem document instead of keeping them in prompts.
4. Run build or validation steps after code changes.