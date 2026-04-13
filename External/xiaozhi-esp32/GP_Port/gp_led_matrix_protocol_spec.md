# GP LED Matrix Protocol Specification

## 中文

### 设计目标

协议以 ESP32 为 I2C 主机、AI8051U 为 I2C 从机，当前优先服务本地通信闭环，兼顾四类数据：状态提示、动作控制、16x16 RGB332 帧、16 行字模数据。协议既要能承载完整图像，也要能支持后续 ACK、状态回传和联调诊断。

- 当前联调默认从机地址固定为 `0x31`。
- 当前联调硬件连线固定为：小智 `IO1=SDA`、`IO2=SCL`；AI8051U `P2.4=SCL`、`P2.3=SDA`。
- 当前样机若采用 `ESP32 3.3V 上拉 + AI8051U 5V 从机` 的直连方式，软件侧建议矩阵链路保持在 `100kHz`，硬件侧长期方案建议补双向电平转换。

### 包结构

每个 I2C 发送单元为一包，格式如下：

| 字段 | 长度 | 说明 |
| --- | --- | --- |
| `magic0` | 1 | 固定为 `0x47` |
| `magic1` | 1 | 固定为 `0x50` |
| `version` | 1 | 当前为 `0x01` |
| `command` | 1 | 命令字 |
| `sequence` | 1 | 包序号，循环递增 |
| `flags` | 1 | 当前保留 ACK 请求位 |
| `payload_length` | 2 | 负载长度，按 little-endian 解释 |
| `payload` | N | 具体命令负载 |
| `checksum` | 1 | 从包头到负载末尾的 XOR |

说明：

- 多字节字段统一按 little-endian 手工序列化，不依赖编译器结构体内存布局。
- 当前优先支持本地 I2C 动作控制，后续再向 MCP 上行/下行映射扩展。

### 命令集合

| 命令 | 值 | 用途 |
| --- | --- | --- |
| `Ping` | `0x01` | 通链探测 |
| `SetBrightness` | `0x02` | 设置整体亮度 |
| `SetMode` | `0x03` | 设置播放模式 |
| `StateHint` | `0x04` | 同步小智当前状态 |
| `SetAction` | `0x05` | 下发本地动作描述（图案、效果、颜色、方向等） |
| `FrameStart` | `0x10` | 开始一次 RGB332 帧传输 |
| `FrameChunk` | `0x11` | 分片发送 RGB332 帧 |
| `FrameCommit` | `0x12` | 提交并显示 RGB332 帧 |
| `ScrollGlyphStart` | `0x20` | 开始滚动字模传输 |
| `ScrollGlyphChunk` | `0x21` | 分片发送字模数据 |
| `ScrollGlyphCommit` | `0x22` | 提交并进入滚动显示 |
| `Heartbeat` | `0x30` | 保活 |
| `Status` | `0x31` | AI8051U 回传状态 |
| `Error` | `0x7f` | AI8051U 回传错误 |

### 图像负载

#### 动作负载（MCP 语义裁剪版）

`SetAction` 负载是一个固定长度动作对象，用于在不依赖外部 MCP 桥接的情况下，先打通本地 I2C 控制链。其字段覆盖：

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

这套动作对象是对 MCP 工具结果的二进制裁剪映射，优先适配本地通信，后续再扩展外部 MCP 接入。

#### RGB332 帧

- 分辨率固定为 `16 x 16`。
- 每像素 1 字节，格式与 `test_image.h` 中现有帧一致。
- 一帧总长度 256 字节，因此必须通过 `FrameStart + FrameChunk + FrameCommit` 发送。
- 当前每片最大负载为 192 字节。

#### 字模滚动

- 单个汉字按 16 行 `uint16_t` 表示，每行 16 bit。
- `ScrollGlyphStart` 负载包含字数、字宽、字距与总字节数。
- `ScrollGlyphChunk` 使用与帧分片相同的前缀结构。

### 推荐时序

1. ESP32 上电后保持链路空闲，直到出现显式图像更新请求。
2. 如果只需切换图案、效果、颜色或亮度，优先使用 `SetAction`，避免每次都下发整帧。
3. 每次设备状态变化默认不自动覆盖上一条矩阵图像，只有显式图像更新才触发发送。
4. 如果使用滚动字模，先完整下发字模，再以 `SetMode` 或 `ScrollGlyphCommit` 切换显示模式。
5. 对需要确认执行结果的关键命令启用 `ACK_REQUIRED`，并在主机侧同步读取 `Status/Error` 回包。

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

The protocol assumes ESP32 as the I2C master and AI8051U as the I2C slave. It supports state hints, full RGB332 frames, glyph-row transfers, and future status/error reporting.

### Packet layout

`magic0`, `magic1`, `version`, `command`, `sequence`, `flags`, `payload_length`, `payload`, `checksum`

### Transport notes

1. Full RGB332 frames are chunked because one frame is 256 bytes.
2. Glyph scrolling uses the same start/chunk/commit pattern.
3. Checksum is a simple XOR now for easy 8051-side parsing.

### Next extension points

1. Add frame retry after `BadChecksum` or `BadSequence`.
2. Add optional heartbeat watchdog on the AI8051U side so the gradient fallback can be restored automatically after link loss.
3. Extend the local `SetAction` mapping to MCP tools after the local I2C loop is stable.
4. Add free-text subtitle delivery on top of the existing glyph-transfer path.
