# GP Debug Feature Usage

## 中文

本文档说明当前 `lichuang-dev` 上已实现的调试圆点与 MCP 功能如何使用。

### 1. 屏幕显示位置

- 当前仅显示一个调试圆点。
- 圆点中心位于屏幕横向右侧 `1/3` 区域的中点。
- 圆点纵向位于屏幕中线位置。

换算后，圆点中心坐标约为：

- `x = 5 / 6 * 屏幕宽度`
- `y = 1 / 2 * 屏幕高度`

### 2. 语音调试用法

设备支持根据语音中的颜色意图更新圆点显示状态。

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

### 3. 设备侧 MCP 工具

当前固件已在 `lichuang-dev` 板级注册以下工具：

- `self.calculator.calculate`
- `self.screen.debug_dot.show`

其中：

- `self.calculator.calculate` 用于验证 MCP 工具链是否正常。
- `self.screen.debug_dot.show` 用于直接控制屏幕右侧圆点。

### 4. 通过 MCP 直接控制圆点

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

### 5. 官方 MCP 桥接地址测试

如果使用官方桥接地址，例如：

```text
wss://api.xiaozhi.me/mcp/?token=...
```

需要将本地脚本作为 MCP Server 接入，因为桥接端会主动先发送：

- `initialize`
- `notifications/initialized`
- `tools/list`

测试命令示例：

```bash
python GP_Port/gp_mcp_endpoint_client.py \
  --mode server \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --timeout 10
```

当前实测结果：

- 可成功连接官方桥接地址。
- 可完成 `initialize` 握手。
- 可返回 `tools/list`，包含计算器和圆点工具。
- 在测试窗口内未收到实际 `tools/call`，因此说明桥接可用，但是否触发工具还取决于上游对话流是否调用。

## English

This document describes how to use the current debug dot and MCP features on `lichuang-dev`.

### Display Placement

- Only a single debug dot is shown.
- The dot center is placed at the center of the right third of the screen.
- Vertically, it is centered on the screen.

### Available Features

- Voice-driven color debugging
- MCP calculator tool
- MCP debug-dot control tool
- Official MCP bridge validation via WebSocket

### MCP Bridge Example

```bash
python GP_Port/gp_mcp_endpoint_client.py \
  --mode server \
  --url "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN" \
  --verbose \
  --timeout 10
```
