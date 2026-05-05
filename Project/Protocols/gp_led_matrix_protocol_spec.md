# GP LED Matrix Protocol Specification

## Category

`蓝牙通信协议`

## 中文

### 设计目标

当前协议服务于 `AI端 -> 蓝牙 -> LED端` 主链路，承载四类数据：状态提示、动作控制、16x16 RGB332 帧、16 行字模数据。协议需要同时满足本地调试、ACK 回包和链路诊断。

V2 版本明确以以下目标为优先级：

- 唯一可识别性：包头必须能在字节流里稳定定界，不能过早相信负载长度。
- 数据完整性：包头与整包分别校验，减少误判和静默花屏。
- 结构可扩展性：头部保留 `header_size`、`packet_type` 和 `reply_to_sequence`，允许后续扩展而不重写主状态机。
- 解析高效性：`LED端` 先校验包头，再决定还要收多少字节，避免边收边猜。
- 双向交互可靠性：ACK 统一为 `Reply` 包类型，通过 `reply_to_sequence` 关联原请求。

当前默认链路：

- `AI端`：`Project/xiaozhi-esp32/main/gp_port/`
- 蓝牙传输：经典蓝牙 SPP -> HC-05
- `LED端`：`Project/STC51/ws2812_driver/`
- 本地链路诊断端点标识：`0x31`

### 包结构

每个发送单元为一包，格式如下：

| 字段 | 长度 | 说明 |
| --- | --- | --- |
| `magic0` | 1 | 固定为 `0x47` |
| `magic1` | 1 | 固定为 `0x50` |
| `version` | 1 | 当前为 `0x02` |
| `header_size` | 1 | 当前固定为 `12` |
| `packet_type` | 1 | `0x01=Request`，`0x02=Reply` |
| `flags` | 1 | 当前保留 ACK 请求位和本地链路位 |
| `sequence` | 1 | 当前包序号，循环递增 |
| `reply_to_sequence` | 1 | Reply 关联的请求序号；Request 固定为 `0` |
| `payload_length` | 2 | 负载长度，按 little-endian 解释 |
| `command` | 1 | 命令字；Reply 直接回显原命令 |
| `header_crc8` | 1 | 前 `11` 字节的 CRC8 |
| `payload` | N | 具体命令负载 |
| `packet_crc16` | 2 | 从包头到负载末尾的 CRC16 |

说明：

- 多字节字段统一按 little-endian 手工序列化，不依赖编译器结构体内存布局。
- `LED端` 和 `AI端` 都必须在读取完整包头后，先校验 `magic/version/header_size/header_crc8`，再信任 `payload_length`。
- `packet_crc16` 覆盖 `header + payload`，用于真正的整包完整性校验；`header_crc8` 只负责快速判定包头是否合法。
- 当前优先支持 `AI端` 与 `LED端` 的本地蓝牙闭环，再向更高层桥接扩展。

### 命令集合

| 命令 | 值 | 用途 |
| --- | --- | --- |
| `Ping` | `0x01` | 通链探测 |
| `SetBrightness` | `0x02` | 设置整体亮度 |
| `SetMode` | `0x03` | 设置播放模式 |
| `StateHint` | `0x04` | 同步 `AI端` 当前状态 |
| `SetAction` | `0x05` | 下发本地动作描述（图案、效果、颜色、方向等） |
| `FrameStart` | `0x10` | 开始一次 RGB332 帧传输 |
| `FrameChunk` | `0x11` | 分片发送 RGB332 帧 |
| `FrameCommit` | `0x12` | 提交并显示 RGB332 帧 |
| `ScrollGlyphStart` | `0x20` | 开始滚动字模传输 |
| `ScrollGlyphChunk` | `0x21` | 分片发送字模数据 |
| `ScrollGlyphCommit` | `0x22` | 提交并进入滚动显示 |
| `Heartbeat` | `0x30` | 保活 |

Reply 不再使用单独的 `Status/Error` 命令字，而是统一复用原请求命令并通过 `packet_type=Reply` 表示回包。

### Reply 负载

当请求设置了 `ACK_REQUIRED` 时，`LED端` 返回一个 `Reply` 包：

- `packet_type = Reply`
- `reply_to_sequence = request.sequence`
- `command = request.command`
- `payload = { status, detail, current_mode }`

其中：

- `status`：沿用共享头中的状态码
- `detail`：细化失败原因，当前主要用于 `HeaderCrc / PacketCrc / HeaderSize / PayloadLength / ChunkOffset / ChunkSize`
- `current_mode`：回包时 `LED端` 当前模式，便于主机侧诊断链路和显示状态

