# GraduationProject

## 当前结构

仓库已按 `Doc / Project / Demo` 三个一级目录重组：

- `Doc/`：保留原文档结构，集中存放论文、设计文档、问题跟踪与结构说明。
- `Project/`：集中存放可构建工程、脚本和调试产物。
- `Demo/`：集中存放实物图片、压缩包和演示说明。

详细结构说明见：`Doc/项目文档/project_file_structure.md`

## 核心路径

- `AI端`：`Project/xiaozhi-esp32/`
- `AI端` 矩阵驱动：`Project/xiaozhi-esp32/main/gp_port/`
- `LED端`：`Project/STC51/Project/ws2812_driver/`
- 联调脚本：`Project/Script/tools/ws2812_dev_cycle.py`
- MCP / 16x16 绘图桥接：`Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
- 调试产物：`Project/Debug/`
- Demo 资源：`Demo/Pic/`

## 当前主线

默认关注链路：

`AI端动作对象 -> AI端蓝牙传输 -> HC-05 -> LED端协议执行 -> WS2812 矩阵显示`

问题与限制统一记录在：`Doc/项目文档/problem_tracking.md`

## 构建与验证

### AI端

1. 打开 `Project/xiaozhi-esp32/`
2. 运行 ESP-IDF 构建或监视
3. 需要 MCP/截图/16x16 图案联调时，运行 `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`

### LED端

1. 打开 `Project/STC51/Project/ws2812_driver/ws2812_driver.uvproj`
2. 执行 Keil rebuild
3. 结合联调脚本与串口日志验证回包和显示结果

### 自动化联调

- VS Code 任务：`WS2812: Dev Cycle`、`WS2812: Dev Cycle Watch`
- 命令行入口：`Project/Script/tools/ws2812_dev_cycle.py`

## 说明

- `.github/`、`.vscode/`、`.venv/` 继续保留在仓库根目录，作为仓库级配置与开发环境入口。
- 原 `GP_Port/` 已拆分到 `Project/xiaozhi-esp32/main/gp_port/`、`Project/Script/mcp/gp_matrix/` 和 `Project/STC51/...`。
