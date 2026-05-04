# GraduationProject

## 当前结构

仓库当前按 `Doc / Project / Demo` 三个一级目录组织，其中活跃开发内容统一按四类归档：

1. `LED端显示驱动`：`Project/STC51/`
2. `AI端接口调度`：`Project/xiaozhi-esp32/main/gp_port/`
3. `蓝牙通信协议`：`Project/Protocols/`
4. `本地绘图脚本`：`Project/Script/`

当前有效说明文档入口见：

- `Doc/Instructions/README.md`
- `Doc/Instructions/project_structure.md`

## 当前主线

默认关注链路：

`AI端动作对象 -> 蓝牙协议 -> HC-05/UART -> LED端协议执行 -> WS2812 矩阵显示`

## 常用入口

- `LED端`：`Project/STC51/ws2812_driver/ws2812_driver.uvproj`
- `AI端`：`Project/xiaozhi-esp32/`
- 协议头：`Project/Protocols/gp_led_matrix_protocol.h`
- 联调脚本：`Project/Script/tools/ws2812_auto_debug.py`
- MCP 绘图桥接：`Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`

## 说明

- `Doc/Instructions/` 只保留当前方案文档。
- `Doc/History/` 只保留历史资料，不作为当前实现依据。
- `.github/` 保留仓库级 prompt、skill 和 Copilot 指令入口。
