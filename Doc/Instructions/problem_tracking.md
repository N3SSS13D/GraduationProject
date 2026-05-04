# 问题说明与约束记录

## 适用范围

本文档只记录当前方案中仍然有效的问题、限制和约束。

分类阅读入口：

1. `LED端显示驱动`：`Project/STC51/`
2. `AI端接口调度`：`Project/xiaozhi-esp32/main/gp_port/`
3. `蓝牙通信协议`：`Project/Protocols/`
4. `本地绘图脚本`：`Project/Script/`

## 当前有效约束

### 1. 结构阅读约束

- 修改时先根据四类结构定位最窄相关目录。
- 不要默认扫描无关分类目录。
- 共享协议字段以 `Project/Protocols/gp_led_matrix_protocol.h` 为准。

### 2. AI端 蓝牙链路约束

- 当前 `AI端` 活跃板型为 `Project/xiaozhi-esp32/main/boards/lichuang-dev/`。
- `HC-05` 配置流程按 `38400` AT 阶段和 `460800` 数据阶段组织。
- 若任务需要解释当前蓝牙链路结构，优先查看 `Doc/Instructions/bt_version_hc05_uart2_architecture.md`。

### 3. 统一验证入口

- `AI端` / `LED端` 联调入口：`Project/Script/tools/ws2812_auto_debug.py`
- `LED端` Keil 工程：`Project/STC51/ws2812_driver/ws2812_driver.uvproj`
- MCP 绘图入口：`Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`

### 4. 当前模块热点

- `LED端`
  - 远程帧、远程动画和本地离线渲染共存，修改时要明确是否会影响 `gp_led_action.c` 的控制切换。
  - 显示问题不要只看 `ws2812_drv.c`；很多行为来自 `app.c` 调度和 `draw_drv.c` / `gp_led_action.c` 的上层状态。
- `AI端`
  - 蓝牙接收保持后台任务组包模型，避免回退成调用时轮询。
  - 预览、触摸和主机绘图转发主要在 `lichuang_dev_board.cc` 串起来，问题常常不只在 `gp_led_matrix_esp32.cc`。
- `协议`
  - 共享字段只允许以 `Project/Protocols/gp_led_matrix_protocol.h` 为源头，不能在两端各自扩展。
  - 若命令语义变化，必须同步更新协议说明和脚本契约说明。
- `脚本`
  - MCP/绘图脚本的目标是 `AI端` 预览与转发接口，不应直接假定 `LED端` 原始串口发包细节。

### 5. Prompt / Skill 结构待解决问题

- 需要持续检查四模块 Prompt 与 Skill 的成对覆盖，避免只更新单侧文档。
- 需要把模块细则优先下沉到分类 Skill 与 README，避免单个 Prompt 过长造成调用偏移。
- 需要定期执行调用冒烟：按四模块各触发一次 Prompt 和 Skill，确认读取边界与输出格式稳定。
- 需要保持中英文 Prompt 同步，避免字段、边界或验证要求出现语义漂移。

### 6. 结构优化建议

- 入口统一为：`分类 README -> 模块 Prompt -> 模块 Skill`。
- 跨模块审查单独使用 `ws2812-code-review*.prompt.md`，不与模块实施 Prompt 混用。
- 若模块新增关键子链路，先更新分类 README 的最小阅读集，再更新 Prompt 与 Skill。
