# GP Matrix Drawing MCP Usage

## Canonical Script

Run the local MCP bridge with:

```bash
python GP_Port/gp_display_mcp_bridge.py --verbose
```

The legacy entry `GP_Port/gp_mcp_endpoint_client.py` still works for compatibility, but the canonical LLM-facing script name is `gp_display_mcp_bridge.py`.

## Tool Selection

Use these tools in this order:

1. `self.screen.matrix_16x16.draw_python`
2. `self.screen.matrix_16x16.show_text`
3. `self.screen.matrix_16x16.draw_frame`
4. `self.screen.matrix_16x16.render_prompt`

`draw_python` is the primary tool for arbitrary patterns.

## Input Rules

### `self.screen.matrix_16x16.draw_python`

Input schema:

```json
{
  "python_source": "for i in range(16):\n    draw.point((i, i), fill=1)\n    draw.point((15 - i, i), fill=1)",
  "primary_rgb888": "#00FF66",
  "background_rgb888": "#000000",
  "source": "mcp_python",
  "transcript": "draw a green X"
}
```

Allowed statements:

- `draw.point`, `draw.line`, `draw.rectangle`, `draw.ellipse`, `draw.polygon`, `draw.arc`, `draw.pieslice`, `draw.chord`
- helper functions `clear`, `point`, `line`, `rectangle`, `fill_rectangle`, `ellipse`, `fill_ellipse`, `circle`, `fill_circle`, `polygon`
- `for ... in range(...)`
- simple numeric assignments

Forbidden statements:

- `import`
- `while`
- function definitions
- class definitions
- file or network access
- attribute access other than `draw.<method>`

Color rules:

- `python_source` draws a binary mask only.
- drawn pixels use `primary_rgb888`
- empty pixels use `background_rgb888`

### `self.screen.matrix_16x16.show_text`

Input schema:

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
  "applied": true
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

## Recommended Examples

### Diagonal Cross

```python
for i in range(16):
    draw.point((i, i), fill=1)
    draw.point((15 - i, i), fill=1)
```

### Border Plus Filled Circle

```python
draw.rectangle((0, 0, 15, 15), outline=1)
fill_circle(8, 8, 4)
```

### Checker Blocks

```python
for y in range(0, 16, 4):
    for x in range(0, 16, 4):
        if ((x + y) // 4) % 2 == 0:
            fill_rectangle(x, y, x + 3, y + 3)
```

## Delivery Notes

- Single-frame tools prefer debug websocket delivery when available.
- Single-frame tools can fall back to the existing HTTP preview flow.
- Text display is intended to use the debug websocket path.