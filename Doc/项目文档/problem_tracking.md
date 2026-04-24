# 问题说明与约束记录

## 命名约定

- `AI端`：指 `External/xiaozhi-esp32/` 下的小智侧自建扩展与板级接入代码。
- `LED端`：指 `STC51/Project/ws2812_driver/` 下的 AI8051U、WS2812、协议执行与显示逻辑。

## 已解决问题

### 2026-04-08 PWM + DMA 通道组偶发错乱

- 现象：双通道 PWM + DMA 输出下，特定通道组偶发错乱。
- 根因：发送长度配对、DMA 源地址对齐、尾部边界保护和异常恢复不足同时触发。
- 处理结果：
  - 强制偶数长度发送。
  - 固定 DMA 缓冲对齐地址。
  - 增加 CH1/CH2 尾部零保护。
  - 增加 DMA 超时恢复。
  - 固定奇偶行与通道映射。
- 相关路径：
  - `STC51/Project/ws2812_driver/Sources/drv/`
  - `STC51/Project/ws2812_driver/Sources/mid/`

### 2026-04-12 GP_Port 快照与调试菜单整理

- 现象：AI端参考资产、调试圆点和 MCP 联调脚本分散，难以复用。
- 处理结果：
  - 导入 `External/xiaozhi-esp32/` 快照作为 AI端参考工程。
  - 将协议、矩阵驱动、调试工具和阶段 prompt 统一收拢到 `External/xiaozhi-esp32/GP_Port/`。
  - AI端调试菜单收敛为 `DBG` 入口和二级调试界面。
- 相关路径：
  - `External/xiaozhi-esp32/GP_Port/`
  - `External/xiaozhi-esp32/main/boards/lichuang-dev/`

### 2026-04-19 BT_Version 传输主线切换

- 现象：LED端仍残留自建 I2C 兼容接口，AI端自建传输与界面文件未按职责归类。
- 处理结果：
  - LED端当前主链路统一为 `UART2(P4.2/P4.3) + HC-05`。
  - 删除 `BT_Version` 上不再使用的自建 I2C 兼容壳接口。
  - AI端自建蓝牙传输整理到 `External/xiaozhi-esp32/GP_Port/transport/`。
  - AI端自建调试界面整理到 `External/xiaozhi-esp32/GP_Port/ui/`。
- 相关路径：
  - `STC51/Project/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
  - `External/xiaozhi-esp32/GP_Port/transport/`
  - `External/xiaozhi-esp32/GP_Port/ui/`

### 2026-04-24 MCP 动画输入多次失败与约束收敛

- 现象：在 `draw_animation` 调用中，`LLM` 多次把“单帧 16 行 token”误传成“16 帧列表”，并反复尝试 `yield_frame/time.sleep/import` 这类不受支持的时序实现，导致参数校验连续失败，最终退化为文字播放。
- 根因：
  - 动画输入契约对 `LLM` 不够直观，`bitmap_rows_hex_list` 的“每项应为完整帧”语义容易与“16 行 row token”混淆。
  - 文档对动画时序控制边界不够强，未明确禁止在绘图语句中自行实现播放时序。
- 处理结果：
  - MCP 桥接新增动画输入归一化层，支持 `frames[]` 模式（每帧三选一：`bitmap_rows_hex`、`bitmap_rows`、`python_source/eval_source`）。
  - 保留 `bitmap_rows_hex_list` 后向兼容，并新增“16 行 token 视为单帧”的容错归一化，降低历史高频误用失败率。
  - 动画 schema 与说明文档补充详细约束、禁用实现方式、可复制示例和调用顺序。
- 相关路径：
  - `External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`
  - `External/xiaozhi-esp32/GP_Port/gp_matrix_drawing_mcp_usage.md`
  - `External/xiaozhi-esp32/GP_Port/gp_matrix_pattern_protocol.md`
  - `.github/prompts/ws2812-ai-control.prompt.md`
  - `.github/prompts/ws2812-ai-control.zh-CN.prompt.md`

## 当前已知约束

### AI端经典蓝牙约束

- 当前 AI端目标板为 `lichuang-dev`，主要工程位于 `External/xiaozhi-esp32/`。
- 已知限制：`ESP-IDF v5.4.3 + esp32s3` 当前无法把经典蓝牙 SPP 后端链接为可运行固件。
- 影响：AI端到 HC-05 的无线闭环仍受目标芯片限制，当前更适合继续完善接口、协议、日志和性能路径，而不是假定无线闭环已经完成。
- 相关文档：
  - `Doc/项目文档/bt_version_hc05_uart2_architecture.md`
  - `Doc/项目文档/bluetooth_replacement_plan.md`

### 统一验证入口

- AI端与 LED端 联调优先使用：`tools/ws2812_dev_cycle.py`
- LED端 Keil 工程：`STC51/Project/ws2812_driver/ws2812_driver.uvproj`
- AI端 MCP/截图脚本：`External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`
