# LED-Side Runtime Design

## 中文

### 角色定位

本文件说明 `LED端` 运行层的职责边界。当前 `LED端` 指 `STC8051 + HC-05 + WS2812` 这一侧，负责接收来自 `AI端` 的协议包、执行动作、准备 ACK，并维持 16x16 矩阵刷新。

当前主链路：

- `AI端` 生成动作对象或帧数据
- 蓝牙链路转发到 `LED端 UART2(P4.2/P4.3)`
- `LED端` 组包、校验、执行动作
- `LED端` 返回 `Status / Error` 回包

### 建议模块划分

1. `gp_led_matrix_ai8051u.h/.c`
   负责 `LED端` 协议上下文、轮询入口和 ACK 缓冲管理。
2. 协议解析层
   负责包头校验、长度校验、校验和检查和命令分发。
3. 渲染执行层
   负责把动作、RGB332 帧和字模数据转换为 WS2812 刷新输入。
4. UART2 / 蓝牙接入层
   负责字节接收、完整包拼装和回包发送。

### 关键数据结构

- `rxBuffer`：当前接收中的完整协议包。
- `frameBuffer`：当前显示帧，长度 256 字节。
- `brightness`：当前全局亮度。
- `mode`：当前显示模式，对应 `GpMatrixMode`。
- `expectedBytes / receivedBytes`：分片传输进度。
- `dirty`：内容已更新，等待进入渲染路径。

### 关键函数边界

| 函数 | 作用 |
| --- | --- |
| `GpLedMatrixAi8051u_Init` | 初始化 `LED端` 协议上下文、UART2 和渲染依赖 |
| `GpLedMatrixAi8051u_PrepareTx` | 准备 `Status / Error` 回包 |
| `GpLedMatrixAi8051u_Poll` | 在主循环中执行协议包消费、动作执行和超时处理 |
| `GpLedMatrixAi8051u_RenderFrame` | 刷新矩阵硬件 |
| `GpLedMatrixAi8051u_LoadGlyphRows` | 装载滚动字模数据 |

### 当前执行原则

- 中断或串口接收回调只负责搬运字节和标记完整包。
- 包校验、动作执行和渲染切换尽量留在主循环完成。
- 完整协议包到达后先准备 ACK，再执行具体 LED 动作，减少 `AI端` 读回超时。
- 定时扫描和渲染路径保持确定性，避免把高成本转换塞进扫描热路径。

### 与测试资源的关系

- `test_image.h` 可作为 `LED端` RGB332 帧测试源。
- 字模测试数组可作为 `ScrollGlyph` 路径的输入样例。
- 这些资源应服务当前协议验证，不应再假设旧的有线主链路。

## English

### Role

This document describes the `LED side` runtime boundary. The LED side is the `STC8051 + HC-05 + WS2812` side that receives packets from the AI side, prepares ACK replies, executes actions, and keeps the matrix refresh path stable.

### Recommended split

1. LED-side protocol context and polling entry
2. Packet parsing and command dispatch
3. Frame, glyph, and action execution
4. UART2 and Bluetooth byte-stream input/output

### Practical rule

Keep the byte receive path minimal. Do packet validation, action execution, and expensive frame conversion in the main polling path unless latency proves otherwise.
