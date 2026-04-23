# GP Debug Feature Usage

## 中文

本文档说明当前 `AI端` 调试界面、截图链路和 MCP 调试能力。以下内容以 `lichuang-dev` 上已稳定运行的单页调试菜单实现为准。

### 1. 菜单结构

- 默认界面保持原有 `AI端` 主界面。
- 主界面右下角提供 `DBG` 入口按钮。
- 点击 `DBG` 后进入独立调试菜单；点击 `Back` 返回主界面。
- 调试菜单隐藏时，所有调试控件一起隐藏，不参与主界面绘制路径。

### 2. 稳定版调试菜单布局

- 顶部固定标题栏：左侧 `Back`，中间 `Debug Menu`，右侧圆形 `S` 截图按钮。
- 菜单主体改为纵向滚动卡片布局，不再使用左右滑动分页。
- 顶部卡片为触摸控制区，集中放置 `Pattern` 与 `Effect` 两组切换按钮。
- 中部卡片为 `Dot` 预览区；默认显示圆点，收到 Wi-Fi 预览图片后会切换为图片缩略预览。
- 下方卡片为 `Link` 状态区和摘要信息区，长文本会自动换行，适合显示完整来源、转写和连接状态。

### 3. 稳定性与响应策略

- 调试菜单只在显式打开时刷新，避免常驻控件增加 LVGL 重绘压力。
- 调试圆点动画仅在调试菜单可见时运行。
- 触摸按钮只在点击时触发矩阵状态下发，不做后台轮询。
- 触摸按钮在本地更新矩阵状态后，会同步生成一条自然语言转写，并复用 `voice_color_analyze` 的 `custom` 请求链路发给大模型。
- 调试菜单显示/隐藏通过异步切换处理，避免点击事件重入影响 LVGL。
- 触摸产生的矩阵更新与截图任务通过板级工作队列在后台执行，避免把重任务放进 LVGL 点击回调。
- `LED端` 收到完整协议包后先准备 ACK，再由主循环执行具体 LED 动作，减少 `AI端` 读回超时。

### 4. 语音调试用法

`AI端` 支持根据语音中的颜色意图更新调试圆点，并把同一份状态同步到 `LED端` 矩阵。

可表达的信息包括：

- 主颜色 `primary_rgb888`
- 辅助颜色 `secondary_rgb888`
- 动画类型 `solid | gradient | pulse`
- 圆点尺寸 `size`
- 动画周期 `duration_ms`

示例语音：

- `把圆点改成红色`
- `把圆点改成蓝绿色并大一点`
- `让圆点渐变显示`
- `让圆点呼吸闪烁`

### 5. 设备侧 MCP 工具

当前 `AI端` 固件已注册以下调试相关 MCP 工具：

- `self.calculator.calculate`
- `self.screen.debug_dot.show`
- `self.screen.matrix_16x16.draw`
- `self.screen.debug_snapshot.capture`
- `self.screen.debug_snapshot.set_upload_url`
- `self.screen.debug_snapshot.get_upload_url`

其中：

- `self.calculator.calculate` 用于验证 MCP 工具链是否正常。
- `self.screen.debug_dot.show` 用于直接控制调试圆点状态。
- `self.screen.matrix_16x16.draw` 用于下发 `16x16 RGB332` 整帧，或直接调用 `python_demo` 预设。
- `self.screen.debug_snapshot.capture` 用于由主机侧触发一次设备截图任务。
- `set_upload_url / get_upload_url` 用于管理设备本地 HTTP 上传地址。

### 6. 触摸调试按键

当前触摸调试菜单支持以下固定控件：

- `Pattern`：循环切换 `solid / diamond / cross / JLU_emblem / python_demo`。
- `Effect`：循环切换 `solid / gradient / pulse`。
- 标题栏 `S`：抓取当前屏幕内容到独立缓冲区，编码为 PNG 后通过本地 HTTP 上传保存。

补充说明：

