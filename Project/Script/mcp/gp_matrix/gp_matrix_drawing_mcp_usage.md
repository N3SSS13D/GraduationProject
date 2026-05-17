# GP Matrix Display — MCP 绘图工具知识库

## 0. 项目概述

本知识库为小智AI提供 GP Matrix Display MCP 桥接服务的完整工具参考。桥接服务 (`gp_display_mcp_bridge.py`) 作为 MCP 端点客户端运行，将大模型的绘图指令转换为 16×16 LED 点阵屏上的实时显示内容。

**运行方式：**
```bash
python Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py
```

**相关协议文档：**
- `Project/Protocols/gp_led_matrix_protocol_spec.md`
- `Project/Protocols/gp_matrix_pattern_protocol.md`

**模块分工：**
| 模块 | 职责 |
|------|------|
| `gp_display_mcp_bridge.py` | 总入口脚本 |
| `gp_mcp_endpoint_client.py` | MCP 编排与工具分发 |
| `gp_matrix_llm_inputs.py` | 绘图输入归一化 |
| `gp_bridge_mcp_service.py` | MCP Schema 辅助块 |
| `gp_bridge_transport_service.py` | 传输与运行时参数解析 |

## 0.1 主机桥工具与 AI 端本地工具边界

- 主机脚本桥接对外暴露的 MCP 工具命名空间是 `self.screen.matrix_16x16.*`。
- `self.screen.matrix_16x16.local.*` 是 `AI端` 板内本地调试工具，只能用于设备本地直连绘制，不属于主机脚本桥的可调用工具集合。
- 允许在主机侧输出的工具包括：`draw_frame`、`draw_python`、`show_text`、`show_scroll_subtitle`、`show_effect`、`draw_animation`、`render_prompt`。
- 禁止在主机 MCP 请求、协议说明或滚动字幕模板中使用 `self.screen.matrix_16x16.local.draw`、`self.screen.matrix_16x16.local.draw_frame` 这类本地工具名。

## 1. 全局约束（始终生效）

- Matrix resolution is always `16x16`.
- Image format: **BITMAP_LAYERED** (multi-layer bitmap).
  - Each layer = `1-byte header` + `32-byte bitmap` + `3-byte RGB888 color` = **36 bytes**.
  - Header byte: high 4 bits = total layer count, low 4 bits = layer index (0-based).
  - Layers are ordered back-to-front: layer 0 is background, higher layers paint on top.
  - A 2-color image uses 2 layers (72 bytes total).
- `bitmap_rows_hex` is the bitmap portion of one layer.
  - Canonical form: exactly `64` hex characters (`16 rows x 16 bits = 32 bytes`).
- `primary_rgb888` maps to the foreground color (layer 1).
- `background_rgb888` maps to the background color (layer 0), default `#000000`.
- `bitmap_ascii` is an LLM-friendly alias for one frame:
  - exactly `16` rows x `16` chars.
  - on-pixels: `1/#/X/*/@`; off-pixels: `0/./_/-/space`.
- `ops` is an LLM-friendly structured draw list:
  - each item includes `op` and operation-specific numeric fields.
