# GP Matrix Drawing MCP Reference

## Category

`本地绘图脚本`

## 0. Canonical Entry

Run the local MCP bridge with:

```bash
python Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py
```

Canonical LLM-facing entry: `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`

Compatibility entry: `Project/Script/mcp/gp_matrix/gp_mcp_endpoint_client.py`

Related protocol docs:

- `Project/Protocols/gp_led_matrix_protocol_spec.md`
- `Project/Protocols/gp_matrix_pattern_protocol.md`

### 0.1 Runtime Layout (Modularized)

The bridge is now organized into independent modules while preserving the same LLM-facing tool names.

1. `gp_display_mcp_bridge.py`
Role: total entry script, direct run without extra arguments.

2. `gp_mcp_endpoint_client.py`
Role: MCP orchestration and tool dispatch.

3. `gp_matrix_llm_inputs.py`
Role: drawing-input normalization for LLM-friendly formats (`bitmap_ascii`, `ops`).

4. `gp_bridge_mcp_service.py`
Role: MCP schema helper blocks shared by tools.

5. `gp_bridge_transport_service.py`
Role: transport/runtime argument parser builder.

## 1. Slice Index (For Auto-Segmented LLM Reading)

Use this index when the document is automatically chunked.

- `Section 2`: Global invariants and hard constraints (always apply).
- `Section 3`: Tool selection decision tree.
- `Section 4`: Per-tool contracts (`draw_python`, `show_text`, `draw_animation`, `draw_frame`, `render_prompt`).
- `Section 5`: Standard anti-failure animation templates (slow blink, breathing, marquee).
- `Section 6`: Flexible animation construction patterns for diverse effects.
- `Section 7`: Common failure patterns and direct fixes.
- `Section 8`: Output payload quick reference.
- `Section 9`: LLM final checklist before tool call.

## 1.1 Tool-Gated Knowledge Template (Must Query Tool First)

Use this template when a topic must never be answered directly and always requires a tool query first.

### 1.1.1 Generic Template

```text
本文档包含 "xxxxx" 等相关内容。当用户提及这些内容时，我不能直接作答，必须在每次对话中调用该工具查询资料后，再依据查询结果进行回答。

## 以下是 “xxxxx” 的内容
(这里填写文档的总结内容)
```

### 1.1.2 Recommended Filled Template (Copy And Edit)

```text
本文档包含 "16x16 动画输入规范"、"bitmap_rows_hex 格式约束"、"draw_animation 反失败模板" 等相关内容。当用户提及这些内容时，我不能直接作答，必须在每次对话中先调用工具查询资料（例如查询最新工具 schema、规则段落或示例模板），再依据查询结果进行回答。

## 以下是 “16x16 动画输入规范” 的内容
1. 动画优先使用 frames 模式，每帧只允许一种输入源：bitmap_rows_hex 或 bitmap_rows 或 python_source/eval_source。
2. 禁止在绘图语句中实现时序：禁止 yield_frame、time.sleep、import。
3. 动画时序只通过 frame_interval_ms 控制。
4. 位图字段与 RGB332 字段不可混用：bitmap_rows_hex 是 64 hex，frame_rgb332_hex 是 512 hex。
5. 若输入帧数超过 24，桥接会重采样到 24 并缩放间隔以尽量保持总时长。
```

### 1.1.3 LLM Execution Notes

- If user asks about any protected topic in `xxxxx`, call the required lookup tool first.
- Do not answer from stale memory if tool lookup is required by policy.
- In the final answer, use the tool result as the source of truth and keep wording aligned with queried data.

## 2. Global Invariants (Always True)

- Matrix resolution is always `16x16`.
- Compact LED-ready frame is always `38 bytes`:
  - `32-byte bitmap` + `3-byte primary RGB888` + `3-byte background RGB888`.
- `bitmap_rows_hex` is only the bitmap portion.
  - Canonical form: exactly `64` hex characters (`16 rows x 16 bits = 32 bytes`).
- `bitmap_ascii` is an LLM-friendly alias for one frame:
  - exactly `16` rows x `16` chars.
  - on-pixels: `1/#/X/*/@`; off-pixels: `0/./_/-/space`.
- `ops` is an LLM-friendly structured draw list:
  - each item includes `op` and operation-specific numeric fields.
- Bitmap semantics are fixed:
  - `1 = LED on`, `0 = LED off`.
  - Rows are `top -> bottom`.
  - In each row, high bit (`bit15`) is the leftmost LED.
- Do not pass `0/1` arrays, ASCII art, spaced binary strings, or `256-byte frame_rgb332_hex` into `bitmap_rows_hex` fields.
- Include `source` and `transcript` whenever possible for traceability.
- For downstream transfer to `LED端`, prefer compact `bitmap_rows_hex + RGB888` over expanded `256-byte RGB332`.
- Buffered animation playback uses one shared `frame_interval_ms` per batch.
- If animation input has more than `24` frames, host bridge resamples to `24` and scales interval to preserve total duration.

## 3. Tool Selection (Decision Tree)

1. Need one custom pattern from code-like instructions:
   - Use `self.screen.matrix_16x16.draw_python`.