- 语音说出 `吉林大学校徽`、`吉大校徽` 或 `JLU_emblem` 时，会直接选中对应校徽图案。
- 语音说出 `python_demo`、`16x16` 或 `像素图` 时，会切到内置整帧预设，并走整帧 `RGB332` 传输链路。
- 每次触摸切换后，屏幕圆点和 `LED端` 矩阵会同步更新；同一条触摸语句也会送入大模型侧分析链路，`source=touch`。
- 截图进度、成功和失败信息统一输出到串口日志，不再占用屏幕菜单空间。
- 点击 `S` 后，设备会冻结按键当下的同一帧，先释放 LVGL 快照资源，再在后台完成 PNG 编码和 HTTP 上传。
- 当前截图链路不再依赖官方语音 MCP 桥接上的反向 `tools/call` 携带图像数据。

### 7. 通过 MCP 直接控制圆点

调用工具：`self.screen.debug_dot.show`

示例参数：

```json
{
  "primary_rgb888": "#14B8A6",
  "secondary_rgb888": "#60A5FA",
  "animation": "gradient",
  "size": 42,
  "duration_ms": 1800,
  "label": "teal",
  "transcript": "把圆点改成蓝绿色并大一点",
  "source": "mcp"
}
```

字段说明：

- `primary_rgb888`：必填，主颜色。
- `secondary_rgb888`：可选，渐变或脉冲辅助颜色。
- `animation`：`solid`、`gradient`、`pulse`。
- `size`：范围 `12..58`。
- `duration_ms`：范围 `300..4000`。

### 8. 本地保存截图

脚本：`External/xiaozhi-esp32/GP_Port/gp_display_mcp_bridge.py`

直接运行即可：

```bash
python External/xiaozhi-esp32/GP_Port/gp_display_mcp_bridge.py
```

默认行为：

- 自动连接预设 MCP WebSocket 端点。
- 以本地 MCP Server 方式接入桥接端。
- 同时启动本地调试 WebSocket Server，默认监听 `ws://<host>:8766/debug`，供 `AI端` 主动连接。
- 同时启动本地 HTTP 端点：`/snapshot`、`/control/snapshot`、`/control/matrix_16x16`、`/control/matrix_prompt_16x16`、`/control/device_preview`、`/status`。
- 对外暴露更适合 LLM 直接理解的 MCP 工具：`self.screen.matrix_16x16.draw_frame`、`self.screen.matrix_16x16.draw_python`、`self.screen.matrix_16x16.show_text`。
- 设备侧 `S` 按键把 PNG 直接上传到 `/snapshot`。
- 调试菜单触摸按钮不再走旧的 LLM/MCP 触摸分析链路；其中 `Draw` 按键会通过独立 debug websocket 向主机发起请求，由主机随机选择 `16x16` 图案并返回位图数据，再由 `AI端` 本地绘制到 `Preview` 区域。
- 主机侧可通过 `/control/snapshot` 请求脚本调用设备 MCP 工具 `self.screen.debug_snapshot.capture`。
- 主机侧也可通过 MCP 直接调用 `self.screen.matrix_16x16.draw_python` 生成任意 `16x16` 图案，或调用 `self.screen.matrix_16x16.show_text` 逐帧显示文字。
- 主机侧也可通过 `/control/device_preview` 按 `device_ip` 把本地 PNG/JPEG 直接发到设备 `POST /debug/preview_image`，设备收到后会在二级菜单预览卡片中显示。
- 脚本会持续打印 MCP 状态、debug websocket 状态、最近一次工具调用、最近一次主机控制调用、最近保存路径和最近错误；`GET /status` 还会返回最近一次调用是否成功、结果摘要和最近调用历史。
- 图片默认保存到项目根目录 `debug_snapshots/`。
- 启动日志会打印 `snapshot upload url=http://.../snapshot`，设备可通过 MCP 工具或串口 `snap_url` 命令写入该地址。
- 固件支持 `snap_url set|get|clear|reset|help`、`snap [quality]`、以及 `debug_ws get|status|set <url>|clear|close|help` 串口命令。
- 如果设备里尚未保存地址且固件提供了编译期默认 URL，首次启动会自动写入。
- 设备连上 Wi-Fi 后会在 monitor 打印 `WiFi STA IP: ...`，可直接作为 `/control/device_preview` 的 `device_ip` 参数。

