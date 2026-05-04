# Prompt Catalog

当前 prompt 和 skill 统一按四类结构理解：

1. `LED端显示驱动`：`Project/STC51/`
2. `AI端接口调度`：`Project/xiaozhi-esp32/main/gp_port/`
3. `蓝牙通信协议`：`Project/Protocols/`
4. `本地绘图脚本`：`Project/Script/`

## 阅读规则

- 先用结构说明定位最窄的相关分类目录，不要默认扫描无关文件。
- prompt 中只补充结构定位和任务边界；分类细则保留在各自的 prompt / skill / 分类文档中。
- 当前有效说明文档优先看：
  - `Doc/Instructions/project_structure.md`
  - `Doc/Instructions/problem_tracking.md`
  - `Doc/Instructions/bt_version_hc05_uart2_architecture.md`
  - `Project/STC51/README.md`
  - `Project/xiaozhi-esp32/main/gp_port/README.md`
  - `Project/Protocols/README.md`
  - `Project/Script/README.md`

## 模块入口优先级

- `LED端显示驱动`
  - 先看 `Project/STC51/README.md`
  - 常见首读文件：`Sources/app/app.c`、`Sources/mid/gp_led_action.c`、`Sources/drv/gp_led_matrix_ai8051u.c`、`Sources/drv/ws2812_drv.c`
- `AI端接口调度`
  - 先看 `Project/xiaozhi-esp32/main/gp_port/README.md`
  - 常见首读文件：`gp_led_matrix_esp32.cc`、`transport/gp_led_matrix_transport.cc`、`ui/gp_debug_display.cc`、`boards/lichuang-dev/lichuang_dev_board.cc`
- `蓝牙通信协议`
  - 先看 `Project/Protocols/README.md`
  - 常见首读文件：`gp_led_matrix_protocol.h`、`gp_led_matrix_protocol_spec.md`、`gp_matrix_pattern_protocol.md`
- `本地绘图脚本`
  - 先看 `Project/Script/README.md`
  - 常见首读文件：`mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md`、`mcp/gp_matrix/gp_display_mcp_bridge.py`、`tools/ws2812_auto_debug.py`

## Prompts

### LED端显示驱动

- [ws2812-led-driver.zh-CN.prompt.md](./ws2812-led-driver.zh-CN.prompt.md)
- [ws2812-led-driver.prompt.md](./ws2812-led-driver.prompt.md)

聚焦 STC51 驱动主链路、协议接收执行、渲染与 Keil 构建验证。

### AI端接口调度

- [ws2812-ai-control.zh-CN.prompt.md](./ws2812-ai-control.zh-CN.prompt.md)
- [ws2812-ai-control.prompt.md](./ws2812-ai-control.prompt.md)

覆盖动作映射、蓝牙收发、触摸/预览、板级接入和主机绘图转发。

### 蓝牙通信协议

- [ws2812-bluetooth-protocol.zh-CN.prompt.md](./ws2812-bluetooth-protocol.zh-CN.prompt.md)
- [ws2812-bluetooth-protocol.prompt.md](./ws2812-bluetooth-protocol.prompt.md)

聚焦共享协议头、字段约束、ACK/状态语义和脚本契约一致性。

### 本地绘图脚本

- [ws2812-local-scripts.zh-CN.prompt.md](./ws2812-local-scripts.zh-CN.prompt.md)
- [ws2812-local-scripts.prompt.md](./ws2812-local-scripts.prompt.md)

聚焦 MCP 绘图桥、主机绘图数据、联调工具流程和协议契约对齐。

### 跨分类代码审查

- [ws2812-code-review.zh-CN.prompt.md](./ws2812-code-review.zh-CN.prompt.md)
- [ws2812-code-review.prompt.md](./ws2812-code-review.prompt.md)

按四类边界检查模块职责、链路一致性和验证缺口。

## Skills

- `karpathy-guidelines`：通用实现约束
- `ws2812-led-driver`：`LED端显示驱动`
- `bluetooth-protocol`：`蓝牙通信协议`
- `local-drawing-scripts`：`本地绘图脚本`
- `ai8051u-i2c-dma`：LED端 I2C DMA 专项参考

四类模块的入口结构统一为：`先分类 README -> 再模块 prompt -> 再专项 skill`。
