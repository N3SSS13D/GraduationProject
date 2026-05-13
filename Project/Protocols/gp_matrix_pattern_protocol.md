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
      "mcp_scroll_subtitle_tool": "self.screen.matrix_16x16.show_scroll_subtitle",
      "mcp_effect_tool": "self.screen.matrix_16x16.show_effect",
      "http_control_endpoint": "/control/matrix_prompt_16x16"
    }
  }
}
```

若 `transcript` 明确属于“字幕 / 跑马灯 / scroll”语义，`AI端` 还会在本次请求里追加：

- `service_hints.preferred_mcp_tool = self.screen.matrix_16x16.show_scroll_subtitle`
- `animation_guidance.mode = scroll_subtitle`
- `animation_guidance.transport_preference = frame_sequence_plus_effect`
- `animation_guidance.recommended_effect = text_scroll`

### 主机侧可用服务

本地 Python MCP 脚本已经补齐六条更适合 LLM 理解的主机侧能力，相关脚本位于 `Project/Script/`：

1. `self.screen.matrix_16x16.draw_python`
作用：使用受限 Python 绘图语句或受限 `eval()` 表达式直接生成一帧统一 `16x16` 图像结果，并返回 `matrix_frame_v1`；其中紧凑格式固定为 `bitmap_rows_hex(64 hex = 32 byte) + primary_rgb888(3 byte) + background_rgb888(3 byte) = 38 byte`。

2. `self.screen.matrix_16x16.show_text`
作用：把文字转成 `16x16` 单帧序列，并按统一格式逐帧显示到 `AI端` 预览区域。

3. `self.screen.matrix_16x16.show_scroll_subtitle`
作用：把原始字幕文字先渲染为离屏长位图，再按 `text_scroll / scroll_left / scroll_right` 这类横向效果参数切成 `matrix_frame_sequence_v1` / `v2` 动画批次。适用于“图像序列 + 效果参数”的字幕传输，不需要调用方手工生成每帧位图。

4. `self.screen.matrix_16x16.show_effect`
作用：当任务可直接映射为 `LED端` 原生动作时，不再重新绘制动画帧，而是直接生成 `matrix_action_result`。当前支持纯色、内置图案、单字模效果，以及多字滚动字幕（`text_scroll` / `scroll_left` / `scroll_right`）三类原生直连；若是文字淡入淡出、逐行显隐等无法由当前 `LED端` 序列字模路径直接表达的多字效果，仍应回退到 `draw_animation`。

5. `self.screen.matrix_16x16.draw_animation`
作用：把紧凑位图序列打包成适合 `LED端` 缓冲播放的 `matrix_frame_sequence_v1`。支持两种输入：`bitmap_rows_hex_list`（每项优先使用 `64` hex）或 `frames`（每帧可用 `bitmap_rows_hex`、`bitmap_rows(16行token)`、`python_source/eval_source` 三选一）。位图语义固定为 `1=亮`、`0=暗`、按 `top->bottom` 存 `16` 行、每行高位对应最左侧 LED，再与前景/背景 `RGB888` 一起组成 `38 byte` 紧凑帧。`frame_interval_ms` 支持 `1..65535 ms`，默认 `42 ms`；若主机侧收到超过 `24` 帧输入，桥接会重采样到 `24` 帧并同步调整间隔，再让 `AI端` 先本地缓存和循环预览。

6. `POST /control/matrix_prompt_16x16`
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
4. 若任务可以映射为 `LED端` 原生动作（纯色、内置图案、滚动字幕、单字淡入淡出/呼吸等），优先调用 `self.screen.matrix_16x16.show_effect`，并通过 debug websocket 发送 `matrix_action_result`；若带自定义字幕字模，主机同时附带 `glyph_rows_hex`，再由 `AI端` 先下发字模、后下发 `SetAction`。
补充：若上游不是通过专用 debug websocket 回传，而是通过主会话 websocket 回复，也应保持 `type:"custom"` 外壳，并把 `matrix_pattern_result` / `matrix_action_result` / `matrix_animation_*` 放进 `payload.type`；兼容旧写法时也可放进 `payload.action`。`AI端` 会把这两种字段归一化到同一板级解析器。
5. 若请求明确要求“字幕通过图像序列传输”，或 `AI端` 在 `matrix_pattern_request` 里给出了 `mcp_scroll_subtitle_tool` / `preferred_mcp_tool=self.screen.matrix_16x16.show_scroll_subtitle`，主机应优先调用 `self.screen.matrix_16x16.show_scroll_subtitle`，不要手工展开字幕帧。
6. 若需要自定义多帧动画而不是纯字幕滚动，继续使用 `self.screen.matrix_16x16.draw_animation`。
7. 最终向 `LED端` 的单帧传输继续使用 `FrameStart + FrameChunk + FrameCommit`；`FrameChunk` / `ScrollGlyphChunk` 采用 `byte_offset_le16 + size` 前缀，每片数据部分固定最多 `64` 字节。对紧凑 `bitmap + RGB888` 单帧（当前仅 `38` 字节）优先只在 `FrameCommit` 等待最终 ACK。
8. 若任务是动画序列，主机应先通过 debug websocket 发送一次 `matrix_animation_start`，再发送带 `frame_index/frame_count` 的 `matrix_pattern_result` 帧列表，最后发送一次 `matrix_animation_end`；`AI端` 先在本地 `Preview` 缓冲并循环播放，收满后再通过蓝牙发送 `AnimationStart + AnimationFrame x N + AnimationEnd` 到 `LED端`。
9. 面向 `LLM` 的动画输入应优先使用 `frames` 模式或完整 `bitmap_rows_hex_list`（每项 `64` hex）；不要使用 `yield_frame`、`time.sleep`、`import` 之类“在绘图语句里自管时序”的方式。时序统一由 `frame_interval_ms` 控制。位图字段只传位图本体，不要传 `0/1` 数组、ASCII 图、二进制字符串，或误传 `256 byte` 的 `frame_rgb332_hex`。为减少偶发失败，桥接兼容常见的 `16` 行 `16-bit hex token` 写法并会在转发前统一归一化。
10. 当服务端已经有 `bitmap_rows_hex + primary_rgb888/background_rgb888` 时，`AI端` 应优先把它编码成紧凑 `bitmap + RGB888` 蓝牙帧，再转发到 `LED端`，不要先膨胀成 `256` 字节 RGB332。
11. 联调时应优先查看协议级 `[GP_TX] / [GP_RX] / [GP_DROP] / [GP_SYNC]` 日志；`[BT_MON]` 只是一段有上限的原始串口抓包窗口，可能会裁剪长包，不能单独用来判断整帧是否完整到达。
12. 若需要点亮 `LED端` 板载调试 LED，统一使用独立的 `SetDebugLed` 协议命令；不要再从 `AI端` 向 HC-05 发送裸 `LED n` 文本命令。
13. 若需要验证链路能否稳定持续收发，而不仅仅是收一条短 ACK，`AI端` 可通过 `1s` 周期任务持续发送 `SetDebugLed` 命令，在 `LED端` 板载调试 LED 上做 `0..7` 的流水灯。
14. 当前 `LED端` UART2 接收链路已针对长包分片做修正：主协议循环在进入 `TryPopByte()` 前统一执行一次 `UART2_ServiceRx()`，不再在逐字节出队过程中重复推进 DMA idle 重装判定，以减少 `FrameChunk` 尾字节被过早丢弃的风险。
15. `LED端` 成功显示通过蓝牙传输的图像后，会保持最后一帧输出；若收到动画批次，则会把最多 `32` 帧保存到本地缓冲区，并按 `AnimationStart` 包内的 `frame_interval_ms` 循环播放，默认间隔为 `42 ms`。
16. 从 V2 开始，主机侧匹配 ACK 时应使用 `packet_type=Reply` 和 `reply_to_sequence`，不要再假定存在固定 `Status/Error` 命令字或固定 `13` 字节回包。

补充：对于语音触发的 `matrix_pattern_request`，`AI端` 会保留一个短暂的 pending 窗口；若上游只返回 TTS 文本而未及时给出 `matrix_*` 结果，设备会提示仍在等待矩阵结果并回到空闲态，而不是继续停留在 `listening`。

## English

The AI-side device now emits a dedicated `custom` request named `matrix_pattern_request` when the touch `Draw` button is pressed.

- Request purpose: ask the upstream LLM or orchestration layer to generate a `16x16 RGB332` pattern.
- Recommended MCP drawing tool: `self.screen.matrix_16x16.draw_python`
- Recommended MCP text tool: `self.screen.matrix_16x16.show_text`
- Recommended MCP scroll subtitle tool: `self.screen.matrix_16x16.show_scroll_subtitle`
- Recommended MCP native effect tool: `self.screen.matrix_16x16.show_effect`
- Recommended MCP animation tool: `self.screen.matrix_16x16.draw_animation`
- Recommended HTTP control endpoint: `POST /control/matrix_prompt_16x16`
- For transcript-driven scroll subtitle requests, the AI side may also include `service_hints.preferred_mcp_tool = self.screen.matrix_16x16.show_scroll_subtitle` plus `animation_guidance` fields such as `mode = scroll_subtitle` and `recommended_effect = text_scroll`.
- When the request can be represented by an LED-side native effect (solid color, built-in pattern, direct text scroll, single-glyph reveal/fade), the host should prefer `matrix_action_result` over synthesizing a bitmap animation. If custom subtitle glyphs are needed, the host includes `glyph_rows_hex` and the AI side uploads glyph rows before forwarding `SetAction`.
- If the upstream service replies on the main conversation websocket instead of the dedicated debug websocket, keep the outer message as `type:"custom"` and place `matrix_pattern_result`, `matrix_action_result`, or `matrix_animation_*` in `payload.type`; `payload.action` remains accepted for compatibility. The AI side normalizes both fields into the same board-level parser.
- Final Bluetooth upload mode: single frames still use `FrameStart + FrameChunk + FrameCommit`, while buffered animations use `AnimationStart + AnimationFrame x N + AnimationEnd`; chunked uploads now use an explicit little-endian byte offset plus chunk size, and compact single frames may rely on the final `FrameCommit` ACK only when the payload fits in one chunk.
- LLM-facing `bitmap_rows_hex` fields should carry only the bitmap portion: the canonical form is exactly `64` hex characters = `32` bytes for `16` rows of `16` bits, with `1 = on`, `0 = off`, `top row first`, and `MSB = leftmost LED`. The bridge also tolerates common `16`-row hex token lists and normalizes them back to the canonical form. The full compact LED-ready frame is `bitmap_rows_hex + primary_rgb888 + background_rgb888 = 38 bytes`.
- For animation sequences, the host should send `matrix_animation_start`, indexed `matrix_pattern_result` frames, and `matrix_animation_end` over the debug websocket. The `AI side` should cache and loop the preview locally before forwarding the full batch to the `LED side`.
- The LED-side animation buffer stores `32` frames and now uses a shared `frame_interval_ms` field in the start packet. Intervals support `1..65535 ms`; if the host receives more than `32` input frames, it should resample the sequence and scale the interval to preserve total duration.
- When `bitmap_rows_hex` is available, the `AI side` should prefer compact `bitmap + RGB888` Bluetooth payloads over expanding to a full `256`-byte RGB332 frame.
- For link debugging, rely on protocol-level `[GP_TX] / [GP_RX] / [GP_DROP] / [GP_SYNC]` logs first; `[BT_MON]` is only a bounded raw UART sniff window and may clip long packets.
- From protocol V2 onward, ACK matching should use `packet_type=Reply` plus `reply_to_sequence`; do not assume dedicated `Status/Error` commands or a fixed reply packet length.
- If the task needs to light the LED-side debug LEDs, use the dedicated `SetDebugLed` protocol command instead of sending raw `LED n` text over HC-05.
- For voice-triggered matrix requests, the AI side keeps a short pending window after sending the request; if only TTS arrives and no `matrix_*` result follows promptly, the device returns to idle with a pending hint instead of re-entering listening.