脚本职责边界：

- 提供本地 MCP Server，专门对接外部 LLM/MCP bridge 的工具调用。
- 提供本地 debug websocket server，专门承接 `AI端` 主动发起的数据传输请求。
- 提供本地 HTTP 保存入口，接收设备截图上传。
- 提供本地 HTTP 控制入口，请求设备执行截图。
- 提供状态查询入口，用于检查桥接、debug websocket 和最近一次执行结果。
- 不再承担串口触发截图的职责。
- 详细 LLM 绘图接口说明见：`External/xiaozhi-esp32/GP_Port/gp_matrix_drawing_mcp_usage.md`。

主机触发截图示例：

```bash
curl -X POST http://127.0.0.1:8765/control/snapshot -H "Content-Type: application/json" -d "{\"quality\":50}"
```

### 9. 官方 MCP 桥接测试

若使用官方桥接地址，例如：

```text
wss://api.xiaozhi.me/mcp/?token=...
```

本地脚本需要作为 MCP Server 响应桥接端主动发起的：

- `initialize`
- `notifications/initialized`
- `tools/list`

测试命令示例：

```bash
python GP_Port/gp_display_mcp_bridge.py \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --timeout 10
```

当前实测结果：

- 可成功连接官方桥接地址。
- 可完成 `initialize` 握手。
- 本地脚本握手后会持续等待，同时接受主机 `/control/snapshot` 请求和 `AI端` 发起的 debug websocket 连接。
- 可返回 `tools/list`，包含计算器、调试圆点和截图相关工具。
- 设备触摸 `S` 走本地 HTTP `/snapshot` 上传链路。
- 调试菜单 `Draw` 按键走 `AI端 debug websocket client -> 本地 Python websocket server -> matrix_pattern_result -> AI 本地 Preview`。
- 主机触发截图走 `/control/snapshot` -> MCP `self.screen.debug_snapshot.capture` -> 设备执行上传。

## English

This document describes the stable `AI side` debug UI, snapshot flow, and MCP-side debugging capabilities on `lichuang-dev`.

### Menu Structure

- The original AI-side main screen remains the default main screen.
- A `DBG` button opens the secondary debug menu.
- The stable debug menu uses a vertically scrollable card layout instead of swipeable sub-pages.
- Closing the menu returns to the original AI-side screen.

### Stable Layout

- Fixed header: `Back / Debug Menu / S`.
- Top card: touch controls for `Pattern` and `Effect`.
- Middle card: centered dot preview.
- Lower cards: LED-side link panel and a wrapped summary view for the current debug state.

### Runtime Strategy

- Debug widgets stay hidden by default.
- Dot animation runs only while the debug menu is visible.
- Touch updates still emit a natural-language transcript, but debug-menu transport now prefers the dedicated debug websocket path instead of the old `voice_color_analyze`/MCP request chain.
- Heavy matrix updates and snapshot work run through a board-side worker queue.
- The LED side prepares ACK data immediately after a full packet is captured, while LED execution stays in the main loop.

### Debug WebSocket Flow

- The host Python script exposes a local websocket server, separate from the MCP bridge connection.
- The `AI side` acts as websocket client and prints connection URL, send/receive payloads, and runtime status in monitor.
- The debug-menu `Draw` button sends `draw_random_pattern_request`.
- The host chooses a random `16x16` template and replies with `matrix_pattern_result` carrying `bitmap_rows_hex` and `primary_rgb888`.
- The `AI side` renders the returned pattern directly in the `Preview` card without routing the touch event through MCP.

### Snapshot Flow

- The header `S` button freezes the current frame, releases the LVGL-owned snapshot buffer quickly, encodes PNG in the background, and uploads it to the local HTTP receiver.
- Host-triggered snapshots use `POST /control/snapshot`, which calls `self.screen.debug_snapshot.capture` on the device.
- Progress and failure details are reported through serial logs instead of persistent on-screen status text.

### Bridge Example

```bash
python GP_Port/gp_display_mcp_bridge.py \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --timeout 10
```
