# GraduationProject Current Structure

## 分类规则

本仓库所有当前有效的 prompt、skill、说明文档和代码路径统一按以下四类理解：

1. `LED端显示驱动`
   - 根路径：`Project/STC51/`
   - 目标：AI8051U + WS2812 矩阵驱动、渲染、Keil 工程
   - 示例目录：`Project/STC51/Examples/`
2. `AI端接口调度`
   - 根路径：`Project/xiaozhi-esp32/main/gp_port/`
   - 目标：语音结果映射、蓝牙传输、调试界面、板级接入
3. `蓝牙通信协议`
   - 根路径：`Project/Protocols/`
   - 目标：共享协议头、协议规范、帧传输与请求契约
4. `本地绘图脚本`
   - 根路径：`Project/Script/`
   - 目标：MCP 桥接、本地绘图脚本、联调工具

## 当前有效文档规则

- 当前方案文档只放在 `Doc/Instructions/`、`Project/STC51/`、`Project/Protocols/`、`Project/Script/`、`Project/xiaozhi-esp32/main/gp_port/` 的对应分类位置。
- 历史方案统一放入 `Doc/History/`。
- `Project/xiaozhi-esp32/` 下原仓库自带文档不纳入本分类规则；仅本仓库新增的 `gp_port` 相关文档参与本规则。

## 常用入口

- `LED端 Keil 工程`：`Project/STC51/ws2812_driver/ws2812_driver.uvproj`
- `AI端矩阵扩展`：`Project/xiaozhi-esp32/main/gp_port/`
- `共享协议头`：`Project/Protocols/gp_led_matrix_protocol.h`
- `协议说明`：`Project/Protocols/gp_led_matrix_protocol_spec.md`
- `绘图/MCP 说明`：`Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`
- `联调脚本说明`：`Project/Script/tools/ws2812_auto_debug.md`

## Prompt / Skill 入口矩阵

- `LED端显示驱动`
  - Prompt: `.github/prompts/ws2812-led-driver*.prompt.md`
  - Skill: `.github/skills/ws2812-led-driver/SKILL.md`

- `AI端接口调度`
  - Prompt: `.github/prompts/ws2812-ai-control*.prompt.md`
  - Skill: `.github/skills/karpathy-guidelines/SKILL.md`

- `蓝牙通信协议`
  - Prompt: `.github/prompts/ws2812-bluetooth-protocol*.prompt.md`
  - Skill: `.github/skills/bluetooth-protocol/SKILL.md`

- `本地绘图脚本`
  - Prompt: `.github/prompts/ws2812-local-scripts*.prompt.md`
  - Skill: `.github/skills/local-drawing-scripts/SKILL.md`

## 调用检查基线

- Prompt frontmatter 最少字段：`name / description / argument-hint / agent / model`
- Skill frontmatter 最少字段：`name / description`
- Prompt/Skill 中引用的路径必须在当前仓库可达
- 中英文入口需成对维护，避免分类语义漂移

## 模块级阅读入口

### 1. LED端显示驱动

- 入口文档：`Project/STC51/README.md`
- 主模块：
  - `Sources/app/app.c`：初始化与主循环
  - `Sources/mid/gp_led_action.c`：远程动作/帧/动画执行
  - `Sources/mid/draw_drv.c`：本地渲染与离线动画
  - `Sources/mid/offline_pattern.c`：本地离线图案资源与像素查询
  - `Sources/drv/gp_led_matrix_ai8051u.c`：协议收包、分发、ACK
  - `Sources/drv/ws2812_drv.c`：底层扫描输出
- 常见首读组合：
  - 收包/ACK：`gp_led_matrix_ai8051u.c + gp_led_action.c + gp_led_matrix_protocol.h`
  - 显示异常：`app.c + draw_drv.c + ws2812_drv.c`

### 2. AI端接口调度

- 入口文档：`Project/xiaozhi-esp32/main/gp_port/README.md`
- 主模块：
  - `gp_led_matrix_esp32.cc`：动作对象到协议发送
  - `transport/gp_led_matrix_transport.cc`：后台 UART 收发
  - `ui/gp_debug_display.cc`：触摸调试与预览
  - `boards/lichuang-dev/lichuang_dev_board.cc`：板级接入、websocket/MCP 转发
- 常见首读组合：
  - 发包/ACK：`gp_led_matrix_esp32.cc + gp_led_matrix_transport.cc + gp_led_matrix_protocol.h`
  - 预览/触摸：`gp_debug_display.cc + lichuang_dev_board.cc`

### 3. 蓝牙通信协议

- 入口文档：`Project/Protocols/README.md`
- 主模块：
  - `gp_led_matrix_protocol.h`：共享字段与常量
  - `gp_led_matrix_protocol_spec.md`：包结构与时序
  - `gp_matrix_pattern_protocol.md`：主机绘图请求契约
- 常见首读组合：
  - 协议改动：`gp_led_matrix_protocol.h + gp_led_matrix_protocol_spec.md`
  - 绘图契约：`gp_matrix_pattern_protocol.md + gp_matrix_drawing_mcp_usage.md`

### 4. 本地绘图脚本

- 入口文档：`Project/Script/README.md`
- 主模块：
  - `mcp/gp_matrix/gp_display_mcp_bridge.py`：主机绘图桥
  - `mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`：使用契约
  - `media_tools/led_image_converter_gui.py`：素材转换
  - `tools/ws2812_auto_debug.py`：联调自动化
- 常见首读组合：
  - 绘图/MCP：`gp_matrix_drawing_mcp_usage.md + gp_display_mcp_bridge.py + gp_matrix_pattern_protocol.md`
  - 联调：`ws2812_auto_debug.py + ws2812_auto_debug.md`
