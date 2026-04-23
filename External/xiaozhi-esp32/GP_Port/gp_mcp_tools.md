# GP MCP Tools

## 中文

当前 `AI端` 已提供稳定的设备侧调试工具集合，同时本地 Python MCP 脚本提供 HTTP 截图接收与 HTTP 控制入口：

### 1. `self.calculator.calculate`

用途：演示 MCP 工具调用链是否正常。

参数：

```json
{
  "operation": "add",
  "left": 12,
  "right": 30
}
```

支持的 `operation`：

- `add`
- `subtract`
- `multiply`
- `divide`
- `mod`

### 2. `self.screen.debug_dot.show`

用途：通过 MCP 直接控制 `AI端` 调试界面中的圆点状态，并复用同一份状态同步 `LED端`。

参数：

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

说明：

- `primary_rgb888` 必填。
- `secondary_rgb888` 可空。
- `animation` 支持 `solid`、`gradient`、`pulse`。
- `size` 范围为 `12..58`。
- `duration_ms` 范围为 `300..4000`。

### 3. `self.screen.debug_snapshot.capture`

用途：由主机侧 Python 脚本通过 MCP 主动请求 `AI端` 执行一次截图任务。

参数：

```json
{
  "quality": 50
}
```

说明：

- 该工具只负责让 `AI端` 开始执行截图任务并返回执行摘要。
- 图像数据不会内联在 MCP 返回里，而是通过设备本地 HTTP 上传地址直接发送到开发机。
- 该工具适用于主机自动化验证，不再依赖串口文本命令作为唯一截图触发方式。

### 4. `self.screen.matrix_16x16.draw_frame`

用途：低层直接绘制接口。当远端模型已经有 `frame_rgb332_hex` 或 `bitmap_rows_hex` 时，使用该工具返回统一 `16x16` 帧结果，并在可用时优先通过 debug websocket 发给 `AI端` 预览。

参数：

```json
{
  "preset": "python_demo",
  "frame_rgb332_hex": "",
  "source": "mcp",
  "transcript": "display low-level matrix frame"
}
```

或：

```json
{
  "preset": "",
  "bitmap_rows_hex": "8000400020001000080004000200010001000200040008001000200040008000",
  "primary_rgb888": "#00FF66",
  "background_rgb888": "#000000",
  "source": "mcp",
  "transcript": "draw custom 16x16 frame"
}
```

说明：

- `preset` 当前支持 `python_demo`。
- `frame_rgb332_hex` 需要提供 `256` 个 RGB332 字节，即 `512` 个十六进制字符。
- 也可使用紧凑编码：`bitmap_rows_hex` 提供 `16` 行位图，共 `64` 个十六进制字符；`primary_rgb888` 提供单个前景色，共 `3` 字节。位图中 `1` 表示图案像素，`0` 表示黑色背景。
- 当 `AI端` debug websocket 已连接时，主机优先通过 websocket 下发 `matrix_pattern_result` 预览数据。
- 当 websocket 不可用且返回里包含完整 `frame_rgb332_hex` 时，主机回退到原有的 HTTP 预览链路。
- 蓝牙 `LED端` 发送链路仍可继续使用现有矩阵帧协议，但 `AI端` 预览不再依赖主机直接向设备私网地址推送图片。

### 5. `self.screen.matrix_16x16.draw_python`

用途：主要的 LLM 绘图接口。输入受限的常用 Python 绘图语句，主机脚本在本地生成一帧 `16x16` 图像，并输出统一帧格式。

参数：

```json
{
  "python_source": "for i in range(16):\n    draw.point((i, i), fill=1)\n    draw.point((15 - i, i), fill=1)",
  "primary_rgb888": "#00FF66",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw a green X"
}
```

说明：

- 该工具输出统一 `matrix_frame_v1` 数据格式。
- 允许常见 `draw.point / draw.line / draw.rectangle / draw.ellipse / draw.polygon` 风格语句。
- 允许 `for ... in range(...)` 和简单变量赋值。
- 不允许 `import`、`while`、函数定义、类定义、文件访问或其它非绘图属性访问。
- `python_source` 只绘制二值掩码；实际颜色由 `primary_rgb888` 和 `background_rgb888` 统一决定。

### 6. `self.screen.matrix_16x16.show_text`

用途：输入文字，脚本通过字模提取把每个字符转成 `16x16` 单帧，并逐帧传输到 `AI端` 预览区域显示。

参数：

```json
{
  "text": "Hi",
  "frame_interval_ms": 180,
  "primary_rgb888": "#FFFFFF",
  "background_rgb888": "#000000",
  "source": "mcp_text",
  "transcript": "show text Hi"
}
```

说明：

- 返回格式为 `matrix_frame_sequence_v1`。
- `frames` 内每一项仍是当前统一的单帧格式 `matrix_frame_v1`。
- 当 `AI端` debug websocket 已连接时，脚本会按 `frame_interval_ms` 逐帧发送并在 `AI端` 显示。

