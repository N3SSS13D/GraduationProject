# GraduationProject

## 项目定位

本仓库当前主线聚焦 `BT_Version`，目标是把 `AI端` 的动作结果通过经典蓝牙链路发送到 `LED端`，再由 `LED端` 执行 WS2812 显示。

- `AI端`：`External/xiaozhi-esp32/`
- `LED端`：`STC51/Project/ws2812_driver/`

当前默认关注的闭环是：

`AI端动作对象 -> AI端蓝牙传输 -> HC-05 -> LED端协议执行 -> WS2812 矩阵显示`

已解决问题与已知限制统一记录在：`Doc/项目文档/problem_tracking.md`

## 目录索引

### AI端

- 主工程：`External/xiaozhi-esp32/`
- 矩阵驱动：`External/xiaozhi-esp32/GP_Port/gp_led_matrix_esp32.h/.cc`
- 蓝牙传输：`External/xiaozhi-esp32/GP_Port/transport/`
- 调试界面：`External/xiaozhi-esp32/GP_Port/ui/`
- 板级接入：`External/xiaozhi-esp32/main/boards/lichuang-dev/`
- MCP / 截图工具：`External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`

### LED端构建

- 工程入口：`STC51/Project/ws2812_driver/ws2812_driver.uvproj`
- 应用层：`STC51/Project/ws2812_driver/Sources/app/`
- 中间层：`STC51/Project/ws2812_driver/Sources/mid/`
- 驱动层：`STC51/Project/ws2812_driver/Sources/drv/`
- 共享头文件：`STC51/Project/ws2812_driver/Sources/inc/`

### 联调与文档

- 联调脚本：`tools/ws2812_dev_cycle.py`
- Prompt 索引：`.github/prompts/README.md`
- GP_Port 总览：`External/xiaozhi-esp32/GP_Port/gp_port_project_overview.md`

## 当前主线

- AI端重点：动作对象映射、蓝牙传输、调试界面、截图工具、性能优化。
- LED端重点：UART2 收包、协议解析、ACK 回包、动作执行、渲染性能优化。
- 当前分支不再把已解决的自建 I2C 兼容问题作为 prompt 主体；历史问题统一查阅 `Doc/项目文档/problem_tracking.md`。

## 关键文档

- 问题说明与约束：`Doc/项目文档/problem_tracking.md`
- 蓝牙链路结构：`Doc/项目文档/bt_version_hc05_uart2_architecture.md`
- 蓝牙替代方案：`Doc/项目文档/bluetooth_replacement_plan.md`
- WS2812 当前实现：`Doc/项目文档/ws2812_driver_current_implementation.md`
- GP_Port 总览：`External/xiaozhi-esp32/GP_Port/gp_port_project_overview.md`
- 调试界面说明：`External/xiaozhi-esp32/GP_Port/gp_debug_feature_usage.md`
- MCP 工具说明：`External/xiaozhi-esp32/GP_Port/gp_mcp_tools.md`

## 构建与验证入口

### LED端

1. 打开 `STC51/Project/ws2812_driver/ws2812_driver.uvproj`
2. 执行 Keil rebuild
3. 通过 USB 调试命令或联调脚本验证日志与回包

### AI端构建

1. 打开 `External/xiaozhi-esp32/`
2. 执行 ESP-IDF 构建与监视
3. 需要截图或 MCP 联调时运行 `External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`

### 联调

- 首选自动化入口：`tools/ws2812_dev_cycle.py`
- VS Code 任务：`WS2812: Dev Cycle`、`WS2812: Dev Cycle Watch`

## 提交边界

默认不提交以下纯本地产物：

- `*.uvgui.*`
- `*.uvopt`
- `__pycache__/`
- `*.pyc`
- 临时截图、测试图片、临时日志
