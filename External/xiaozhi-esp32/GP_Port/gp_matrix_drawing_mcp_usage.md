# GP Matrix Drawing MCP Reference

## Canonical Entry

Run the local MCP bridge with:

```bash
python GP_Port/gp_display_mcp_bridge.py --verbose
```

Canonical LLM-facing entry: `GP_Port/gp_display_mcp_bridge.py`

Compatibility entry: `GP_Port/gp_mcp_endpoint_client.py`

## Tool Selection

Use these tools in this order:

1. `self.screen.matrix_16x16.draw_python`
Primary tool for most custom drawings.

2. `self.screen.matrix_16x16.show_text`
Use when the request is naturally text and should become a short frame sequence.

3. `self.screen.matrix_16x16.draw_frame`
Use when you already have `frame_rgb332_hex` or `bitmap_rows_hex`.

4. `self.screen.matrix_16x16.render_prompt`
Legacy fallback for vague natural-language prompts when you do not want to author draw code directly.

## Shared Rules

- The matrix is always `16x16`.
- Single-frame tools return `matrix_frame_v1`.
- `primary_rgb888` colors painted pixels.
- `background_rgb888` colors empty pixels.
- Include `source` and `transcript` whenever possible for traceability.
- Downstream transfer to `LED端` should prefer compact `bitmap_rows_hex + RGB888` over expanding to a full `256`-byte RGB332 frame.

## `self.screen.matrix_16x16.draw_python`

### When To Use It

Use `draw_python` for arbitrary patterns, especially when an LLM needs programmatic control.

### Input Schema

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

At least one of `python_source` or `eval_source` is required.

### Execution Model

- `python_source`: restricted Python statements executed with `exec(...)`.
- `eval_source`: restricted Python expression evaluated with `eval(...)`.
- If both are present, `python_source` runs first, then `eval_source` runs in the same scope.
- Inside `python_source`, `eval("<expression>")` is also available and uses the same restrictions as `eval_source`.

### Choose Between `python_source` And `eval_source`

- Prefer `python_source` for readable imperative steps, variable assignments, and short `for` loops.
- Prefer `eval_source` for list comprehensions, conditional expressions, or compact expression-heavy drawing logic.
- Prefer using both only when `python_source` needs to define reusable variables before `eval_source` runs.

### Allowed Helpers

- `range`
- `min`
- `max`
- `abs`
- `int`
- `eval`
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

### Allowed `draw.<method>` Calls

- `draw.point`
- `draw.line`
- `draw.rectangle`
- `draw.ellipse`
- `draw.polygon`
- `draw.arc`
- `draw.pieslice`
- `draw.chord`

### Allowed Statement/Expression Shapes

- simple assignments such as `step = 4`
- `for ... in range(...)`
- `if` blocks with numeric or boolean conditions
- arithmetic expressions
- list literals and tuple literals
- list comprehensions in `eval_source`
- conditional expressions in `eval_source`

### Forbidden Constructs

- `import`
- `while`
- function definitions
- class definitions
- async comprehensions
- file access
- network access
- arbitrary attribute access other than `draw.<allowed_method>`

### Color Rules

- Drawing code always creates a binary mask.
- Painted pixels use `primary_rgb888`.
- Unpainted pixels use `background_rgb888`.

### Copyable Examples

Imperative diagonal cross:

```json
{
  "python_source": "for i in range(16):\n    point(i, i)\n    point(15 - i, i)",
  "primary_rgb888": "#00FF66",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw a green X"
}
```

`eval_source` with a comprehension:

```json
{
  "eval_source": "[(point(i, i), point(15 - i, i)) for i in range(16)]",
  "primary_rgb888": "#FFFFFF",
  "background_rgb888": "#000000",
  "source": "mcp_eval",
  "transcript": "draw a white X through eval_source"
}
```

Mixed mode with a shared variable:

```json
{
  "python_source": "step = 4",
  "eval_source": "[fill_rectangle(x, 0, x + 1, 1) for x in range(0, 16, step)]",
  "primary_rgb888": "#FFD60A",
  "background_rgb888": "#101010",
  "source": "mcp_python",
  "transcript": "draw yellow bars with a shared step"
}
```

Nested `eval(...)` inside `python_source`:

