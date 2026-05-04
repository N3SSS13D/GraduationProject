---
name: 本地脚本与 MCP 改动（中文）
description: "在 Project/Script 下实现一个本地绘图脚本或 MCP 工具改动"
argument-hint: "脚本任务（例如：MCP 桥行为、payload 归一化、自动联调工具流程）"
agent: agent
model: "GPT-5 (copilot)"
---
仅实现一个 `本地绘图脚本` 任务。

## 文件结构定位

优先读取脚本资产，再按边界扩展：

1. `本地绘图脚本`
   - `Project/Script/mcp/gp_matrix/`
   - `Project/Script/tools/`
   - `Project/Script/media_tools/`
2. `蓝牙通信协议`（契约对齐）
   - `Project/Protocols/gp_matrix_pattern_protocol.md`
   - `Project/Protocols/gp_led_matrix_protocol.h`
3. `AI端接口调度`（集成边界）
   - `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`
   - `Project/xiaozhi-esp32/main/boards/lichuang-dev/lichuang_dev_board.cc`

## 执行要求

1. MCP 工具名与参数名必须自解释。
2. 脚本 payload 格式必须与当前协议文档一致。
3. 主机脚本链路必须收敛到 `AI端` 预览/上传接口。
4. 不要绕过 `AI端` 调度，直接假定 `LED端` 原始串口细节。
5. 若脚本流程变化，同步更新 `Project/Script/README.md` 与 `Doc/Instructions/problem_tracking.md`。
6. 默认将 `Project/Script/tools/ws2812_auto_debug.py` 作为联调自动化主入口。
7. 自动联调链路默认顺序必须保持：
   - 仅执行 `Project/STC51/ws2812_driver/ws2812_driver.uvproj` 的 Keil 编译，不做下载
   - 等待 `20s` 后打开 `AI8051U` 串口监视（默认 `COM15`，可参数覆盖）
   - 执行 `Project/xiaozhi-esp32` 的 ESP-IDF `build flash monitor`
8. 工具路径必须可配置且执行前先校验：
   - Keil 根目录：`S:\Embedded\Keil`
   - ESP-IDF 根目录：`S:\Embedded\ESP\v5.4.3\esp-idf`
9. 自动化入口变更后必须清理旧引用，不能保留失效脚本路径。

## 输出格式

- `Assumptions`
- `Plan`
- `Files changed`
- `Verification`
- `Operational notes`
