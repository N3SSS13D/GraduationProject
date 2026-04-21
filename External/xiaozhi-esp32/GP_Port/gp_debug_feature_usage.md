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
- 菜单主体为单页绝对布局，不再使用左右滑动分页。
- 左侧为触摸控制区，集中放置 `Pattern` 与 `Effect` 两组切换按钮。
- 右上为 `Dot` 预览区，圆点和标题都居中显示。
- 右中为 `Link` 状态区，状态圆点位于 `Link` 文字后方，状态文本独立显示在下方。
- 左下为摘要信息区，用于显示当前颜色、动画、来源和状态概览。

### 3. 稳定性与响应策略

- 调试菜单只在显式打开时刷新，避免常驻控件增加 LVGL 重绘压力。
- 调试圆点动画仅在调试菜单可见时运行。
- 触摸按钮只在点击时触发矩阵状态下发，不做后台轮询。
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
- `self.screen.debug_snapshot.capture`
- `self.screen.debug_snapshot.set_upload_url`
- `self.screen.debug_snapshot.get_upload_url`

其中：

- `self.calculator.calculate` 用于验证 MCP 工具链是否正常。
- `self.screen.debug_dot.show` 用于直接控制调试圆点状态。
- `self.screen.debug_snapshot.capture` 用于由主机侧触发一次设备截图任务。
- `set_upload_url / get_upload_url` 用于管理设备本地 HTTP 上传地址。

### 6. 触摸调试按键

当前触摸调试菜单支持以下固定控件：

- `Pattern`：循环切换 `solid / diamond / cross / JLU_emblem`。
- `Effect`：循环切换 `solid / gradient / pulse`。
- 标题栏 `S`：抓取当前屏幕内容到独立缓冲区，编码为 PNG 后通过本地 HTTP 上传保存。

补充说明：

- 语音说出 `吉林大学校徽`、`吉大校徽` 或 `JLU_emblem` 时，会直接选中对应校徽图案。
- 每次触摸切换后，屏幕圆点和 `LED端` 矩阵会同步更新。
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

脚本：`External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py`

直接运行即可：

```bash
python External/xiaozhi-esp32/GP_Port/gp_mcp_endpoint_client.py
```

默认行为：

- 自动连接预设 MCP WebSocket 端点。
- 以本地 MCP Server 方式接入桥接端。
- 同时启动本地 HTTP 端点：`/snapshot`、`/control/snapshot`、`/status`。
- 设备侧 `S` 按键把 PNG 直接上传到 `/snapshot`。
- 主机侧可通过 `/control/snapshot` 请求脚本调用设备 MCP 工具 `self.screen.debug_snapshot.capture`。
- 脚本会持续打印连接状态、最近一次工具调用、最近一次主机控制调用、最近保存路径和最近错误。
- 图片默认保存到项目根目录 `debug_snapshots/`。
- 启动日志会打印 `snapshot upload url=http://.../snapshot`，设备可通过 MCP 工具或串口 `snap_url` 命令写入该地址。
- 固件支持 `snap_url set|get|clear|reset|help` 和 `snap [quality]` 串口命令。
- 如果设备里尚未保存地址且固件提供了编译期默认 URL，首次启动会自动写入。

脚本职责边界：

- 提供本地 HTTP 保存入口，接收设备截图上传。
- 提供本地 HTTP 控制入口，请求设备执行截图。
- 提供状态查询入口，用于检查桥接和最近一次执行结果。
- 不再承担串口触发截图的职责。

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
python GP_Port/gp_mcp_endpoint_client.py \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --timeout 10
```

当前实测结果：

- 可成功连接官方桥接地址。
- 可完成 `initialize` 握手。
- 本地脚本握手后会持续等待，同时接受主机 `/control/snapshot` 请求。
- 可返回 `tools/list`，包含计算器、调试圆点和截图相关工具。
- 设备触摸 `S` 走本地 HTTP `/snapshot` 上传链路。
- 主机触发截图走 `/control/snapshot` -> MCP `self.screen.debug_snapshot.capture` -> 设备执行上传。

## English

This document describes the stable `AI side` debug UI, snapshot flow, and MCP-side debugging capabilities on `lichuang-dev`.

### Menu Structure

- The original AI-side main screen remains the default main screen.
- A `DBG` button opens the secondary debug menu.
- The stable debug menu uses a single-page layout, not swipeable sub-pages.
- Closing the menu returns to the original AI-side screen.

### Stable Layout

- Fixed header: `Back / Debug Menu / S`.
- Left side: touch controls for `Pattern` and `Effect`.
- Right-top: centered dot preview.
- Right-middle: LED-side link panel with a status dot after `Link`.
- Lower area: summary text for the current debug state.

### Runtime Strategy

- Debug widgets stay hidden by default.
- Dot animation runs only while the debug menu is visible.
- Heavy matrix updates and snapshot work run through a board-side worker queue.
- The LED side prepares ACK data immediately after a full packet is captured, while LED execution stays in the main loop.

### Snapshot Flow

- The header `S` button freezes the current frame, releases the LVGL-owned snapshot buffer quickly, encodes PNG in the background, and uploads it to the local HTTP receiver.
- Host-triggered snapshots use `POST /control/snapshot`, which calls `self.screen.debug_snapshot.capture` on the device.
- Progress and failure details are reported through serial logs instead of persistent on-screen status text.

### Bridge Example

```bash
python GP_Port/gp_mcp_endpoint_client.py \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --timeout 10
```
