# GP MCP Tools

## 中文

当前 `lichuang-dev` 已提供稳定的设备侧调试工具集合，同时本地 Python MCP 脚本提供 HTTP 截图接收与 HTTP 控制入口：

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

用途：通过 MCP 直接控制右侧中部的调试圆点。

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

用途：由主机侧 Python 脚本通过 MCP 主动请求设备执行一次截图任务。

参数：

```json
{
  "quality": 50
}
```

说明：

- 该工具只负责让设备开始执行截图任务并返回执行摘要。
- 图像数据不会内联在 MCP 返回里，而是通过设备本地 HTTP 上传地址直接发送到开发机。
- 该工具适用于主机自动化验证，不再依赖串口文本命令作为唯一截图触发方式。

### 4. `self.screen.debug_snapshot.set_upload_url`

用途：设置设备侧标题栏 `S` 按键使用的本地 HTTP 上传地址。

参数：

```json
{
  "url": "http://192.168.1.100:8765/snapshot"
}
```

说明：

- 该地址应来自本地 Python 脚本启动时打印的 `snapshot upload url=...`。
- 设置为空字符串可清除当前配置。
- 这是设备侧持久化配置，供标题栏 `S` 按键直接走 HTTP 上传，不再依赖反向 MCP `tools/call`。
- 当前固件还会在启动时自动写入编译期默认 URL；若设备里已经存在手动设置值，则不会重复覆盖。

### 5. `self.screen.debug_snapshot.get_upload_url`

用途：读取当前设备侧标题栏 `S` 按键使用的本地 HTTP 上传地址。

补充说明：

- 语音模型可调用这些工具，但设备触摸按键与语音工具调用并不是同一链路。
- 当前设备侧 `S` 按键走本地 HTTP 上传。
- 主机触发截图走本地 HTTP 控制端转 MCP 调用设备工具。

### 推荐调用顺序

远端 MCP 客户端建议按以下顺序调用：

1. `initialize`
2. `notifications/initialized`
3. `tools/list`
4. `tools/call`

### 本地 HTTP 端点

本地 Python 脚本启动后默认暴露以下 HTTP 端点：

- `POST /snapshot`：接收设备上传的 PNG 并保存到 `debug_snapshots/`。
- `POST /control/snapshot`：请求脚本调用设备 MCP 工具 `self.screen.debug_snapshot.capture`。
- `GET /status`：查看桥接连接、初始化状态、最近一次调用和最近错误。

主机触发截图示例：

```bash
curl -X POST http://127.0.0.1:8765/control/snapshot -H "Content-Type: application/json" -d "{\"quality\":50}"
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
```

说明：

- `snap` 与 `snap <quality>` 仍保留在固件中，主要用于设备侧底层调试和排障。
- `get`：打印当前已持久化的上传地址。
- `set <url>`：立即写入新地址到 NVS。
- `clear`：清空当前地址。
- `reset`：恢复到固件里编译期写死的默认地址。
- `help`：打印帮助和当前状态。

### MCP 端点测试脚本

可使用 [GP_Port/gp_mcp_endpoint_client.py](GP_Port/gp_mcp_endpoint_client.py) 直接接入远端 `wss` 地址。

注意：

- 该脚本默认以本地 MCP Server 方式运行，等待桥接端调用本地工具。
- 该脚本会额外启动本地 HTTP 服务器，既接收设备侧 `S` 按键上传的 PNG，也接收主机侧截图控制请求。

示例：

```bash
python GP_Port/gp_mcp_endpoint_client.py \
  --verbose
```

输出内容包括：

- 连接是否成功
- `initialize` 往返过程
- `tools/list` 请求与返回内容
- 上游是否能看到 `self.calculator.calculate` 和 `self.screen.debug_dot.show` 本地工具
- 本地保存路径是否正确回传
- 本地 HTTP 服务是否启动，并打印了 `snapshot upload url`、`snapshot control url` 与 `status url`

如果服务端长时间不回包，可通过 `--timeout 15` 调整等待时间。

## English

`lichuang-dev` exposes stable device-side snapshot/debug tools, and the local Python bridge exposes a local HTTP receiver plus a local HTTP control endpoint:

1. `self.calculator.calculate`
2. `self.screen.debug_dot.show`
3. `self.screen.debug_snapshot.capture`
4. `self.screen.debug_snapshot.set_upload_url`
5. `self.screen.debug_snapshot.get_upload_url`

They are intended for remote MCP endpoint integration and validation of the tool-call flow, while the actual snapshot payload is uploaded through the local HTTP path.

## Endpoint Validation Script

`GP_Port/gp_mcp_endpoint_client.py` can connect to a remote `wss://.../mcp/...` endpoint and validate the full JSON-RPC flow.

For official bridge endpoints, run it in the default server mode, let the device upload snapshots to `/snapshot`, and use `/control/snapshot` when the host needs to request a device-side capture.

Example:

```bash
python GP_Port/gp_mcp_endpoint_client.py \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose
```

Dependency:

```bash
python -m pip install websockets
```
