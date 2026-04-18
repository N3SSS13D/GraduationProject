# GraduationProject

## 项目概述

本仓库用于毕业设计开发，当前已经形成稳定的 I2C 联调版本，核心闭环为：

`小智语音/调试状态 -> ESP32 动作映射 -> 本地 I2C 自定义协议 -> AI8051U 接收执行 -> WS2812 LED 矩阵显示`

当前主线由两部分组成：

1. `STC51/Project/ws2812_driver/`
   - AI8051U 侧 WS2812 复用扫描显示系统。
   - 已具备稳定的 PWM + DMA、74HC595 行选、PMOS 高侧切换、USB 调试和 I2C 从机链路。
2. `External/xiaozhi-esp32/`
   - 小智 ESP32 参考工程及本项目扩展。
   - 已完成 `GP_Port/` 下的 I2C 协议、矩阵驱动、调试界面、截图工具和联调脚本集成。

## 当前稳定版能力

### STC51 / AI8051U / WS2812 侧

- 已稳定运行 PWM + DMA 双通道输出。
- 已完成 74HC595 + PMOS 行扫描、尾波复位和节拍调度收口。
- 已支持 `normal_pair` 与 `legacy_shift` 两类扫描/发送模式。
- 已支持 USB/串口调试命令：颜色、图案、间隔、渲染模式切换。
- 已支持 AI8051U I2C 从机、协议解析、动作执行、ACK/错误状态回包。
- 当前默认采用 START/STOP 保留在 ISR、RX/TX 双向 DMA 负责包数据搬运的 I2C 后端。

### XiaoZhi / ESP32 / GP_Port 侧

- 已接通共享协议头 `gp_led_matrix_protocol.h`。
- 已完成 ESP32 侧矩阵驱动 `gp_led_matrix_esp32.h/.cc`。
- 已完成 AI8051U 接口层 `gp_led_matrix_ai8051u.h/.c` 和动作执行层对接。
- 已支持语音颜色结果与调试圆点状态同步到 LED 矩阵。
- 已支持矩阵预设：`diamond`、`cross`、`JLU_emblem`、`scroll_subtitle`。
- 已支持“未指定预设时默认纯色满屏”和“仅在显式图像更新时通信”策略。
- 已支持图案背景色独立控制，选中预设后不再轮播旧测试图案。
- 已支持稳定版调试菜单：`DBG` 入口、固定标题栏 `Back / Debug Menu / S`、单页触摸控制区、圆点预览区、链路状态区和摘要信息区。
- 已支持设备侧截图：按下 `S` 冻结当前帧、后台编码 PNG、通过本地 HTTP `/snapshot` 上传。
- 已支持主机侧通过 `/control/snapshot` 触发设备执行截图。
- 已支持本地 Python MCP 联调脚本输出桥接状态、HTTP 状态和最近一次保存结果。

## 仓库结构

```text
GraduationProject/
|-- README.md
|-- Doc/
|   `-- 项目文档/
|       |-- project_status_summary_2026-04-12.md
|       |-- usb_play_v2_guide.md
|       |-- ws2812_driver_current_implementation.md
|       |-- xiaozhi_ai8051u_i2c_interface_protocol.md
|       `-- xiaozhi_esp32_porting_summary.md
|-- External/
|   `-- xiaozhi-esp32/
|       |-- main/                      # 小智应用、板级与设备抽象
|       `-- GP_Port/                   # 本项目扩展协议、驱动、联调脚本和说明
|-- STC51/
|   `-- Project/
|       `-- ws2812_driver/
|           |-- Sources/
|           |   |-- app/               # 扫描调度与应用层流程
|           |   |-- mid/               # 渲染、动画、按键控制
|           |   |-- drv/               # WS2812 / 74HC595 / I2C 驱动
|           |   |-- inc/               # 共享头文件与配置
|           |   |-- timer.c            # 定时器与节拍控制
|           |   |-- usblib.c           # USB 命令入口
|           |   `-- main.c             # MCU 入口与初始化
|           `-- ws2812_driver.uvproj
`-- .github/
    `-- prompts/                       # 项目开发 prompt 集合
```

## 关键文档入口

- 项目阶段总览：[Doc/项目文档/project_status_summary_2026-04-12.md](Doc/项目文档/project_status_summary_2026-04-12.md)
- WS2812 驱动实现说明：[Doc/项目文档/ws2812_driver_current_implementation.md](Doc/项目文档/ws2812_driver_current_implementation.md)
- 小智与 AI8051U I2C 协议说明：[Doc/项目文档/xiaozhi_ai8051u_i2c_interface_protocol.md](Doc/项目文档/xiaozhi_ai8051u_i2c_interface_protocol.md)
- 小智移植与联调总结：[Doc/项目文档/xiaozhi_esp32_porting_summary.md](Doc/项目文档/xiaozhi_esp32_porting_summary.md)
- GP_Port 总览：[External/xiaozhi-esp32/GP_Port/gp_port_project_overview.md](External/xiaozhi-esp32/GP_Port/gp_port_project_overview.md)
- 调试界面与截图说明：[External/xiaozhi-esp32/GP_Port/gp_debug_feature_usage.md](External/xiaozhi-esp32/GP_Port/gp_debug_feature_usage.md)
- MCP 工具与本地桥接说明：[External/xiaozhi-esp32/GP_Port/gp_mcp_tools.md](External/xiaozhi-esp32/GP_Port/gp_mcp_tools.md)

## 构建与验证

### STC51 工程

1. 使用 Keil 打开 `STC51/Project/ws2812_driver/ws2812_driver.uvproj`。
2. 执行编译，必要时通过 STC ISP 下载固件。
3. 通过 USB 或串口命令验证颜色、图案、间隔和渲染模式切换。

### ESP32 工程

1. 使用 ESP-IDF 插件打开 `External/xiaozhi-esp32/`。
2. 选择 `lichuang-dev` 并执行构建。
3. 运行 `GP_Port/gp_mcp_endpoint_client.py`，联调 `/snapshot`、`/control/snapshot` 和设备侧 MCP 工具。

## 提交边界

默认不提交以下纯本地产物：

- `*.uvgui.*`
- `*.uvopt`
- `__pycache__/`
- `*.pyc`
- 临时导出截图、测试图片和临时日志