- Bitmap semantics are fixed:
  - `1 = LED on` (paint this layer's color), `0 = LED off` (transparent).
  - Rows are `top -> bottom`.
  - In each row, high bit (`bit15`) is the leftmost LED.
- Do not pass `0/1` arrays, ASCII art, spaced binary strings, or `256-byte frame_rgb332_hex` into `bitmap_rows_hex` fields.
- Do not pass PIL `Image` objects, base64 image blobs, or other full-color image payloads into `bitmap_rows_hex`; use `bitmap_ascii`, `ops`, or `python_source` instead.
- Include `source` and `transcript` whenever possible for traceability.
- `draw_frame` / `draw_animation` / `draw_python` 仍以 bitmap 为主格式；若任务可直接映射为 `LED端` 原生效果命令，则使用 `show_effect` 生成 `matrix_action_v2`，并可附带 `glyph_rows_hex` 做字幕字模上传。
- `draw_frame` requires `bitmap_rows_hex` + `primary_rgb888` (+ optional `background_rgb888`).
- Buffered animation playback uses one shared `frame_interval_ms` per batch.
- If animation input has more than `32` frames, host bridge resamples to `32` and scales interval to preserve total duration.

## 2. 工具选择决策树

1. Need one custom pattern from code-like instructions:
   - Use `self.screen.matrix_16x16.draw_python`.
  - Prefer `ops` first; only use `python_source` / `eval_source` when structured ops cannot express the pattern.
2. Need one LED-side native effect on solid color, built-in pattern, or direct subtitle text:
  - Use `self.screen.matrix_16x16.show_effect`.
3. Need scrolling subtitle with `图像序列 + 效果参数` transport:
  - Use `self.screen.matrix_16x16.show_scroll_subtitle`.
4. Need per-character text sequence:
  - Use `self.screen.matrix_16x16.show_text`.
5. Need buffered LED-side animation sequence from existing frames or a base image:
   - Use `self.screen.matrix_16x16.draw_animation`.
   - For per-frame drawing control, use `ops_sequence` mode.
   - For simple effect from one base image, use `image + effect` mode.
6. Already have one final bitmap or full RGB332 frame:
   - Use `self.screen.matrix_16x16.draw_frame`.
7. Only have vague natural language prompt and no explicit draw plan:
   - Use `self.screen.matrix_16x16.render_prompt`.

## 3. 工具详细约定

### 3.1 `self.screen.matrix_16x16.draw_python` — 代码绘图

Use when generating one frame from restricted drawing logic.

#### Input (show_text)

```json
{
  "python_source": "for i in range(16):\n    point(i, i)\n    point(15 - i, i)",
  "eval_source": "[(fill_rectangle(x, 0, x + 1, 1), fill_rectangle(x, 14, x + 1, 15)) for x in range(0, 16, 4)]",
  "primary_rgb888": "#00FF66",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw a green X with top and bottom bars"
}
```

At least one of `python_source`, `eval_source`, or `ops` is required.

#### LLM-safe structured mode (`ops`)

```json
{
  "ops": [
    {"op": "clear", "fill": 0},
    {"op": "line", "x0": 0, "y0": 0, "x1": 15, "y1": 15, "width": 1},
    {"op": "line", "x0": 15, "y0": 0, "x1": 0, "y1": 15, "width": 1},
    {"op": "fill_circle", "cx": 8, "cy": 8, "radius": 2}
  ],
  "primary_rgb888": "#00FF66",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw X plus center circle"
}
```

Supported `op` values:

- `clear`
- `point`
- `line`
- `rectangle`
- `fill_rectangle`
- `ellipse`
- `fill_ellipse`
- `circle`
- `fill_circle`
- `polygon`

#### Execution model

- `python_source` runs as restricted statements (`exec`).
- `eval_source` runs as restricted expression (`eval`).
- If both exist: run `python_source` first, then `eval_source` in same scope.
- The bridge already creates the `16x16` canvas and exposes `draw` plus helper functions; do not create `Image.new(...)`, `ImageDraw.Draw(...)`, or `ImageFont.truetype(...)` inside tool input.

#### Allowed helper names

- `range`, `min`, `max`, `abs`, `int`, `eval`
- `clear`, `point`, `line`, `rectangle`, `fill_rectangle`
- `ellipse`, `fill_ellipse`, `circle`, `fill_circle`, `polygon`

#### Allowed `draw.<method>`

- `draw.point`, `draw.line`, `draw.rectangle`, `draw.ellipse`
- `draw.polygon`, `draw.arc`, `draw.pieslice`, `draw.chord`

#### Forbidden constructs

- `import`, `while`, function/class definitions, async comprehensions.
- Never write `from PIL import Image`, `import PIL`, `Image.new(...)`, or other PIL object construction inside `draw_python` input.
- File/network access.
- Attribute access other than allowed `draw.<method>`.

### 3.2 `self.screen.matrix_16x16.show_text` — 文字逐字显示

Use when each character should become one frame.

#### Input (draw_frame)

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

#### Rules

- One character -> one `16x16` frame.
- `frame_interval_ms` controls sequence playback interval.
- 如果目标是连续滚动字幕，而不是逐字翻页，请改用 `self.screen.matrix_16x16.show_scroll_subtitle`。

### 3.2.1 `self.screen.matrix_16x16.show_scroll_subtitle` — 滚动字幕序列

Use when the input is raw subtitle text and the transport must stay `matrix_frame_sequence_v2` instead of one native `matrix_action_v2` effect command.

#### Input

```json
{
  "text": "吉林大学欢迎你",
  "effect": {
    "name": "text_scroll",
    "step": 1,
    "glyph_spacing": 1,
    "leading_padding": 16,
    "trailing_padding": 16
  },
  "frame_interval_ms": 96,
  "primary_rgb888": "#00FFAA",
  "background_rgb888": "#000000",
  "source": "mcp_scroll_subtitle",
  "transcript": "滚动字幕 吉林大学欢迎你"
}
```

#### Rules

- 主机桥会先把整段文字光栅化为一条离屏位图带，再截取连续 `16x16` 窗口生成动画帧。
- 英文/ASCII 字符会额外保留 side-bearing 边界，默认比旧实现更易分辨；除非明确需要宽字距，否则保持较小 `glyph_spacing` 即可。
- `effect.name` 仅支持横向滚动别名：`text_scroll`、`scroll_left`、`scroll_right`、`marquee_left`、`marquee_right`。
- `step` 是每帧平移的像素步长；若省略 `frame_count`，桥会按完整滚动路径自动生成足够帧数，再按 `32` 帧上限重采样。
- 若任务只需要 `LED端` 原生字幕滚动，不要求图像序列传输，优先使用 `self.screen.matrix_16x16.show_effect`。
- 若你已经有明确帧列表，或需要多字淡入淡出、逐行显隐等非横向滚动效果，再使用 `self.screen.matrix_16x16.draw_animation`。

### 3.2.2 `self.screen.matrix_16x16.show_effect` — LED 端原生效果命令

Use when the task can be represented by one LED-side native action instead of a pre-rendered frame sequence.

#### Input

```json
{
  "effect": "scroll_left",
  "text": "吉林大学",
  "primary_rgb888": "#00FFAA",
  "frame_interval_ms": 96,
  "source": "mcp_effect",
  "transcript": "scroll 吉林大学 from right to left"
}
```

#### Supported content sources

- `solid`: omit `text` and `pattern_name`.
- `pattern`: provide `pattern_name` or `pattern_id`.
- `glyph`: provide `text`; bridge converts each character to one uploaded `16x16` glyph.

#### Supported effects

- `static`
- `breath`
- `gradient`
- `scroll_left`
- `scroll_right`
- `text_scroll`
- `fade_in`
- `fade_out`
- `color_cycle`
- `row_reveal`
- `row_hide`
- `gradient_reveal`

#### Rules

- Multi-character direct text is currently supported only for `text_scroll`, `scroll_left`, and `scroll_right`.
- Single-character text can use any supported native effect.
- Built-in pattern names: `diamond`, `cross`, `jlu_emblem`, `checker`, `border`, `diagonal_x`.
- If the effect uses gradient color mode and `secondary_rgb888` is omitted, the bridge falls back to black.
- Default timing values are aligned with the LED-side local defaults:
  - `scroll_*` / `text_scroll`: `frame_interval_ms = 96`
  - `breath`: `frame_interval_ms = 64`, `timeline = 1600 / 240 / breath_curve`
  - `fade_*`: `frame_interval_ms = 64`, `timeline = 1200 / 200 / ease_in_out`
  - `row_reveal` / `row_hide` / `gradient_reveal`: `frame_interval_ms = 64`, `timeline = 960 / 120 / linear`
  - `color_cycle`: `frame_interval_ms = 80`
- Transport shape: the bridge emits `matrix_action_result`; if `text` is present, it also includes `glyph_rows_hex` for AI-side glyph upload before `SetAction`.
- If the request explicitly needs subtitle frame-sequence transport, fall back to `show_scroll_subtitle`; if the desired text effect still cannot be expressed there, fall back to `draw_animation`.

### 3.3 `self.screen.matrix_16x16.draw_animation` — 多帧动画

Use for multi-frame buffered animation when native `show_effect` cannot express the required result.

- 若输入只有“文本 + 横向滚动参数”，优先使用 `self.screen.matrix_16x16.show_scroll_subtitle`，不要手工展开字幕位图帧。

#### Input mode A: `bitmap_rows_hex_list`

Each item should be one full frame (`64` hex chars).

```json
{
  "bitmap_rows_hex_list": [
    "018003c007e00ff01ff83ffc7ffe3ffc1ff80ff007e003c00180",
    "00c001e003f007f80ffc1ffe3fff1ffe0ffc07f003f001e000c0"
  ],
  "frame_interval_ms": 42,
  "primary_rgb888": "#3DDC97",
  "background_rgb888": "#000000",
  "source": "mcp_animation",
  "transcript": "play pulse animation"
}
```

#### Input mode B: `frames` (recommended for LLM)

Each frame must provide exactly one source:

- `bitmap_rows_hex`
- `bitmap_rows` (16 row tokens)
- `bitmap_ascii` (16 lines x 16 chars)
- `python_source` / `eval_source` / `ops`

```json
{
  "frames": [
    {
      "bitmap_ascii": [
        "................",
        "................",
        "....######......",
        "...########.....",
        "..###....###....",
        "..##......##....",
        "..##......##....",
        "..###....###....",
        "...########.....",
        "....######......",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................"
      ]
    },
    {
      "ops": [
        {"op": "fill_rectangle", "x0": 0, "y0": 0, "x1": 15, "y1": 15}
      ],
      "primary_rgb888": "#00FF00",
      "background_rgb888": "#000000"
    }
  ],
  "frame_interval_ms": 420,
  "primary_rgb888": "#00FF00",
  "background_rgb888": "#000000",
  "source": "mcp_animation",
  "transcript": "blink green full frame"
}
```

#### Input mode C: `image + effect` (AI-side effect synthesis)

Use this when LLM has one base image and wants animation by effect parameters.

```json
{
  "image": {
    "bitmap_ascii": [
      "................",
      "................",
      "....######......",
      "...########.....",
      "..###....###....",
      "..##......##....",
      "..##......##....",
      "..###....###....",
      "...########.....",
      "....######......",
      "................",
      "................",
      "................",
      "................",
      "................",
      "................"
    ]
  },
  "effect": {
    "name": "marquee_left",
    "frame_count": 16,
    "step": 1
  },
  "frame_interval_ms": 70,
  "primary_rgb888": "#00FF88",
  "background_rgb888": "#000000",
  "source": "mcp_animation",
  "transcript": "image plus marquee effect"
}
```

Supported `effect.name` (14 effects):

| Effect | Description | Key parameters |
|---|---|---|
| `blink` | On/off toggle | `duty_cycle` (0..1) |
| `flash` | Brief flash | `on_count` (frames per flash) |
| `wipe_left` | Reveal from left | — |
| `wipe_right` | Reveal from right | — |
| `wipe_up` | Reveal from top | — |
| `wipe_down` | Reveal from bottom | — |
| `marquee_left` | Scroll left | `step` (px/frame) |
| `marquee_right` | Scroll right | `step` (px/frame) |
| `scroll_up` | Vertical scroll up | `step` (px/frame) |
| `scroll_down` | Vertical scroll down | `step` (px/frame) |
| `breathe` | Density breathing | `min_density`, `max_density` |
| `fade_in` | Density 0→1 | — |
| `fade_out` | Density 1→0 | — |
| `pulse` | Grow/shrink from center | `min_scale`, `max_scale` |

Common parameters:
- `frame_count`: generated frame count (2..96, default 16).
- `step`: shift step (1..16, default 1).
- `duty_cycle`: 0..1 ratio for blink.
- `min_density`/`max_density`: 0..1 for breathe/fade.
- `min_scale`/`max_scale`: 0.1..1.0 for pulse.

### 3.3.1 `ops_sequence` — 逐帧 ops 动画

Use to specify each animation frame as an individual ops array:

```json
{
  "ops_sequence": [
    [{"op": "clear", "fill": 0}, {"op": "fill_circle", "cx": 7, "cy": 7, "radius": 3}],
    [{"op": "clear", "fill": 0}, {"op": "fill_circle", "cx": 8, "cy": 8, "radius": 3}],
    [{"op": "clear", "fill": 0}, {"op": "fill_circle", "cx": 8, "cy": 8, "radius": 4}]
  ],
  "frame_interval_ms": 120,
  "primary_rgb888": "#00FF88",
  "background_rgb888": "#000000",
  "source": "mcp_animation",
  "transcript": "circle pulse via ops_sequence"
}
```

Each inner array is a complete frame's ops — same format as `draw_python` ops.

#### Compatibility fallback

If `bitmap_rows_hex_list` itself is exactly `16` row tokens, bridge treats it as one frame.

If a frame uses `bitmap_ascii`, bridge normalizes it to canonical `bitmap_rows_hex` before transport.

If `image + effect` is used, bridge synthesizes frames on AI-side path before LED-side buffered playback.

#### Hard constraints

- `frame_interval_ms` range: `1..65535`.
- `42` is near-`24 fps` default.
- `420` is a good obvious blink default.

#### Forbidden animation authoring patterns

- Do not call `yield_frame(...)`.
- Do not call `time.sleep(...)`.
- Do not write any `import` statements in drawing code.
- Do not split one frame into `16` items in `bitmap_rows_hex_list` unless each item is full `64`-hex frame.
- Do not manually rasterize raw subtitle text into `bitmap_rows_hex_list` when `show_scroll_subtitle` already matches the request.

### 3.4 `self.screen.matrix_16x16.draw_frame` — 直接提交位图帧

Use when you already have final frame data. **Bitmap-only** — preset and frame_rgb332_hex are no longer supported.

#### Input

```json
{
  "bitmap_rows_hex": "8001400220041008081004200240018001800240042008101008200440028001",
  "primary_rgb888": "#F5F5F5",
  "background_rgb888": "#000000",
  "source": "mcp_frame",
  "transcript": "draw a white X from bitmap"
}
```

#### Contract

- `bitmap_rows_hex`: exactly `64` hex chars (**required**).
- `bitmap_ascii`: accepted as alias, normalized to `bitmap_rows_hex`.
- `primary_rgb888`: foreground color in `#RRGGBB` (**required**).
- `background_rgb888`: background color in `#RRGGBB` (optional, defaults to `#000000`).
- Internally converted to BITMAP_LAYERED format (2 layers: background + foreground).

### 3.5 `self.screen.matrix_16x16.render_prompt` — 自然语言渲染

Use as a fallback when no explicit drawing plan exists.

```json
{
  "prompt": "draw a cyan heart",
  "primary_rgb888": "#14B8A6",
  "background_rgb888": "#000000",
  "source": "mcp_prompt",
  "transcript": "draw a cyan heart"
}
```

## 4. 动画安全模板

All templates below are safe defaults for direct LLM use.

### 4.1 模板：慢速闪烁

```json
{
  "tool": "self.screen.matrix_16x16.draw_animation",
  "arguments": {
    "frame_interval_ms": 420,
    "primary_rgb888": "#00FF00",
    "background_rgb888": "#000000",
    "source": "mcp_animation",
    "transcript": "slow blink green",
    "frames": [
      { "python_source": "clear(0)" },
      { "python_source": "fill_rectangle(0, 0, 15, 15)" }
    ]
  }
}
```

### 4.2 模板：呼吸脉冲

```json
{
  "tool": "self.screen.matrix_16x16.draw_animation",
  "arguments": {
    "frame_interval_ms": 120,
    "primary_rgb888": "#00FF88",
    "background_rgb888": "#000000",
    "source": "mcp_animation",
    "transcript": "breathing pulse",
    "frames": [
      { "python_source": "clear(0); fill_circle(8, 8, 1)" },
      { "python_source": "clear(0); fill_circle(8, 8, 2)" },
      { "python_source": "clear(0); fill_circle(8, 8, 3)" },
      { "python_source": "clear(0); fill_circle(8, 8, 4)" },
      { "python_source": "clear(0); fill_circle(8, 8, 5)" },
      { "python_source": "clear(0); fill_circle(8, 8, 4)" },
      { "python_source": "clear(0); fill_circle(8, 8, 3)" },
      { "python_source": "clear(0); fill_circle(8, 8, 2)" }
    ]
  }
}
```

### 4.3 模板：跑马灯扫描

```json
{
  "tool": "self.screen.matrix_16x16.draw_animation",
  "arguments": {
    "frame_interval_ms": 70,
    "primary_rgb888": "#00FF00",
    "background_rgb888": "#000000",
    "source": "mcp_animation",
    "transcript": "marquee scan",
    "frames": [
      { "python_source": "clear(0); fill_rectangle(0, 0, 0, 15)" },
      { "python_source": "clear(0); fill_rectangle(1, 0, 1, 15)" },
      { "python_source": "clear(0); fill_rectangle(2, 0, 2, 15)" },
      { "python_source": "clear(0); fill_rectangle(3, 0, 3, 15)" },
      { "python_source": "clear(0); fill_rectangle(4, 0, 4, 15)" },
      { "python_source": "clear(0); fill_rectangle(5, 0, 5, 15)" },
      { "python_source": "clear(0); fill_rectangle(6, 0, 6, 15)" },
      { "python_source": "clear(0); fill_rectangle(7, 0, 7, 15)" },
      { "python_source": "clear(0); fill_rectangle(8, 0, 8, 15)" },
      { "python_source": "clear(0); fill_rectangle(9, 0, 9, 15)" },
      { "python_source": "clear(0); fill_rectangle(10, 0, 10, 15)" },
      { "python_source": "clear(0); fill_rectangle(11, 0, 11, 15)" },
      { "python_source": "clear(0); fill_rectangle(12, 0, 12, 15)" },
      { "python_source": "clear(0); fill_rectangle(13, 0, 13, 15)" },
      { "python_source": "clear(0); fill_rectangle(14, 0, 14, 15)" },
      { "python_source": "clear(0); fill_rectangle(15, 0, 15, 15)" }
    ]
  }
}
```

### 4.4 模板：图像+闪烁特效

```json
{
  "tool": "self.screen.matrix_16x16.draw_animation",
  "arguments": {
    "image": {
      "python_source": "clear(0); fill_circle(8, 8, 5)"
    },
    "effect": {
      "name": "blink",
      "frame_count": 12,
      "duty_cycle": 0.5
    },
    "frame_interval_ms": 160,
    "primary_rgb888": "#3DDC97",
    "background_rgb888": "#000000",
    "source": "mcp_animation",
    "transcript": "image blink effect"
  }
}
```

### 4.5 模板：图像+呼吸特效

```json
{
  "tool": "self.screen.matrix_16x16.draw_animation",
  "arguments": {
    "image": {
      "ops": [
        {"op": "clear", "fill": 0},
        {"op": "fill_circle", "cx": 8, "cy": 8, "radius": 5}
      ]
    },
    "effect": {
      "name": "breathe",
      "frame_count": 16,
      "min_density": 0.2,
      "max_density": 1.0
    },
    "frame_interval_ms": 100,
    "primary_rgb888": "#00FF88",
    "background_rgb888": "#000000",
    "source": "mcp_animation",
    "transcript": "image breathe effect"
  }
}
```

### 4.6 模板：图像+跑马灯特效

```json
{
  "tool": "self.screen.matrix_16x16.draw_animation",
  "arguments": {
    "image": {
      "bitmap_rows_hex": "018003c007e00ff01ff83ffc7ffe3ffc1ff80ff007e003c00180"
    },
    "effect": {
      "name": "marquee_right",
      "frame_count": 16,
      "step": 1
    },
    "frame_interval_ms": 70,
    "primary_rgb888": "#00FF00",
    "background_rgb888": "#000000",
    "source": "mcp_animation",
    "transcript": "image marquee effect"
  }
}
```

### 4.7 模板：滚动字幕序列

```json
{
  "tool": "self.screen.matrix_16x16.show_scroll_subtitle",
  "arguments": {
    "text": "吉林大学欢迎你",
    "effect": {
      "name": "text_scroll",
      "step": 1,
      "glyph_spacing": 1,
      "leading_padding": 16,
      "trailing_padding": 16
    },
    "frame_interval_ms": 96,
    "primary_rgb888": "#FFD60A",
    "background_rgb888": "#000000",
    "source": "mcp_scroll_subtitle",
    "transcript": "显示滚动字幕 吉林大学欢迎你"
  }
}
```

## 5. 灵活动画模式

### 5.1 随机绘制模式 (`draw_random_pattern_request`)

The debug websocket supports a random draw mode that cycles through diverse patterns and effects:

| Category | Weight | Examples |
|---|---|---|
| Static patterns | 30% | heart, diamond, cross, ring, arrow, checker, border, smile |
| Effect animations | 30% | blink/flash/wipe/marquee/breathe/fade from random patterns |
| **吉林大学 scroll** | **20%** | Horizontal scrolling "吉林大学" text (mandatory) |
| ops_sequence | 20% | circle loading, moving square, cross fade, snake |

Each random draw picks a random color from: red, green, blue, yellow, orange, purple, pink, cyan, white.

### 5.2 吉林大学滚动文字

Every random draw session includes a 20% chance of showing "吉林大学" scrolling horizontally across the 16x16 matrix. The text uses 4 hardcoded 16x16 glyph bitmaps matching the LED-side font data.

To explicitly trigger a JLU scroll:
```json
{
  "tool": "self.screen.matrix_16x16.show_scroll_subtitle",
  "arguments": {
    "text": "吉林大学",
    "effect": {
      "name": "text_scroll",
      "step": 1,
      "leading_padding": 16,
      "trailing_padding": 16
    },
    "primary_rgb888": "#FFD60A",
    "background_rgb888": "#000000",
    "frame_interval_ms": 96,
    "transcript": "吉林大学"
  }
}
```

### 5.3 Ops-Sequence 动画

Four built-in animation templates for programmatic frame generation:

Use these safe transformations on standard templates:

- Speed variants:
  - Slow: `frame_interval_ms = 300..700`
  - Medium: `120..260`
  - Fast: `40..100`
- Color variants:
  - Keep one structure, vary `primary_rgb888` only.
- Direction variants:
  - Reverse frame order for inverse motion.
- Density variants:
  - In marquee, use single column, dual columns, or 2-pixel bar.
- Shape variants:
  - Replace `fill_circle` with `fill_rectangle` or `polygon` for different pulse style.

Safe generation prompt for LLM:

```text
Generate only the arguments object for self.screen.matrix_16x16.draw_animation.
Constraints:
1) Use frames[] mode.
2) Each frame must contain exactly one source and use python_source only.
3) No import, no time.sleep, no yield_frame.
4) Frame count 6..32.
5) frame_interval_ms 40..500.
6) Every python_source starts with clear(0).
7) Provide explicit primary_rgb888 and background_rgb888.
Output JSON only.
```

## 6. 常见错误与修复

- Error pattern: `bitmap_rows_hex_list[] must contain ...`
  - Fix: use `frames[]` mode or provide full `64`-hex frame per list item.
- Error pattern: `Unsupported helper function: yield_frame`
  - Fix: remove `yield_frame`, encode timing with `frame_interval_ms`.
- Error pattern: syntax error from escaped newlines
  - Fix: keep each frame logic short and single-purpose.
- Error pattern: wrong hex length (`bitmap_rows_hex`)
  - Fix: `bitmap_rows_hex = 64` hex chars (16 rows × 4 hex digits each).
- Error pattern: `preset` or `frame_rgb332_hex` not accepted
  - Fix: use `bitmap_rows_hex` + `primary_rgb888` only. Preset and RGB332 modes are removed.

## 7. 输出负载参考

### 7.1 单帧输出（draw_python / draw_frame / render_prompt）

- `data_format = matrix_frame_v2`
- Includes `frame_rgb332_hex` (preview), `bitmap_rows_hex`, `compact_layered_hex` (BITMAP_LAYERED binary), colors, dimensions, trace fields.
- `compact_layered_hex` = 2 layers (background + foreground) × 36 bytes = 72 bytes hex-encoded.

### 7.2 序列输出（show_text / draw_animation）

- `data_format = matrix_frame_sequence_v2`
- Includes `frame_interval_ms`, `frame_count`, `frames[]`.
- Each frame includes its own `compact_layered_hex` for layered transport.
- Animation output includes compact format metadata and per-frame indices.

## 8. 调用前检查清单

1. Correct tool selected by intent.
2. For animation, prefer `frames[]` mode.
3. No `yield_frame`, `time.sleep`, or `import` in drawing code.
4. Colors explicitly set.
5. Timing only via `frame_interval_ms`.
6. Bitmap fields are not confused with RGB332 fields.
7. Payload is JSON-only, no explanation text mixed in.
8. Keep each frame logic simple and deterministic.

## 9. 传输管道与验证

### 9.1 动画传输管道

```
Python (MCP bridge)
  → Debug WS: matrix_animation_start + N×matrix_pattern_result + matrix_animation_end
    → ESP32 (lichuang_dev_board): accumulate PendingMatrixAnimation[] (max 32)
      → ShowBitmapAnimation() → ShowLayeredAnimationLocked()
        → BLE: AnimationStart(0x04) + N×AnimationFrame + AnimationEnd
          → AI8051U: BeginAnimationUpload → StoreAnimationFrame × N → CommitAnimation
            → Tick1ms → RenderAnimationFrame → RenderBitmapLayeredFrame
```

### 9.2 关键限制

- Max animation frames: **32** (protocol `GP_MATRIX_ANIMATION_MAX_FRAMES`)
- Max layers per frame: **16** (protocol `GP_MATRIX_BITMAP_LAYERED_MAX_COLORS`)
- Max layers per animation frame: **4** (protocol `GP_MATRIX_ANIMATION_MAX_LAYERS`)
- Per-layer size: **36 bytes** (1 header + 32 bitmap + 3 RGB)
- 2-color frame payload: **72 bytes** (2 layers)
- Frame chunk size: **64 bytes** (2 chunks per frame)
- Input frames > 32: **auto-resampled** to 32 with scaled interval

### 9.3 格式验证

- All frames use **BITMAP_LAYERED** (format 0x04)
- Single frames: FrameStart(0x04) → FrameChunk(s) → FrameCommit
- Animation frames: AnimationStart(0x04) → AnimationFrame[] → AnimationEnd
- LED-side rendering: clear→black, then layer 0 (background) → layer 1 (foreground) paint-on-top

### 9.4 投递模式

- Single-frame tools: debug websocket → `matrix_pattern_result`
- Animation tools: debug websocket → `matrix_animation_start` + frames + `matrix_animation_end`
- Preview: ESP32 LCD shows bitmap preview during accumulation
- Fallback: HTTP preview for single frames when debug WS unavailable