### 7. `self.screen.matrix_16x16.render_prompt`

用途：兼容旧流程。在本地 Python MCP 脚本中把自然语言图案描述映射为模板图案，并输出一帧统一 `16x16` 数据格式。

参数：

```json
{
  "prompt": "绘制一个青色爱心",
  "primary_rgb888": "",
  "background_rgb888": "#000000",
  "source": "mcp_prompt",
  "transcript": "绘制一个青色爱心"
}
```

说明：

- `prompt` 必填。
- 当前脚本内置若干 `16x16` 模板图案，用于把自由文本稳定映射为统一帧结果。
- 返回值中会同时给出 `frame_rgb332_hex` 和紧凑编码 `bitmap_rows_hex`。
- 新任务优先考虑 `self.screen.matrix_16x16.draw_python`；`render_prompt` 主要用于兼容旧的 prompt-to-template 流程。

### 8. `self.screen.preview_image.fetch_http`

用途：由主机脚本或本地 HTTP 控制入口向 `AI端` 下发一个图片 URL，设备再通过 Wi-Fi 主动 `GET` 主机上的 PNG/JPEG，并显示在调试二级菜单 `Preview` 区域。

参数：

```json
{
  "url": "http://192.168.1.100:8765/generated/matrix_prompt_preview_20260422_120000_16x16.png",
  "source": "host_http",
  "transcript": "绘制一个青色爱心"
}
```

说明：

- 该工具只接收控制元数据，真正的图像字节通过 HTTP 传输。
- `AI端` 拉取成功后，会直接把图片显示到调试预览区域。
- 这条链路优先用于当前 `16x16` 图案预览调试，因为已验证 `AI -> host` 的 HTTP 方向更稳定。

### 9. `self.screen.debug_snapshot.set_upload_url`

用途：设置 `AI端` 标题栏 `S` 按键使用的本地 HTTP 上传地址。

参数：

```json
{
  "url": "http://192.168.1.100:8765/snapshot"
}
```

说明：

- 该地址应来自本地 Python 脚本启动时打印的 `snapshot upload url=...`。
- 设置为空字符串可清除当前配置。
- 这是 `AI端` 持久化配置，供标题栏 `S` 按键直接走 HTTP 上传，不再依赖反向 MCP `tools/call`。
- 当前固件还会在启动时自动写入编译期默认 URL；若本地已经存在手动设置值，则不会重复覆盖。

### 10. `self.screen.debug_snapshot.get_upload_url`

用途：读取当前 `AI端` 标题栏 `S` 按键使用的本地 HTTP 上传地址。

补充说明：

- 语音模型可调用这些工具，但设备触摸按键与语音工具调用并不是同一链路。
- 当前 `AI端` `S` 按键走本地 HTTP 上传。
- 主机触发截图走本地 HTTP 控制端，再转 MCP 调用 `AI端` 工具。

### 推荐调用顺序

远端 MCP 客户端建议按以下顺序调用：

1. `initialize`
2. `notifications/initialized`
3. `tools/list`
4. `tools/call`

### 本地 HTTP 端点

本地 Python 脚本启动后默认暴露以下 HTTP 端点：

- `POST /snapshot`：接收 `AI端` 上传的 PNG 并保存到 `debug_snapshots/`。
- `POST /control/snapshot`：请求脚本调用 `AI端` MCP 工具 `self.screen.debug_snapshot.capture`。
- `GET /generated/<file>`：读取主机脚本在 `debug_snapshots/` 下保存的 PNG/JPEG 资源，供 `AI端` 主动拉取。
- `POST /control/matrix_16x16`：请求脚本验证/生成 `16x16` 帧，保存 PNG，并让 `AI端` 调用 `self.screen.preview_image.fetch_http`。
- `POST /control/matrix_prompt_16x16`：请求脚本先将 `prompt` 渲染为 `16x16 RGB332` 帧并保存 PNG，再让 `AI端` 调用 `self.screen.preview_image.fetch_http`。
- `POST /control/device_preview`：按 `device_ip` 把本地 PNG/JPEG 直接上传到 `AI端` `http://<device_ip>:8781/debug/preview_image`，用于验证 Wi-Fi 图像预览链路。
- `GET /status`：查看桥接连接、初始化状态、最近一次调用是否成功、最近结果摘要，以及最近若干次调用历史。

补充：

- 对 `16x16` 图案预览链路，当前推荐使用“主机生成 PNG + `AI端` HTTP 拉取”模式；MCP 只负责 URL/控制命令。
- `HTTP /control/device_preview` 与 `GET /debug/preview_status` 仍保留用于排查通用 PNG/JPEG 预览链路，但主链路已经从“主机推送到 AI 私网地址”切换为“AI 主动从主机拉取”。

