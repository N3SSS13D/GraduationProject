# Bluetooth Protocol Technical Reference

Concise technical reference for `蓝牙通信协议`. Derived from thesis architecture doc. See `Doc/Instructions/README.md` for navigation.

## Files

| File | Role |
|---|---|
| `Project/Protocols/gp_led_matrix_protocol.h` | Single source of truth: magic, flags, command IDs, payload structs, CRC constants |
| `Project/Protocols/gp_led_matrix_protocol_spec.md` | Packet format, V2/V3, CRC scheme, frame chunking, ACK semantics |
| `Project/Protocols/gp_matrix_pattern_protocol.md` | Host↔AI drawing contract: JSON format, bitmap types, animation params |

## Packet Format (V3 Compact, 6-Byte Header)

```
Offset  Size  Field           Description
  0      1    magic           0x47 (fixed)
  1      1    flags           [reserved:5][is_reply:1][local_only:1][ack_req:1]
  2      1    sequence        Rolling counter (uint8, wraps)
  3      1    command         Command ID (see Command Set below)
  4      1    payload_length  0..255 (max payload bytes)
  5      1    header_crc8     CRC-8/MAXIM over bytes 0-4
  6      N    payload         Command payload (N = payload_length)
 6+N     2    packet_crc16    CRC-16/XMODEM over bytes 0..(5+N)
```

**Detection**: `byte[0] == 0x47 && byte[1] != 0x50` → V3. (V2 legacy uses `byte[1] == 0x50`.)

**Total overhead**: 8 bytes (6 header + 2 trailer). V2 overhead: 14 bytes.

## V2 Legacy Format (12-Byte Header)

Used for backward compatibility. Detection: `byte[0]==0x47, byte[1]==0x50`. Fields: magic0, magic1, version, header_size, packet_type, flags, sequence, reply_to_sequence, command, payload_length, header_crc8.

## Dual CRC Rationale

1. `header_crc8` (CRC-8/MAXIM): Validates first 5 header bytes before trusting `payload_length`. Prevents noise from causing oversized buffer reads.
2. `packet_crc16` (CRC-16/XMODEM): Validates entire packet (header + payload). Catches payload corruption that passes header CRC.

## Request/Reply Matching

- Request: `flags.is_reply = 0`
- Reply: `flags.is_reply = 1`, `command = original command` (echo), `sequence = original sequence`
- No separate error/status commands; all commands share unified ACK semantics.
- AI-side matches reply by: `IS_REPLY` flag + `sequence` + `command` + both CRC passes.

## Command Set

### Parameter/Control Commands
| Command | ID | Payload | Description |
|---|---|---|---|
| Ping | 0x01 | 0B | Link probe. Reply echoes empty. |
| SetBrightness | 0x02 | 1B | Global brightness 0-255 |
| SetMode | 0x03 | 1B | Display mode |
| StateHint | 0x04 | 1B | AI state sync (idle/listening/speaking) |
| SetAction | 0x05 | 28B | Display action: content type, effect, color, timing, direction |
| SetDebugLed | 0x06 | 2B | Set single 7-seg LED digit value |
| SetDebugLedFlow | 0x07 | 1B | Start/stop debug LED flow animation |
| SetTime | 0x08 | 6B | Clock sync: HH,MM,SS,YY,MM,DD |
| RequestCachedBitmap | 0x09 | 0B | LED→AI: request resend of last cached bitmap |
| Heartbeat | 0x30 | 0B | Keep-alive (no reply expected) |

### Full-Frame Image Commands (Chunked Transfer)
| Command | ID | Payload | Description |
|---|---|---|---|
| FrameStart | 0x10 | 5B | Begin frame: format(hdr:1 + w:2 + h:2), total_size |
| FrameChunk | 0x11 | 3B+N | Data chunk: offset(2B) + chunk_len(1B) + data (max 64B/chunk) |
| FrameCommit | 0x12 | 1B | Commit frame: render mode. Triggers display. |
| ScrollGlyphStart | 0x20 | 5B | Begin scrolling glyph: same header format as FrameStart |
| ScrollGlyphChunk | 0x21 | 3B+N | Glyph chunk data |
| ScrollGlyphCommit | 0x22 | 1B | Commit glyph, start scroll render |

### Animation Commands
| Command | ID | Payload | Description |
|---|---|---|---|
| AnimationStart | 0x13 | 5-6B | Begin animation: total_frames, frame_interval_ms(2B, optional), flags |
| AnimationFrame | 0x14 | 3B+N | Single frame: format(hdr:1 + w:2 + h:2), chunk data |
| AnimationEnd | 0x15 | 1B | Commit animation, start playback |

### Lightweight Image Commands (Single-Packet, V3 only)
| Command | ID | Payload | Description |
|---|---|---|---|
| LayeredFrame | 0x18 | 36-576B | Static layered frame (1-16 layers, 36B/layer): [hdr:1][bitmap:32][RGB:3]×N |
| LayeredAnimFrame | 0x19 | 36-144B | Single animation frame (1-4 layers, 36B/layer) |

## Image Format Specifications

### 1. RGB332 Full Frame
- Size: 256 bytes (16×16 pixels × 1 byte)
- Encoding: RRRGGGBB (3-3-2 bits per channel)
- Transfer: FrameStart(5B) + FrameChunk×N + FrameCommit(1B)
- Efficiency: 256B payload / 264B total ≈ 97%

### 2. Compact Bitmap + RGB888
- Size: 38 bytes (1B header + 32B bitmap + 2B reserved + 3B RGB888)
- Bitmap: 16 rows × 2 bytes/row (uint16 LE, MSB-first per row)
- Color: single RGB888 for all set bits
- Transfer: FrameStart + FrameChunk×1 + FrameCommit
- Efficiency: 38B payload / 49B total ≈ 78%

### 3. Layered Bitmap (Per Layer = 36 bytes)
```
Offset  Size  Field
  0      1    [total_layers:4][seq:4]
  1     32    Bitmap data (16 rows × 2 bytes/row, uint16 LE)
 33      3    RGB888 color for this layer
```
- Layer 0 = base (pixels not set → black)
- Subsequent layers overlay on top (pixel=1 → replace with this layer's color)
- Max: 16 layers static, 4 layers per animation frame
- Transfer: `LayeredFrame` single packet (≤4 layers: 144B payload, 152B total. Efficiency: ~84%)
- Transfer: FrameStart/Chunk/Commit (5-16 layers: efficiency ~59%)

## Animation Constraints
| Parameter | Limit |
|---|---|
| Max frames per animation | 32 |
| Frame interval | 1-65535 ms |
| Max animation layers/frame | 4 |
| Frame storage (LED-side) | 32 × 36B = 1152 bytes |

## Host Drawing Contract

Host scripts output `matrix_pattern_result` JSON:
```json
{
  "type": "matrix_pattern_result",
  "bitmap_rows_hex": ["0x0000", "0x07E0", ...],  // 16 hex strings
  "rgb_color": [255, 0, 0],                       // RGB888 integer array
  "frame_format": "layered_bitmap",               // or "compact_bitmap", "rgb332_frame"
  "animation_frames": [...]                       // optional, for animations
}
```
See `gp_matrix_pattern_protocol.md` for full contract specification.
