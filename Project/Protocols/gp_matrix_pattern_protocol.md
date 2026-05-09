# GP Matrix Pattern Request Protocol

## Category

`蓝牙通信协议`

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
      "protocol_version": 2,
      "link": "bluetooth_frame_upload_v2",
      "chunk_bytes": 64,
      "chunk_offset": "byte_offset_le16",
      "reply_mode": "packet_type_reply",
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

本地 Python MCP 脚本已经补齐四条更适合 LLM 理解的主机侧能力，相关脚本位于 `Project/Script/`：

1. `self.screen.matrix_16x16.draw_python`
作用：使用受限 Python 绘图语句或受限 `eval()` 表达式直接生成一帧统一 `16x16` 图像结果，并返回 `matrix_frame_v1`；其中紧凑格式固定为 `bitmap_rows_hex(64 hex = 32 byte) + primary_rgb888(3 byte) + background_rgb888(3 byte) = 38 byte`。

2. `self.screen.matrix_16x16.show_text`
作用：把文字转成 `16x16` 单帧序列，并按统一格式逐帧显示到 `AI端` 预览区域。

3. `self.screen.matrix_16x16.draw_animation`
作用：把紧凑位图序列打包成适合 `LED端` 缓冲播放的 `matrix_frame_sequence_v1`。支持两种输入：`bitmap_rows_hex_list`（每项优先使用 `64` hex）或 `frames`（每帧可用 `bitmap_rows_hex`、`bitmap_rows(16行token)`、`python_source/eval_source` 三选一）。位图语义固定为 `1=亮`、`0=暗`、按 `top->bottom` 存 `16` 行、每行高位对应最左侧 LED，再与前景/背景 `RGB888` 一起组成 `38 byte` 紧凑帧。`frame_interval_ms` 支持 `1..65535 ms`，默认 `42 ms`；若主机侧收到超过 `24` 帧输入，桥接会重采样到 `24` 帧并同步调整间隔，再让 `AI端` 先本地缓存和循环预览。

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
4. 最终向 `LED端` 的单帧传输继续使用 `FrameStart + FrameChunk + FrameCommit`；`FrameChunk` / `ScrollGlyphChunk` 采用 `byte_offset_le16 + size` 前缀，每片数据部分固定最多 `64` 字节。对紧凑 `bitmap + RGB888` 单帧（当前仅 `38` 字节）优先只在 `FrameCommit` 等待最终 ACK。
5. 若任务是动画序列，主机应先通过 debug websocket 发送一次 `matrix_animation_start`，再发送带 `frame_index/frame_count` 的 `matrix_pattern_result` 帧列表，最后发送一次 `matrix_animation_end`；`AI端` 先在本地 `Preview` 缓冲并循环播放，收满后再通过蓝牙发送 `AnimationStart + AnimationFrame x N + AnimationEnd` 到 `LED端`。
6. 面向 `LLM` 的动画输入应优先使用 `frames` 模式或完整 `bitmap_rows_hex_list`（每项 `64` hex）；不要使用 `yield_frame`、`time.sleep`、`import` 之类“在绘图语句里自管时序”的方式。时序统一由 `frame_interval_ms` 控制。位图字段只传位图本体，不要传 `0/1` 数组、ASCII 图、二进制字符串，或误传 `256 byte` 的 `frame_rgb332_hex`。为减少偶发失败，桥接兼容常见的 `16` 行 `16-bit hex token` 写法并会在转发前统一归一化。
7. 当服务端已经有 `bitmap_rows_hex + primary_rgb888/background_rgb888` 时，`AI端` 应优先把它编码成紧凑 `bitmap + RGB888` 蓝牙帧，再转发到 `LED端`，不要先膨胀成 `256` 字节 RGB332。
8. 联调时应优先查看协议级 `[GP_TX] / [GP_RX] / [GP_DROP] / [GP_SYNC]` 日志；`[BT_MON]` 只是一段有上限的原始串口抓包窗口，可能会裁剪长包，不能单独用来判断整帧是否完整到达。
9. 若需要点亮 `LED端` 板载调试 LED，统一使用独立的 `SetDebugLed` 协议命令；不要再从 `AI端` 向 HC-05 发送裸 `LED n` 文本命令。
10. 若需要验证链路能否稳定持续收发，而不仅仅是收一条短 ACK，`AI端` 可通过 `1s` 周期任务持续发送 `SetDebugLed` 命令，在 `LED端` 板载调试 LED 上做 `0..7` 的流水灯。
11. 当前 `LED端` UART2 接收链路已针对长包分片做修正：主协议循环在进入 `TryPopByte()` 前统一执行一次 `UART2_ServiceRx()`，不再在逐字节出队过程中重复推进 DMA idle 重装判定，以减少 `FrameChunk` 尾字节被过早丢弃的风险。
12. `LED端` 成功显示通过蓝牙传输的图像后，会保持最后一帧输出；若收到动画批次，则会把最多 `32` 帧保存到本地缓冲区，并按 `AnimationStart` 包内的 `frame_interval_ms` 循环播放，默认间隔为 `42 ms`。
13. 从 V2 开始，主机侧匹配 ACK 时应使用 `packet_type=Reply` 和 `reply_to_sequence`，不要再假定存在固定 `Status/Error` 命令字或固定 `13` 字节回包。