主机触发截图示例：

```bash
curl -X POST http://127.0.0.1:8765/control/snapshot -H "Content-Type: application/json" -d "{\"quality\":50}"

curl -X POST http://127.0.0.1:8765/control/matrix_16x16 -H "Content-Type: application/json" -d "{\"preset\":\"python_demo\"}"

curl -X POST http://127.0.0.1:8765/control/matrix_prompt_16x16 -H "Content-Type: application/json" -d "{\"prompt\":\"绘制一个青色爱心\"}"

curl -X POST http://127.0.0.1:8765/control/device_preview -H "Content-Type: application/json" -d "{\"device_ip\":\"192.168.1.88\",\"image_path\":\"D:/GraduationProject/debug_snapshots/sample.png\"}"
```

### 串口调试命令

当前 `lichuang-dev` 固件已补充 UART0 文本命令，可直接在串口监视器中输入：

```text
snap
snap 50
snap_url get
snap_url set http://49.140.69.242:8765/snapshot
snap_url clear
snap_url reset
snap_url help
debug_ws get
debug_ws status
debug_ws set ws://49.140.69.242:8766/debug
debug_ws clear
debug_ws close
debug_ws help
```

说明：

- `snap` 与 `snap <quality>` 仍保留在固件中，主要用于 `AI端` 侧底层调试和排障。
- `get`：打印当前已持久化的上传地址。
- `set <url>`：立即写入新地址到 NVS。
- `clear`：清空当前地址。
- `reset`：恢复到固件里编译期写死的默认地址。
- `help`：打印帮助和当前状态。
- `debug_ws get|status`：打印当前 debug websocket 的配置和运行时状态。
- `debug_ws set <url>`：更新 `AI端` 调试 websocket 连接地址。
- `debug_ws clear`：清除持久化地址，回退到固件默认 websocket 地址。
- `debug_ws close`：主动关闭当前 debug websocket 连接，便于重新连线调试。

### MCP 端点测试脚本

可使用 [GP_Port/gp_display_mcp_bridge.py](GP_Port/gp_display_mcp_bridge.py) 直接接入远端 `wss` 地址。

注意：

- 该脚本默认以本地 MCP Server 方式运行，等待桥接端调用本地工具。
- 该脚本会额外启动本地 debug websocket server 与本地 HTTP 服务器。
- debug websocket server 专门承接 `AI端` 主动发起的数据传输请求，例如调试菜单 `Draw` 按键请求随机 `16x16` 图案。
- HTTP 服务器仍负责设备侧 `S` 按键截图上传和主机侧 HTTP 控制请求。

示例：

```bash
python GP_Port/gp_display_mcp_bridge.py \
  --verbose
```

输出内容包括：

- 连接是否成功
- `initialize` 往返过程
- `tools/list` 请求与返回内容
- 上游是否能看到 `self.screen.matrix_16x16.draw_frame`、`self.screen.matrix_16x16.draw_python` 与 `self.screen.matrix_16x16.show_text` 本地工具
- 本地保存路径是否正确回传
- 本地 debug websocket 服务是否启动，并打印了 `debug_ws` 监听地址和收发日志
- 本地 HTTP 服务是否启动，并打印了 `snapshot upload url`、`snapshot control url` 与 `status url`
- 本地 HTTP 服务是否打印了 `matrix control url` 与 `matrix prompt control url`

详细的 LLM 绘图/文字接口说明见：`GP_Port/gp_matrix_drawing_mcp_usage.md`

如果服务端长时间不回包，可通过 `--timeout 15` 调整等待时间。

## English

`lichuang-dev` exposes stable AI-side snapshot/debug tools, and the local Python bridge exposes a local HTTP receiver plus a local HTTP control endpoint:

1. `self.calculator.calculate`
2. `self.screen.debug_dot.show`
3. `self.screen.matrix_16x16.draw_frame`
4. `self.screen.matrix_16x16.draw_python`
5. `self.screen.matrix_16x16.show_text`
6. `self.screen.matrix_16x16.render_prompt`
5. `self.screen.debug_snapshot.capture`
6. `self.screen.debug_snapshot.set_upload_url`
7. `self.screen.debug_snapshot.get_upload_url`

They are intended for remote MCP endpoint integration and validation of the tool-call flow, while the actual snapshot payload is uploaded through the local HTTP path.

## Endpoint Validation Script

`GP_Port/gp_display_mcp_bridge.py` can connect to a remote `wss://.../mcp/...` endpoint and validate the full JSON-RPC flow.

For official bridge endpoints, run it in the default server mode, let the device upload snapshots to `/snapshot`, and use `/control/snapshot` when the host needs to request a device-side capture.

Example:

```bash
python GP_Port/gp_display_mcp_bridge.py \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose
```

Dependency:

```bash
python -m pip install websockets
python -m pip install Pillow
```
