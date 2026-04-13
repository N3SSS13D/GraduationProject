# xiaozhi-esp32 移植与联调总结

## 1. 变更概述

本次工作已经从“引入 `xiaozhi-esp32` 参考快照”推进到“完成小智 ESP32 与 AI8051U/WS2812 的本地 I2C 联调主链”。当前目标不再只是参考上游架构，而是围绕毕业设计形成可运行、可验证、可继续扩展的本地显示闭环。

- 复用 ESP32 侧的小智 AI、MCP、显示调试链路设计。
- 为 STC AI8051U + WS2812 系统补充本地 I2C 协议、驱动、动作映射和联调诊断能力。

引入方式：

- 子目录快照位置：`External/xiaozhi-esp32/`
- 来源仓库提交：`3c7c4c7` (`Add MCP debug dot tooling and endpoint validation`)
- 引入方式为快照复制，不保留原仓库 `.git` 元数据。

## 2. 本轮已完成能力

围绕 `GP_Port` 目录，本轮已经完成并验证的重点如下：

### 2.1 16x16 LED 矩阵本地闭环

- 形成 ESP32 与 AI8051U 间的共享协议头：`GP_Port/gp_led_matrix_protocol.h`
- 形成 ESP32 侧矩阵驱动：`GP_Port/gp_led_matrix_esp32.h/.cc`
- 形成 AI8051U 接口层与执行层：`gp_led_matrix_ai8051u.h/.c`、`gp_led_action.h/.c`
- 已支持 `SetAction`、RGB332 帧、滚动字模、`Status/Error` 回包
- 已补充协议/架构/阶段提示词与设计文档

### 2.2 调试圆点显示链路

- 在 `lichuang-dev` 上集成 `GpDebugLcdDisplay`
- 圆点支持：
  - RGB888 颜色
  - `solid / gradient / pulse` 动画
  - 尺寸调节
- 当前显示效果已简化为“只显示圆点”
- 当前圆点中心位置：屏幕右侧 `1/3` 区域中点，即约 `x = 5/6 * width`，`y = 1/2 * height`
- 圆点状态变化会同步映射为 LED 矩阵动作，而不是停留在屏幕本地调试显示
- 未指定预设时，矩阵默认显示纯色满屏，不再只通过背景色表达同步状态

### 2.3 语音颜色与预设显示

- 已支持语音颜色识别并映射到矩阵 `SetAction`
- 已支持矩阵预设调用：`diamond`、`cross`、`python_demo`、`scroll_subtitle`
- 已支持“颜色 + 预设名”组合指令，例如“显示蓝色菱形”“显示绿色十字”
- 已支持独立背景色语音指令，例如“背景改成蓝色”“把背景色设为黑色”
- 当前矩阵不会再因设备待机、聆听、说话状态自动覆盖上一条显式图像
- 当前矩阵在指定预设后只显示该预设，不再沿用旧测试逻辑在三个图案之间自动轮播

### 2.4 链路状态与诊断

- 已将矩阵链路 I2C 速率固定到 `100kHz`
- 已关闭 ESP32 侧内部上拉，并要求 AI8051U `P2.3/P2.4` 以开漏、无内部上拉方式工作
- 已实现 AI8051U USB I2C 调试日志细分，便于区分“总线有活动”和“协议真实执行成功”
- 已实现小智屏幕左侧链路状态面板，可显示 `INIT`、`ONLINE`、`NO-REPLY`、`ERROR` 及最近一次命令摘要

### 2.5 MCP 工具扩展

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

1. 已经不只是参考，而是形成了本地 I2C 动作对象闭环。
2. `voice_color_analyze` / `voice_color_result` 已能稳定映射到 8051 侧显示动作。
3. STC 侧工作边界已经明确收敛为：协议收包、动作仲裁、矩阵输出执行与回包诊断。

## 4. 建议的下一步集成路线

建议按以下顺序推进 `GraduationProject`：

1. 继续完成硬件联调验证
   - 覆盖纯色、预设、滚动字幕、重复命令、断链和异常回复场景。

2. 补充链路恢复策略
   - 增加心跳超时、自动释放远程模式和重连策略。

3. 扩展自由文本与素材管理
   - 在当前内置字幕/预设稳定后，再考虑更高层自由文本下发。

4. 收敛最终文档与 prompt 边界
   - 将已验证行为持续同步到 `Doc/项目文档/` 和 `.github/prompts/`。

## 5. 验证状态

- `xiaozhi-esp32` 当前版本构建成功。
- 本地 I2C 动作对象、ACK 读回、链路状态面板与预设识别逻辑已接入。
- `GraduationProject` 已完成从“快照导入”到“本地显示联调闭环”的第一阶段落地。