## English

The AI-side device now emits a dedicated `custom` request named `matrix_pattern_request` when the touch `Draw` button is pressed.

- Request purpose: ask the upstream LLM or orchestration layer to generate a `16x16 RGB332` pattern.
- Recommended MCP drawing tool: `self.screen.matrix_16x16.draw_python`
- Recommended MCP text tool: `self.screen.matrix_16x16.show_text`
- Recommended MCP animation tool: `self.screen.matrix_16x16.draw_animation`
- Recommended HTTP control endpoint: `POST /control/matrix_prompt_16x16`
- Final Bluetooth upload mode: single frames still use `FrameStart + FrameChunk + FrameCommit`, while buffered animations use `AnimationStart + AnimationFrame x N + AnimationEnd`; chunked uploads now use an explicit little-endian byte offset plus chunk size, and compact single frames may rely on the final `FrameCommit` ACK only when the payload fits in one chunk.
- LLM-facing `bitmap_rows_hex` fields should carry only the bitmap portion: the canonical form is exactly `64` hex characters = `32` bytes for `16` rows of `16` bits, with `1 = on`, `0 = off`, `top row first`, and `MSB = leftmost LED`. The bridge also tolerates common `16`-row hex token lists and normalizes them back to the canonical form. The full compact LED-ready frame is `bitmap_rows_hex + primary_rgb888 + background_rgb888 = 38 bytes`.
- For animation sequences, the host should send `matrix_animation_start`, indexed `matrix_pattern_result` frames, and `matrix_animation_end` over the debug websocket. The `AI side` should cache and loop the preview locally before forwarding the full batch to the `LED side`.
- The LED-side animation buffer stores `32` frames and now uses a shared `frame_interval_ms` field in the start packet. Intervals support `1..65535 ms`; if the host receives more than `32` input frames, it should resample the sequence and scale the interval to preserve total duration.
- When `bitmap_rows_hex` is available, the `AI side` should prefer compact `bitmap + RGB888` Bluetooth payloads over expanding to a full `256`-byte RGB332 frame.
- For link debugging, rely on protocol-level `[GP_TX] / [GP_RX] / [GP_DROP] / [GP_SYNC]` logs first; `[BT_MON]` is only a bounded raw UART sniff window and may clip long packets.
- From protocol V2 onward, ACK matching should use `packet_type=Reply` plus `reply_to_sequence`; do not assume dedicated `Status/Error` commands or a fixed reply packet length.
- If the task needs to light the LED-side debug LEDs, use the dedicated `SetDebugLed` protocol command instead of sending raw `LED n` text over HC-05.
