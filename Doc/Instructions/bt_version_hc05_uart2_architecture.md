# BT_Version HC-05 与 UART2 架构说明

## 适用范围

本文档只描述当前有效的 `BT_Version` 主链路。

## 结构分类

1. `AI端接口调度`
   - `Project/xiaozhi-esp32/main/gp_port/`
   - `Project/xiaozhi-esp32/main/boards/lichuang-dev/`
2. `蓝牙通信协议`
   - `Project/Protocols/`
3. `LED端显示驱动`
   - `Project/STC51/ws2812_driver/`
4. `本地绘图脚本`
   - `Project/Script/`

## 当前主链路

当前默认链路：

`AI端动作对象 -> 蓝牙协议 -> HC-05 / UART -> LED端协议执行 -> WS2812 矩阵显示`

## 主链路分层职责

1. `AI端接口调度`
   - `gp_led_matrix_esp32.cc` 负责把动作对象、紧凑位图帧和动画批次编码成共享协议包
   - `gp_led_matrix_transport.cc` 负责 `HC-05 / UART` 的发包、后台收包与回包组装
   - `lichuang_dev_board.cc` 负责连接调试界面、主机绘图转发和板级初始化
2. `蓝牙通信协议`
   - `gp_led_matrix_protocol.h` 定义共享包头、命令字、分片大小和动画上限
3. `LED端显示驱动`
   - `gp_led_matrix_ai8051u.c` 负责收包、校验、分发和 ACK
   - `gp_led_action.c` 负责把协议命令转成动作、帧或动画执行
   - `ws2812_drv.c` 负责底层扫描显示
4. `本地绘图脚本`
   - 为 `AI端` 提供 `matrix_pattern_request` 结果和联调工具，不直接承担 `LED端` 的原始串口协议实现

## 当前发送与回包时序

1. `AI端` 根据调试界面、语音或主机绘图结果生成动作对象/帧数据
2. `gp_led_matrix_esp32.cc` 按共享协议拼包，并通过 `gp_led_matrix_transport.cc` 发送
3. `LED端` 在 `gp_led_matrix_ai8051u.c` 中完成组包、校验、命令分发和 ACK 生成
4. `gp_led_action.c` 根据命令切换远程模式、本地模式、单帧显示或动画缓冲
5. `ws2812_drv.c` 输出最终显示；若为本地离线动画，则由 `app.c + draw_drv.c` 的调度链继续刷新

## 当前有效实现边界

- `AI端` 侧接口调度与蓝牙发送逻辑集中在 `Project/xiaozhi-esp32/main/gp_port/`
- `LED端` 侧协议执行与显示驱动集中在 `Project/STC51/ws2812_driver/`
- 协议字段与共享常量集中在 `Project/Protocols/`
- 联调和绘图工具集中在 `Project/Script/`

## 当前验证重点

1. `AI端` 动作对象字段与协议字段保持一致
2. `LED端` 收包、ACK、执行路径保持一致
3. `FrameChunk` 分片大小固定为共享协议常量 `64`
4. 脚本侧绘图输出与协议输入格式一致