2. Need scrolling or per-character text sequence:
   - Use `self.screen.matrix_16x16.show_text`.
3. Need buffered LED-side animation sequence:
   - Use `self.screen.matrix_16x16.draw_animation`.
4. Already have one final bitmap or full RGB332 frame:
   - Use `self.screen.matrix_16x16.draw_frame`.
5. Only have vague natural language prompt and no explicit draw plan:
   - Use `self.screen.matrix_16x16.render_prompt`.

## 4. Tool Contracts

### 4.1 `self.screen.matrix_16x16.draw_python`

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

#### Allowed helper names

- `range`, `min`, `max`, `abs`, `int`, `eval`
- `clear`, `point`, `line`, `rectangle`, `fill_rectangle`
- `ellipse`, `fill_ellipse`, `circle`, `fill_circle`, `polygon`

#### Allowed `draw.<method>`

- `draw.point`, `draw.line`, `draw.rectangle`, `draw.ellipse`
- `draw.polygon`, `draw.arc`, `draw.pieslice`, `draw.chord`

#### Forbidden constructs

- `import`, `while`, function/class definitions, async comprehensions.
- File/network access.
- Attribute access other than allowed `draw.<method>`.

### 4.2 `self.screen.matrix_16x16.show_text`

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

### 4.3 `self.screen.matrix_16x16.draw_animation`

Use for multi-frame buffered animation.

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

Supported `effect.name`:

- `blink`
- `breathe`
- `marquee`
- `marquee_left`
- `marquee_right`

Effect parameter notes:

- `frame_count`: generated animation frame count.
- `duty_cycle`: blink on-ratio (0..1).
- `step`: marquee shift step per frame.
- `direction`: optional for `marquee` (`left` or `right`).
- `min_density`/`max_density`: breathe density range (0..1).

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

### 4.4 `self.screen.matrix_16x16.draw_frame`

Use when you already have final frame data.

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

- `bitmap_rows_hex`: exactly `64` hex chars.
- `bitmap_ascii`: accepted as alias, normalized to `bitmap_rows_hex`.
- `frame_rgb332_hex`: exactly `512` hex chars (if used instead).

### 4.5 `self.screen.matrix_16x16.render_prompt`

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

## 5. Standard Anti-Failure Animation Templates

All templates below are safe defaults for direct LLM use.

### 5.1 Template: Slow Blink

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

### 5.2 Template: Breathing Pulse (Area breathing)

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

### 5.3 Template: Marquee (Column scan)

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

### 5.4 Template: Image + Blink Effect (Direct LLM Call)

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

### 5.5 Template: Image + Breathe Effect (Direct LLM Call)

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

### 5.6 Template: Image + Marquee Effect (Direct LLM Call)

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

## 6. Flexible Animation Patterns (Diversity Without Failure)

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
4) Frame count 6..24.
5) frame_interval_ms 40..500.
6) Every python_source starts with clear(0).
7) Provide explicit primary_rgb888 and background_rgb888.
Output JSON only.
```

## 7. Failure Patterns -> Direct Fix

- Error pattern: `bitmap_rows_hex_list[] must contain ...`
  - Fix: use `frames[]` mode or provide full `64`-hex frame per list item.
- Error pattern: `Unsupported helper function: yield_frame`
  - Fix: remove `yield_frame`, encode timing with `frame_interval_ms`.
- Error pattern: syntax error from escaped newlines
  - Fix: keep each frame logic short and single-purpose.
- Error pattern: wrong hex length (`frame_rgb332_hex` / `bitmap_rows_hex`)
  - Fix: `frame_rgb332_hex = 512` hex, `bitmap_rows_hex = 64` hex.

## 8. Output Payload Quick Reference

### 8.1 Single frame output (`draw_python`, `draw_frame`, `render_prompt`)

- `data_format = matrix_frame_v1`
- Includes `frame_rgb332_hex`, `bitmap_rows_hex`, colors, dimensions, trace fields.

### 8.2 Sequence output (`show_text`, `draw_animation`)

- `data_format = matrix_frame_sequence_v1`
- Includes `frame_interval_ms`, `frame_count`, `frames[]`.
- Animation output includes compact format metadata and per-frame indices.

## 9. Final LLM Checklist Before Calling MCP

1. Correct tool selected by intent.
2. For animation, prefer `frames[]` mode.
3. No `yield_frame`, `time.sleep`, or `import` in drawing code.
4. Colors explicitly set.
5. Timing only via `frame_interval_ms`.
6. Bitmap fields are not confused with RGB332 fields.
7. Payload is JSON-only, no explanation text mixed in.
8. Keep each frame logic simple and deterministic.

## 10. Delivery Notes

- Single-frame tools prefer debug websocket delivery when available.
- Single-frame tools can fall back to HTTP preview flow.
- Text and animation playback are intended for debug websocket path.
- Animation sequence transport should use:
  - `matrix_animation_start`
  - indexed `matrix_pattern_result` frames
  - `matrix_animation_end`
- `AI端` should buffer and loop preview locally, then forward full batch to `LED端`.
- Compact bitmap frames are `38` bytes, so low-latency flow can rely on final `FrameCommit` ACK when payload fits one chunk.
