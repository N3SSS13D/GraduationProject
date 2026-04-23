# GP Color Debug LLM Protocol

## 中文

### 当前定位

本协议说明 `AI端` 如何通过现有 JSON 控制流，把语音或触摸生成的颜色控制意图整理成 `voice_color_result`，再同步给本地调试界面和 `LED端` 动作桥接。

当前原则：

1. 不改音频链路，只复用 `custom` 消息通道。
2. `AI端` 只负责生成结构化颜色/效果结果。
3. `LED端` 复用同一份结果做颜色、预设和动画下发。

### 颜色分析请求

`AI端` 在收到 `stt` 文本，或本地触摸菜单生成控制语句后，如果判断为颜色调试意图，会发送：

```json
{
  "session_id": "<session>",
  "type": "custom",
  "payload": {
    "action": "voice_color_analyze",
    "source": "stt or touch",
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

The `AI side` keeps the existing JSON control flow and adds one `custom` request/response pair for speech-driven or touch-generated color debugging. The returned state is then reused by the local debug UI and the LED-side action bridge.

- Request action: `voice_color_analyze`
- Response action: `voice_color_result`
- Allowed request sources: `stt`, `touch`
- Required color field: `primary_rgb888` in `#RRGGBB`
- Optional fields: `secondary_rgb888`, `animation`, `size`, `duration_ms`, `label`, `source`, `transcript`

## 服务端接入建议

1. 服务端收到 `voice_color_analyze` 后，将 `transcript` 和 `GP_Port/gp_color_debug_llm.prompt.md` 组合成一次大模型调用；`source` 可用于区分 `stt` 还是触摸生成文本，但不要改变返回结构。
2. 要求大模型只返回单个 JSON 对象，不带代码块或解释。
3. 服务端校验字段后，再封装进 `{"type":"custom","payload":...}` 回给 `AI端`。
4. `AI端` 收到结果后，应复用同一份结构同步本地调试界面和 `LED端` 协议动作，而不是再维护一套独立测试数据。
