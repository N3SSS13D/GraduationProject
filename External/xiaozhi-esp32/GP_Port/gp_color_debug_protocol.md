# GP Color Debug LLM Protocol

## 中文

### 现有交互模式

1. ESP32 通过 WebSocket 或 MQTT 建立会话。
2. 设备主动上报控制类 JSON，例如 `listen`、`abort`、`mcp`。
3. 服务端回推 `stt`、`tts`、`llm`、`mcp`、`custom` 等消息。
4. 本次扩展复用 `custom` 通道做颜色分析，不改动音频链路。

### 颜色分析请求

设备在收到 `stt` 文本后，如果判断为颜色调试意图，会发送：

```json
{
  "session_id": "<session>",
  "type": "custom",
  "payload": {
    "action": "voice_color_analyze",
    "source": "stt",
    "transcript": "把圆点改成蓝绿色并大一点",
    "response_format": {
      "action": "voice_color_result",
      "primary_rgb888": "#RRGGBB",
      "secondary_rgb888": "#RRGGBB or empty",
      "animation": "solid|gradient|pulse",
      "size": 36,
      "duration_ms": 1400,
      "label": "teal",
      "source": "llm",
      "transcript": "original transcript"
    }
  }
}
```

### 颜色分析返回

服务端应通过 `custom` 返回：

```json
{
  "type": "custom",
  "payload": {
    "action": "voice_color_result",
    "primary_rgb888": "#14B8A6",
    "secondary_rgb888": "#60A5FA",
    "animation": "gradient",
    "size": 42,
    "duration_ms": 1800,
    "label": "teal",
    "source": "llm",
    "transcript": "把圆点改成蓝绿色并大一点"
  }
}
```

## English

The device keeps the existing JSON control flow and adds one `custom` request/response pair for voice-driven color debugging.

- Request action: `voice_color_analyze`
- Response action: `voice_color_result`
- Required color field: `primary_rgb888` in `#RRGGBB`
- Optional fields: `secondary_rgb888`, `animation`, `size`, `duration_ms`, `label`, `source`, `transcript`

## 服务端接入建议

1. 服务端收到 `voice_color_analyze` 后，将 `transcript` 和 [GP_Port/gp_color_debug_llm.prompt.md](GP_Port/gp_color_debug_llm.prompt.md) 组合成一次大模型调用。
2. 要求大模型只返回单个 JSON 对象，不带代码块或解释。
3. 服务端校验字段后，再封装进 `{"type":"custom","payload":...}` 回给设备。
