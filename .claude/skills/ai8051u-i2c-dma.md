# AI8051U I2C DMA

## 分类

`LED端显示驱动`

## 何时使用

- 向 AI8051U 项目添加 I2C DMA 支持
- 将仅中断的 AI8051U I2C 从机转换为 DMA 支持的负载路径
- 审阅 `DMA_I2CT_*`、`DMA_I2CR_*` 和 `DMA_I2C_*` 配置是否正确
- 调试 AI8051U 上的 `RXLOSS`、`TXOVW`、ACK 错误或 DMA 长度不匹配问题
- 复用毕业设计模式：START/STOP 成帧保留在 ISR 中，负载字节通过 RX/TX DMA 传输

## 适用范围

已有可工作 I2C 路径、希望在不重写整个协议栈的情况下添加 DMA 的 AI8051U 固件项目。

活跃分支默认使用蓝牙路径。仅在任务明确针对 AI8051U I2C DMA 后端或归档的 I2C 行为时使用此 skill。

## 推荐策略

1. 保持已有 I2C 从机状态机（如果已稳定）
2. 将 DMA 作为后端层添加到已有 `rxBuffer` 和 `txBuffer` 周围，不改变协议解析
3. 优先采用成帧+DMA 分割：
   - START/STOP、超时和封包队列保留在 I2C ISR 中
   - 负载字节通过 I2CT/I2CR DMA 传输
4. 若目标项目保守，使用 `SetDmaMode(context, enableRx, enableTx)` 分方向逐步启用

## 寄存器组

- `DMA_I2CT_CFG/CR/STA/AMT/DONE/TXAH/TXAL`：I2C TX DMA 配置和状态
- `DMA_I2CR_CFG/CR/STA/AMT/DONE/RXAH/RXAL`：I2C RX DMA 配置和状态
- `DMA_I2C_CR/ST1/ST2/ITVH/ITVL`：共享 DMA 门控、数量/完成选择、ACK 错误和传输间隔

## 流程

1. 检查已有 I2C 驱动，找到面向字节的 RX/TX 入口点
2. 验证项目已有 `ai8051u_def.h` 中的有效辅助函数：
   - `DMA_I2C_SetTxAddress`、`DMA_I2C_SetRxAddress`
   - `DMA_I2C_SetTxAmount`、`DMA_I2C_SetRxAmount`
   - `DMA_I2C_EnableTx`、`DMA_I2C_EnableRx`
   - `DMA_I2C_TriggerTx`、`DMA_I2C_TriggerRx`
   - `DMA_I2C_EnableDMA`
3. 添加 DMA 后端初始化器：
   - 禁用两个方向
   - 清除 DMA 标志
   - 清除 RX FIFO
   - 设置传输间隔和优先级
   - 在明确启用前保持 DMA 空闲
4. 添加方向特定的运行时控制，理想情况通过 `SetDmaMode(context, enableRx, enableTx)` 函数
5. TX DMA：
   - 保持封包构建逻辑不变
   - 当主机启动读取时，将 TX DMA 指向 `txBuffer`，设置数量为 `txLength - 1`，启用并触发
   - 当 `DMA_I2CT_STA.I2CTIF` 指示完成时清除 `txPending`
6. RX DMA：
   - 将 RX DMA 指向 `rxBuffer`，设置数量为 `rxBufferSize - 1`
   - 在 START 时触发 RX
   - 在 STOP 或 RX DMA 完成时，读取 `DMA_I2CR_DONE`
   - 若地址字节被捕获到缓冲区则规范化封包
   - 将完成的封包排入已有解析器路径
7. 保持 DMA 故障处理明确：
   - TX 覆写时清除 `TXOVW`
   - RX 丢失时清除 `RXLOSS`
   - 干净地停止受影响的 DMA 方向
   - 重新启用匹配的 I2C 从机中断回退路径
8. 添加验证日志或计数器：DMA TX 完成长度、DMA RX 完成长度、`RXLOSS`、`TXOVW`、ACK 错误、最终回复长度

## 验证清单

- TX DMA 回复长度与协议封包长度匹配
- 重复开始读取事务不会在 STOP 后仍保持 TX DMA 活跃
- RX DMA 不会重复或丢失地址字节
- DMA 启用时无明显 LED 变暗或扫描抖动
- `RXLOSS` 和 `TXOVW` 均被清除和计数
- 超时和 STOP 仍正确重置 I2C 传输状态

## 在仓库中的参考位置

- `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- `Project/STC51/ws2812_driver/Sources/inc/gp_led_matrix_ai8051u.h`
- `Project/STC51/ws2812_driver/Sources/app/app.c`

活跃默认配置：RX DMA 启用、TX DMA 启用、协议成帧 ISR 保留、两个方向使用已有 `rxBuffer`/`txBuffer`

## DMA 启用顺序

1. 确认原始仅中断路径仍可工作
2. 启用 RX DMA 和 TX DMA 进行封包负载传输
3. 对比 `DMA_I2C_ReadTxDone()` 和 `DMA_I2C_ReadRxDone()` 与预期封包大小
4. 确认地址字节未被保留为虚假的第一个负载字节
5. 对比封包长度、校验和成功率、DMA 故障计数器与仅中断基线