### 图像负载

#### 动作负载

`SetAction` 负载是一个固定长度动作对象，用于在不依赖额外桥接改造的情况下，直接打通 `AI端` 到 `LED端` 的动作链。其字段覆盖：

- `source`：本地来源或 MCP 来源
- `content`：纯色、图案、字模、状态或帧缓冲
- `effect`：静态、呼吸、渐变、左右滚动、淡入淡出、颜色循环
- `direction`：正常、180 度、顺/逆时针 90 度
- `color_mode`：纯色或渐变
- `brightness`
- 主/次 RGB888 颜色
- `pattern_id` / `glyph_id`
- `scroll_step` / `anim_step` / `gradient_span`
- `flags`：是否使用次色、是否进入远程模式、是否释放远程模式

这套动作对象是对 `AI端` 结果对象的二进制裁剪映射，优先适配当前本地蓝牙通信。

#### RGB332 帧

- 分辨率固定为 `16 x 16`。
- 每像素 1 字节，格式与 `test_image.h` 中现有帧一致。
- 一帧总长度 256 字节，因此必须通过 `FrameStart + FrameChunk + FrameCommit` 发送。
- 当前每片最大数据负载为 `64` 字节。
- `FrameChunk` 前缀改为 `byte_offset_lo + byte_offset_hi + size`，显式表示本片写入的字节偏移，不再使用“第几片 * 64”这种隐式推导。

#### 字模滚动

- 单个汉字按 16 行 `uint16_t` 表示，每行 16 bit。
- `ScrollGlyphStart` 负载包含字数、字宽、字距与总字节数。
- `ScrollGlyphChunk` 使用与帧分片相同的显式字节偏移前缀。

### 推荐时序

1. ESP32 上电后保持链路空闲，直到出现显式图像更新请求。
2. 如果只需切换图案、效果、颜色或亮度，优先使用 `SetAction`，避免每次都下发整帧。
3. 每次设备状态变化默认不自动覆盖上一条矩阵图像，只有显式图像更新才触发发送。
4. 如果使用滚动字模，先完整下发字模，再以 `SetMode` 或 `ScrollGlyphCommit` 切换显示模式。
5. 对需要确认执行结果的关键命令启用 `ACK_REQUIRED`，并在主机侧按 `packet_type=Reply + reply_to_sequence` 匹配回包。
6. RGB332 整帧传输固定按 `64` 字节数据分片，外加 `3` 字节显式偏移前缀，这样 `AI端` 与 `LED端` 的 UART 缓冲区、ACK 节奏和错误定位都更稳定。

### 当前验证重点

1. `AI端` 动作对象字段与 `GpMatrixActionPayload` 保持一致。
2. 蓝牙链路下的 `sequence`、`reply_to_sequence`、`payload_length`、`header_crc8` 和 `packet_crc16` 必须稳定。
3. `LED端` 在通过包头校验之前不能信任 `payload_length`；在完整收包后先准备 ACK，再进入动作执行。
4. 分片帧和字模传输不得阻塞 WS2812 刷新热路径，且分片偏移必须严格连续。

### 错误码建议

- `0x00`：成功
- `0x01`：忙
- `0x02`：命令不支持
- `0x03`：校验失败
- `0x04`：序号异常
- `0x05`：长度异常
- `0x7f`：内部错误

## English

### Intent

The protocol serves the current `AI side -> Bluetooth -> LED side` workflow. V2 prioritizes unambiguous framing, stronger integrity checks, extensibility, efficient parsing, and reliable request/reply matching.

### Packet layout

`magic0`, `magic1`, `version`, `header_size`, `packet_type`, `flags`, `sequence`, `reply_to_sequence`, `payload_length`, `command`, `header_crc8`, `payload`, `packet_crc16`

### Transport notes

1. Full RGB332 frames are chunked because one frame is 256 bytes.
2. Frame and glyph chunks now carry an explicit little-endian byte offset plus chunk size.
3. Receivers validate the header CRC8 before trusting `payload_length`, then validate the full packet CRC16.
4. ACKs are generic `Reply` packets matched by `reply_to_sequence`, not separate `Status/Error` commands.

### Next extension points

1. Add frame retry after `BadChecksum` or `BadSequence`.
2. Add optional heartbeat watchdog on the LED side so fallback behavior can be restored automatically after link loss.
3. Keep `SetAction` aligned with the AI-side debug and voice result objects.
4. Add free-text subtitle delivery on top of the existing glyph-transfer path.
