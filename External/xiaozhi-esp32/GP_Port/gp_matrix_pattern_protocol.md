# GP Matrix Pattern Request Protocol

## 中文

`AI端` 现在新增了一条独立于颜色调试链路的 `custom` 请求，用来把触摸屏上的固定按钮直接映射为一次 `16x16` 图案生成任务。

### 设备侧请求

当用户点击 `Draw` 按钮时，设备会发送：

```json
{
  "session_id": "<session>",
  "type": "custom",
  "payload": {
    "action": "matrix_pattern_request",
    "source": "touch",
    "transcript": "绘制任意图案的命令",
    "target": {
      "type": "led_matrix",
      "width": 16,
      "height": 16,
      "pixel_format": "rgb332"
    },
    "transport": {
      "link": "bluetooth_frame_upload",
      "chunk_bytes": 64,
      "ack_required": true
    },
    "service_hints": {
      "mcp_render_tool": "self.screen.matrix_16x16.render_prompt",
      "mcp_frame_tool": "self.screen.matrix_16x16.draw_frame",
      "mcp_drawing_tool": "self.screen.matrix_16x16.draw_python",
      "mcp_text_tool": "self.screen.matrix_16x16.show_text",
      "http_control_endpoint": "/control/matrix_prompt_16x16"
    }
  }
}
```

### 主机侧可用服务

本地 Python MCP 脚本已经补齐三条更适合 LLM 理解的主机侧能力：

1. `self.screen.matrix_16x16.draw_python`
作用：使用受限 Python 绘图语句直接生成一帧统一 `16x16` 图像结果，并返回 `frame_rgb332_hex + bitmap_rows_hex`。

2. `self.screen.matrix_16x16.show_text`
作用：把文字转成 `16x16` 单帧序列，并按统一格式逐帧显示到 `AI端` 预览区域。

3. `POST /control/matrix_prompt_16x16`
作用：主机收到 prompt 后，先本地渲染，再通过当前统一预览链路把结果发到 `AI端`。

示例：

```bash
curl -X POST http://127.0.0.1:8765/control/matrix_prompt_16x16 \
  -H "Content-Type: application/json" \
  -d "{\"prompt\":\"绘制一个青色爱心\"}"
```

### 接入建议

1. 语音/触摸入口只负责给出自然语言意图，不直接内联图像数据。
2. 服务端收到 `matrix_pattern_request` 后，可以优先调用 `self.screen.matrix_16x16.draw_python` 生成统一帧结果。
3. 如果服务端更适合走 HTTP，可直接向 `/control/matrix_prompt_16x16` 发起 POST。
4. 最终向 `LED端` 的传输固定使用 `FrameStart + FrameChunk + FrameCommit`，每片 `64` 字节，并在每一步等待 ACK。
5. 当服务端已经有 `bitmap_rows_hex + primary_rgb888/background_rgb888` 时，`AI端` 应优先把它编码成紧凑 `bitmap + RGB888` 蓝牙帧，再转发到 `LED端`，不要先膨胀成 `256` 字节 RGB332。

## English

The AI-side device now emits a dedicated `custom` request named `matrix_pattern_request` when the touch `Draw` button is pressed.

- Request purpose: ask the upstream LLM or orchestration layer to generate a `16x16 RGB332` pattern.
- Recommended MCP drawing tool: `self.screen.matrix_16x16.draw_python`
- Recommended MCP text tool: `self.screen.matrix_16x16.show_text`
- Recommended HTTP control endpoint: `POST /control/matrix_prompt_16x16`
- Final Bluetooth upload mode: `FrameStart + FrameChunk + FrameCommit`, `64` bytes per chunk, ACK required at each stage.
- When `bitmap_rows_hex` is available, the `AI side` should prefer compact `bitmap + RGB888` Bluetooth payloads over expanding to a full `256`-byte RGB332 frame.