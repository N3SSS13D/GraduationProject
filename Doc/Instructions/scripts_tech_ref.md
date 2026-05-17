# Drawing Scripts & MCP Technical Reference

Concise technical reference for `本地绘图脚本`. Derived from thesis architecture doc. See `Doc/Instructions/README.md` for navigation.

## File Map

```
Project/Script/
  mcp/gp_matrix/
    gp_display_mcp_bridge.py              -- Entry point: thin wrapper launching endpoint client
    gp_mcp_endpoint_client.py             -- Core: MCP client + HTTP server + Debug WS + drawing tools
    gp_matrix_drawing_mcp_usage.md        -- Tool usage contract: inputs, constraints, avoidance
    gp_bridge_mcp_service.py              -- MCP tool mode: drawing ops, bitmap ASCII validation
    gp_bridge_transport_service.py        -- Parameter parser: WS URL, HTTP host/port config
    gp_matrix_llm_inputs.py               -- LLM input normalization: bitmap ASCII, drawing ops
  media_tools/
    led_image_converter_gui.py            -- Tkinter GUI: image/text → 16x16 asset converter
  tools/
    ws2812_auto_debug.py                  -- Automated: Keil build → STC serial → ESP-IDF flash/monitor
    generate_glyph_demo.py                -- TrueType font → 16x16 glyph image generator
    generate_thesis_flowcharts.py         -- Thesis Mermaid flowchart generator
```

## Core Service Architecture

```
gp_display_mcp_bridge.py (entry)
  └→ gp_mcp_endpoint_client.py
       ├── McpWebSocketClient    -- Remote MCP connection (wss://api.xiaozhi.me/mcp/)
       ├── McpBridgeServer       -- Local tool registry + dispatch
       ├── HttpServer (port 8765)-- HTTP fallback endpoints
       └── DebugWebSocket (port 8766) -- AI-side frame delivery
```

## MCP Tools Reference

### Host LLM Tools (`self.screen.matrix_16x16.*`)

| Tool | Input | Output | Description |
|---|---|---|---|
| `draw_python` | Python source string | `bitmap_rows_hex` + `rgb_color` | Execute restricted Pillow drawing via AST whitelist |
| `render_prompt` | Natural language text | `bitmap_rows_hex` | LLM interprets text → pattern template |
| `show_text` | Text, font_size, color, speed | Animation frame sequence | Text → glyph → 16×16 bitmap per char |
| `show_scroll_subtitle` | Text, scroll_dir, speed | Scroll frame sequence | Long text → offscreen bitmap → chunked 16×16 windows |
| `show_effect` | Effect name + params | `matrix_action_result` JSON | Direct native effect (no frame-by-frame) |
| `draw_animation` | Frame list + timing | Animation JSON batch | Multi-frame animation with resampling |

### Local Debug Tools (`self.screen.matrix_16x16.local.*`)

| Tool | Description |
|---|---|
| `snapshot` | Capture current matrix display state |
| `preview` | Preview frame on ESP32 LCD before Bluetooth upload |
| (others) | Hidden in low-budget mode (`tools/list` minimum tier) |

## Drawing Pipeline

```
User Input (voice/text)
  → MCP tools/call
    → Parameter validation + normalization (gp_matrix_llm_inputs.py)
      → Drawing execution:
          draw_python: AST-whitelist parse → exec() in restricted namespace
          show_text:   char → PIL ImageFont → numpy array → 16×16 bitmap
          show_scroll:  text → wide PIL image → 16×16 sliding windows
      → Format encoding:
          bitmap_rows_hex: ["0x0000", ...]  (16 hex strings, MSB-first per row)
          rgb_color: [R, G, B]  (0-255 each)
      → Transport:
          Debug WebSocket: {"type":"matrix_pattern_result", ...}
          HTTP fallback: POST /control/matrix_prompt_16x16
```

## AST Whitelist (draw_python Security)

`draw_python` executes user/Python code under AST whitelist. Allowed operations:
- `Image.new()`, `ImageDraw.Draw()`
- Drawing primitives: `point()`, `line()`, `rectangle()`, `ellipse()`, `polygon()`, `arc()`, `chord()`, `pieslice()`
- Color: only `(R, G, B)` tuples
- Math: `+`, `-`, `*`, `/`, `//`, `%`, `**`, `abs()`, `min()`, `max()`, `round()`, `range()`, `len()`
- Control: `for`, `if/else`, list comprehensions, variable assignment

Blocked: `import`, `exec`, `eval`, `open`, `__`, file I/O, network, `os`, `sys`, `subprocess`.

## Transport Details

| Transport | Address | Format | Use |
|---|---|---|---|
| Debug WebSocket | `ws://<esp32-ip>:8766/debug` | JSON: `matrix_pattern_result` | Primary result delivery |
| HTTP POST | `http://<esp32-ip>:8765/control/matrix_prompt_16x16` | JSON body | Fallback when WS unavailable |
| HTTP POST | `http://<esp32-ip>:8765/control/screenshot` | PNG binary | Debug screenshot upload |
| MCP WebSocket | `wss://api.xiaozhi.me/mcp/` | JSON-RPC | Remote MCP tool registration |

## Auto-Debug Chain

`ws2812_auto_debug.py` automated sequence:

1. **Keil rebuild**: Run Keil UV4 to rebuild `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
2. **Wait 20s**: Allow rebuild + STC ISP flash programming to complete
3. **STC serial monitor**: Open AI8051U serial port (default `COM15`, 9600 bps) for logs
4. **ESP-IDF**: `idf.py build flash monitor` for `Project/xiaozhi-esp32`

Tool paths (configurable via env/args):
- Keil: `S:\Embedded\Keil`
- ESP-IDF: `S:\Embedded\ESP\v5.4.3\esp-idf`

## Image Format Constants (Script-Side)

Must match `Project/Protocols/gp_led_matrix_protocol.h`:
| Constant | Value |
|---|---|
| RBG332 frame size | 256 bytes |
| Compact bitmap size | 38 bytes (32B bmp + 3B rgb + 3B hdr) |
| Layered layer size | 36 bytes (32B bmp + 3B rgb + 1B hdr) |
| Max animation frames | 32 |
| Max layers (static) | 16 |
| Max animation layers | 4 |

## Common Pitfalls

1. **Never use `yield_frame` or `time.sleep`** in drawing code — breaks WebSocket event loop
2. **Bitmap row hex format**: MSB-first per row, uint16 little-endian. `"0x07E0"` = bits `0000011111100000` (row 1 of heart)
3. **Color in `draw_python`**: Only `(R, G, B)` tuples 0-255. No hex strings, no HSL, no named colors.
4. **Animation resampling**: If input has >32 frames, resample to 32 (uniform sampling). If <1, error.
5. **Namespace boundary**: Host tools use `self.screen.matrix_16x16.*`; local debug uses `.local.*`. Never mix.
6. **Debug WS receives raw JSON**: Must include `type` field (`matrix_pattern_result`, `matrix_animation_start`, etc.)
