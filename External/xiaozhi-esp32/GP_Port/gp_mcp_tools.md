# GP MCP Tools

## 中文

当前 `lichuang-dev` 已新增两个可被远端 MCP 调用的工具：

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

### 推荐调用顺序

远端 MCP 客户端建议按以下顺序调用：

1. `initialize`
2. `notifications/initialized`
3. `tools/list`
4. `tools/call`

### MCP 端点测试脚本

可使用 [GP_Port/gp_mcp_endpoint_client.py](GP_Port/gp_mcp_endpoint_client.py) 直接验证远端 `wss` 地址。

注意：

- 如果目标端点是“由对端先发 `initialize`”的官方桥接地址，应使用 `--mode server`。
- 如果目标端点本身就是一个 MCP Server，需要由本脚本主动发起 `initialize`，应使用 `--mode client`。

示例：

```bash
python GP_Port/gp_mcp_endpoint_client.py \
  --mode server \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --server-exit-after-tool-calls 1
```

输出内容包括：

- 连接是否成功
- `initialize` 往返过程
- `tools/list` 请求与返回内容
- 计算器调用结果
- 圆点控制调用结果

如果服务端长时间不回包，可通过 `--timeout 15` 调整等待时间。

## English

`lichuang-dev` now exposes two MCP tools:

1. `self.calculator.calculate`
2. `self.screen.debug_dot.show`

They are intended for remote MCP endpoint integration and validation of the tool call flow.

## Endpoint Validation Script

`GP_Port/gp_mcp_endpoint_client.py` can connect to a remote `wss://.../mcp/...` endpoint and validate the full JSON-RPC flow:

1. `initialize`
2. `notifications/initialized`
3. `tools/list`
4. `tools/call` for `self.calculator.calculate`
5. `tools/call` for `self.screen.debug_dot.show`

Mode selection:

- Use `--mode server` when the remote endpoint sends `initialize` first.
- Use `--mode client` when the remote endpoint is itself an MCP server.

Example:

```bash
python GP_Port/gp_mcp_endpoint_client.py \
  --mode server \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --server-exit-after-tool-calls 1
```

Dependency:

```bash
python -m pip install websockets
```
