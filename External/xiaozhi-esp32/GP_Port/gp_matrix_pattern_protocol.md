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
      "mcp_animation_tool": "self.screen.matrix_16x16.draw_animation",
      "mcp_text_tool": "self.screen.matrix_16x16.show_text",
      "http_control_endpoint": "/control/matrix_prompt_16x16"
    }
  }
}
```

### 主机侧可用服务

本地 Python MCP 脚本已经补齐四条更适合 LLM 理解的主机侧能力：

1. `self.screen.matrix_16x16.draw_python`
作用：使用受限 Python 绘图语句或受限 `eval()` 表达式直接生成一帧统一 `16x16` 图像结果，并返回 `frame_rgb332_hex + bitmap_rows_hex`。

2. `self.screen.matrix_16x16.show_text`
作用：把文字转成 `16x16` 单帧序列，并按统一格式逐帧显示到 `AI端` 预览区域。

3. `self.screen.matrix_16x16.draw_animation`
作用：把紧凑 `bitmap_rows_hex` 序列打包成适合 `LED端` 蓝牙播放的 `matrix_frame_sequence_v1`；做 `10 fps` 时，优先使用 `10` 帧、`100 ms` 间隔。

4. `POST /control/matrix_prompt_16x16`
作用：主机收到 prompt 后，先本地渲染，再通过当前统一预览链路把结果发到 `AI端`。

示例：

```bash
curl -X POST http://127.0.0.1:8765/control/matrix_prompt_16x16 \
  -H "Content-Type: application/json" \
  -d "{\"prompt\":\"绘制一个青色爱心\"}"
```

### 接入建议

1. 语音/触摸入口只负责给出自然语言意图，不直接内联图像数据。
2. 服务端收到 `matrix_pattern_request` 后，可以优先调用 `self.screen.matrix_16x16.draw_python` 生成统一帧结果；简单步骤优先放进 `python_source`，表达式/推导式优先放进 `eval_source`。
3. 如果服务端更适合走 HTTP，可直接向 `/control/matrix_prompt_16x16` 发起 POST。
4. 最终向 `LED端` 的传输固定使用 `FrameStart + FrameChunk + FrameCommit`，每片 `64` 字节；对紧凑 `bitmap + RGB888` 单帧（当前仅 `38` 字节）优先只在 `FrameCommit` 等待最终 ACK，以支持更低延迟的连续动画播放。
5. 当服务端已经有 `bitmap_rows_hex + primary_rgb888/background_rgb888` 时，`AI端` 应优先把它编码成紧凑 `bitmap + RGB888` 蓝牙帧，再转发到 `LED端`，不要先膨胀成 `256` 字节 RGB332。
6. 联调时应优先查看协议级 `[GP_TX] / [GP_RX] / [GP_DROP] / [GP_SYNC]` 日志；`[BT_MON]` 只是一段有上限的原始串口抓包窗口，可能会裁剪长包，不能单独用来判断整帧是否完整到达。
7. 若需要点亮 `LED端` 板载调试 LED，统一使用独立的 `SetDebugLed` 协议命令；不要再从 `AI端` 向 HC-05 发送裸 `LED n` 文本命令。
8. 若需要验证链路能否稳定持续收发，而不仅仅是收一条短 ACK，`AI端` 可通过 `1s` 周期任务持续发送 `SetDebugLed` 命令，在 `LED端` 板载调试 LED 上做 `0..7` 的流水灯。
9. 当前 `LED端` UART2 接收链路已针对长包分片做修正：主协议循环在进入 `TryPopByte()` 前统一执行一次 `UART2_ServiceRx()`，不再在逐字节出队过程中重复推进 DMA idle 重装判定，以减少 `FrameChunk` 尾字节被过早丢弃的风险。
10. `LED端` 成功显示通过蓝牙传输的图像后，会保持最后一帧输出；后续只有显式释放远程模式或本地切换控制模式时才会退出该显示。

## English

The AI-side device now emits a dedicated `custom` request named `matrix_pattern_request` when the touch `Draw` button is pressed.

- Request purpose: ask the upstream LLM or orchestration layer to generate a `16x16 RGB332` pattern.
- Recommended MCP drawing tool: `self.screen.matrix_16x16.draw_python`
- Recommended MCP text tool: `self.screen.matrix_16x16.show_text`
- Recommended MCP animation tool: `self.screen.matrix_16x16.draw_animation`
- Recommended HTTP control endpoint: `POST /control/matrix_prompt_16x16`
- Final Bluetooth upload mode: `FrameStart + FrameChunk + FrameCommit`, `64` bytes per chunk; compact `bitmap + RGB888` frames may rely on the final `FrameCommit` ACK only when the payload fits in one chunk.
- When `bitmap_rows_hex` is available, the `AI side` should prefer compact `bitmap + RGB888` Bluetooth payloads over expanding to a full `256`-byte RGB332 frame.
- For link debugging, rely on protocol-level `[GP_TX] / [GP_RX] / [GP_DROP] / [GP_SYNC]` logs first; `[BT_MON]` is only a bounded raw UART sniff window and may clip long packets.
- If the task needs to light the LED-side debug LEDs, use the dedicated `SetDebugLed` protocol command instead of sending raw `LED n` text over HC-05.