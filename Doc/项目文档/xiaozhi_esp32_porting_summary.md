# xiaozhi-esp32 移植与对话总结

## 1. 变更概述

本次在 `GraduationProject` 中引入了 `xiaozhi-esp32` 的参考快照，用于后续开展以下两类工作：

- 参考 ESP32 侧的小智 AI、MCP、显示调试链路设计。
- 为 STC AI8051U + WS2812 系统补充与小智 AI 联动相关的接口设计与协议映射。

引入方式：

- 子目录快照位置：`External/xiaozhi-esp32/`
- 来源仓库提交：`3c7c4c7` (`Add MCP debug dot tooling and endpoint validation`)
- 引入方式为快照复制，不保留原仓库 `.git` 元数据。

## 2. 本轮 xiaozhi-esp32 已完成能力

围绕 `GP_Port` 目录，本轮已经完成并验证的重点如下：

### 2.1 16x16 LED 矩阵相关接口设计

- 形成 ESP32 与 AI8051U 间的共享协议头：`GP_Port/gp_led_matrix_protocol.h`
- 形成 ESP32 侧矩阵驱动：`GP_Port/gp_led_matrix_esp32.h/.cc`
- 形成 AI8051U 接口层设计头：`GP_Port/gp_led_matrix_ai8051u.h`
- 补充协议/架构/阶段提示词与设计文档

### 2.2 调试圆点显示链路

- 在 `lichuang-dev` 上集成 `GpDebugLcdDisplay`
- 圆点支持：
  - RGB888 颜色
  - `solid / gradient / pulse` 动画
  - 尺寸调节
- 当前显示效果已简化为“只显示圆点”
- 当前圆点中心位置：屏幕右侧 `1/3` 区域中点，即约 `x = 5/6 * width`，`y = 1/2 * height`

### 2.3 MCP 工具扩展

设备侧已新增两个 MCP 工具：

- `self.calculator.calculate`
- `self.screen.debug_dot.show`

其中：

- 计算器工具用于验证 MCP 调用链。
- 圆点工具用于远端直接控制调试显示状态。

### 2.4 官方 MCP 桥接地址验证

已新增测试脚本：

- `GP_Port/gp_mcp_endpoint_client.py`

实测结论：

- 可以成功连接官方 `wss://api.xiaozhi.me/mcp/?token=...` 地址。
- 该地址的交互方向不是“本地作为 client 主动 initialize”，而是“桥接端主动发 initialize，本地需作为 MCP server 响应”。
- 已完成：
  - `initialize` 握手
  - `notifications/initialized`
  - `tools/list`
  - `ping` 心跳响应
- 测试窗口内未收到真实 `tools/call`，因此链路可用性已验证，但真实业务调用仍依赖上游对话流。

## 3. 对 GraduationProject 的直接意义

这次移植为 `GraduationProject` 后续接入“小智 AI -> STC AI8051U -> WS2812 显示系统”提供了三个直接参考面：

1. MCP 工具模型参考
   - 可借鉴 `self.xxx.yyy` 的工具命名和参数建模方式。

2. 语音颜色意图到显示效果的映射参考
   - 可将 `voice_color_analyze` / `voice_color_result` 这套结构映射到 8051 侧显示命令。

3. AI8051U 接口层边界参考
   - 可把 STC 侧工作重点收敛为：协议收包、颜色/动画参数解析、矩阵输出动作执行。

## 4. 建议的下一步集成路线

建议按以下顺序推进 `GraduationProject`：

1. 在 `STC51/Project/ws2812_driver` 中增加“小智 AI 输入接口层”
   - 将 MCP/语音解析结果统一映射为内部显示动作对象。

2. 在显示驱动层补充 RGB/动画动作接口
   - 对齐 `solid / gradient / pulse / size` 等控制维度。

3. 将 `GP_Port/gp_led_matrix_ai8051u.h` 中的接口思想落到 STC51 工程源码
   - 对应 `Sources/app/`、`Sources/drv/`、`Sources/inc/` 等现有分层。

4. 视需要裁剪 `External/xiaozhi-esp32/GP_Port/` 中的文档和脚本
   - 将最终保留内容沉淀进 `Doc/项目文档/` 和 `.github/prompts/`。

## 5. 验证状态

- `xiaozhi-esp32` 当前版本已提交。
- `xiaozhi-esp32` 中 MCP 桥接地址测试通过握手验证。
- `xiaozhi-esp32` 固件构建成功。
- `GraduationProject` 已完成 `External/xiaozhi-esp32/` 子目录快照导入。
