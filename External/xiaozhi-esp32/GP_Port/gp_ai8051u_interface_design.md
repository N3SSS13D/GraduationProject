# AI8051U Interface Layer Design

## 中文

### 角色定位

AI8051U 作为 LED 矩阵协处理器，负责接收 ESP32 下发的 I2C 包、维护帧缓存、驱动 16x16 矩阵扫描刷新，并按需回传状态或错误码。

### 建议模块划分

1. `gp_led_matrix_ai8051u.h/.c`
   负责初始化、主轮询入口、上下行缓冲区管理。
2. `gp_led_matrix_protocol_8051.c`
   负责包头解析、长度检查、校验和检查、命令分发。
3. `gp_led_matrix_renderer.c`
   负责将 RGB332 或字模数据转换为矩阵刷新缓冲。
4. `gp_led_matrix_i2c_slave.c`
   负责 AI8051U I2C 从机模式、中断接收、发送缓冲装载。

### 关键数据结构

- `rx_buffer`：接收单包数据。
- `frame_buffer`：当前显示帧，长度 256 字节。
- `brightness`：全局亮度。
- `mode`：当前显示模式，和协议中的 `GpMatrixMode` 对应。
- `expected_bytes / received_bytes`：多片传输进度。
- `dirty`：帧内容发生变化，等待刷新。

### 建议函数边界

| 函数 | 作用 |
| --- | --- |
| `GpMatrixAi8051u_Init` | 初始化 I2C 从机、GPIO、刷新定时器、上下文 |
| `GpMatrixAi8051u_OnI2cReceive` | 处理一次 I2C 接收事件 |
| `GpMatrixAi8051u_PrepareTx` | 填充状态/错误回包 |
| `GpMatrixAi8051u_Poll` | 主循环轮询，处理脏帧与超时 |
| `GpMatrixAi8051u_RenderFrame` | 刷新矩阵硬件 |
| `GpMatrixAi8051u_LoadGlyphRows` | 将字模行转换为滚动缓存 |

### I2C 从机建议

- 优先使用 `I2CSLADR` 配置固定地址。
- `STA/RX/TX/STO` 四类从机事件建议全部开启中断。
- 解析逻辑尽量在中断外完成；中断只搬运字节和置位事件标志。

### 刷新建议

- 行扫描刷新建议由定时器中断或主循环中的高频调度驱动。
- 若矩阵驱动芯片支持片上缓存，可将 `RenderFrame` 退化成批量寄存器写入。
- 若需要 RGB332 到驱动格式转换，建议在接收完成后做一次预转换，避免扫描中实时换算。

### 与 `test_image.h` 的关系

- `g_testImageFrames` 可直接作为 RGB332 测试源。
- `g_testScrollGlyphRows` 可直接作为滚动字模测试源。
- 这两类数据正好对应本协议的两套下行数据格式。

## English

### Role

The AI8051U acts as a dedicated LED-matrix controller. It receives I2C packets, owns the active frame buffer, refreshes the matrix, and optionally returns status/error information.

### Recommended implementation split

1. I2C slave receive path
2. Packet parser and dispatcher
3. Frame and glyph buffer management
4. Matrix refresh backend

### Practical rule

Keep interrupt handlers minimal. Move checksum validation, command execution, and frame conversion into the main polling path unless latency proves unacceptable.