```json
{
  "python_source": "step = 4\neval(\"[fill_rectangle(x, 0, x + 1, 1) for x in range(0, 16, step)]\")",
  "primary_rgb888": "#14B8A6",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw cyan bars with nested eval"
}
```

Checker blocks with statement control flow:

```json
{
  "python_source": "for y in range(0, 16, 4):\n    for x in range(0, 16, 4):\n        if ((x + y) // 4) % 2 == 0:\n            fill_rectangle(x, y, x + 3, y + 3)",
  "primary_rgb888": "#FF9F0A",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw orange checker blocks"
}
```

## `self.screen.matrix_16x16.show_text`

### Input Schema

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

Rules:

- each character becomes one `16x16` frame
- frames are sent one by one to `AI端` through the debug websocket when connected
- `frame_interval_ms` controls the interval between frames

## `self.screen.matrix_16x16.draw_frame`

Use this when you already have frame data.

Preferred inputs:

- `bitmap_rows_hex` plus `primary_rgb888` and optional `background_rgb888`
- or `frame_rgb332_hex` when you already own the full RGB332 frame

Example:

```json
{
  "bitmap_rows_hex": "8001400220041008081004200240018001800240042008101008200440028001",
  "primary_rgb888": "#F5F5F5",
  "background_rgb888": "#000000",
  "source": "mcp_frame",
  "transcript": "draw a white X from a bitmap mask"
}
```

## `self.screen.matrix_16x16.render_prompt`

Use only as a fallback when a free-text description is easier than explicit draw code.

Example:

```json
{
  "prompt": "draw a cyan heart",
  "primary_rgb888": "#14B8A6",
  "background_rgb888": "#000000",
  "source": "mcp_prompt",
  "transcript": "draw a cyan heart"
}
```

## Output Format

### Single Frame

`draw_python`, `draw_frame`, and `render_prompt` return:

```json
{
  "data_format": "matrix_frame_v1",
  "content_type": "python_draw",
  "frame_rgb332_hex": "<512 hex chars>",
  "bitmap_rows_hex": "<64 hex chars>",
  "width": 16,
  "height": 16,
  "primary_rgb888": "#00FF66",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw a green X",
  "python_source": "for i in range(16):\n    point(i, i)\n    point(15 - i, i)",
  "eval_source": "[(point(i, i), point(15 - i, i)) for i in range(16)]",
  "applied": true,
  "tool_name": "self.screen.matrix_16x16.draw_python"
}
```

### Text Sequence

`show_text` returns:

```json
{
  "data_format": "matrix_frame_sequence_v1",
  "content_type": "text",
  "text": "Hi",
  "frame_interval_ms": 180,
  "frame_count": 2,
  "frames": [
    {
      "data_format": "matrix_frame_v1",
      "content_type": "text",
      "glyph": "H",
      "frame_index": 0,
      "frame_count": 2,
      "frame_rgb332_hex": "<512 hex chars>",
      "bitmap_rows_hex": "<64 hex chars>",
      "primary_rgb888": "#FFFFFF",
      "background_rgb888": "#000000"
    }
  ],
  "source": "mcp_text",
  "transcript": "show text Hi",
  "applied": true
}
```

## LLM Calling Checklist

1. If you need a custom pattern, start with `self.screen.matrix_16x16.draw_python`.
2. Use `python_source` for readable imperative steps.
3. Use `eval_source` for compact expression-based drawing logic.
4. If you already have a bitmap mask, skip code generation and call `draw_frame`.
5. If the request is text, call `show_text`.
6. Always provide explicit colors for reproducible output.
7. Keep generated output bounded to one `16x16` frame unless text playback is explicitly needed.

## Delivery Notes

- Single-frame tools prefer debug websocket delivery when available.
- Single-frame tools can fall back to the existing HTTP preview flow.
- Text display is intended to use the debug websocket path.
- After the `AI端` receives `matrix_pattern_result`, it should forward `bitmap_rows + RGB888` to the `LED端` over Bluetooth using the compact frame format instead of expanding to a full RGB332 frame.
- Keep Bluetooth `FrameChunk` sizing aligned with the shared protocol constant: `64` bytes on both the `AI端` and `LED端`.