# WS2812 驱动当前实现与控制指令总结

## 0. 最新状态（2026-04-11）

- `Sources/app/test.c` 已重构为轻量测试调度层。
- PWM+DMA 双通道发送、图像缓冲、行编码、DMA恢复机制统一由 `Sources/drv/ws2812_drv.c` 提供。
- `test.c` 当前只保留：
  - 固定 16x8 梯形图像构建
  - Timer0 one-shot 驱动双行扫描
  - 行间隔参数接口（供 USB 调整）
  - PWMAT 中断转发到 `WS2812DRV_OnDmaIsr`
- 已新增问题复盘文档：
  - 英文：`STC51/Project/ws2812_driver/problem.md`
  - 中文：`STC51/Project/ws2812_driver/problem_zh.md`
- legacy 模式已修正为：
  - 当前行按奇偶绑定固定通道输出图像数据（偶数行 CH1，奇数行 CH2）
  - 上一行对应通道在数据段输出 WS2812 `0` 码（不是全低）
  - 发送尾部追加固定 reset-low 槽，保证锁存/熄灭及时
  - `test.c` 对 legacy 行间隔加入最小安全值钳制，避免 DMA 忙冲突引发闪烁

### 0.1 test 与 ws2812_drv 的职责边界

- `test.c`：
  - 组织测试图案
  - 扫描节拍调度
  - 调用发送接口
- `ws2812_drv.c`：
  - 图像缓冲管理（清空/写像素）
  - 行 PWM 编码
  - 双行交织缓冲生成
  - DMA 触发、等待、超时恢复
  - DMA busy 状态与中断处理

## 1. 概述

本文档总结当前 `STC51/Project/ws2812_driver` 工程中已经实现的驱动相关能力、扫描策略以及可用控制指令。

目标平台与约束：
- MCU: STC AI8051U
- WS2812 输出: PWMA + DMA
- 行选: 两颗级联 74HC595 控 16 路 PMOS 高侧
- 行通道复用: 偶数行 CH1/P10，奇数行 CH2/P12

## 2. 目录落位与分层现状

当前已使用目录（与工程代码一致）：
- `Sources/app/`: 扫描任务与测试控制（当前主流程在 `test.c`）
- `Sources/drv/`: 74HC595 等板级驱动（如 `hc595_drv.c`）
- `Sources/inc/`: 对外头文件与全局配置
- `Sources/`: 自动生成外设初始化/中断与系统入口（`main.c`, `timer.c`, `exti.c`, `usblib.c`）

说明：当前工程尚未全面迁移到 `Sources/output/` 和 `Sources/hal/` 的严格分层；现实现以可运行与时序稳定为优先。

## 3. 已实现驱动能力

## 3.1 WS2812 编码与 DMA 输出

- 使用定长缓冲作为上限：
  - 最大列数 64，PWM 缓冲上限 `64 * 24 + 2`
- 运行时按渲染模式切换有效列数：
  - 16x64 模式：有效列 64
  - 16x8 模式：有效列 8
- 实际 DMA 发送长度按公式动态计算：
  - `dma_len = active_leds * 24 + 2`

### 3.1.1 PWM+DMA 结构分层（当前实现）

- 入口与调度层（`Sources/app/test.c`）
  - 负责扫描状态机、行切换、编码缓存更新、DMA 触发与完成等待。
- 行选驱动层（`Sources/drv/hc595_drv.c`）
  - 负责 74HC595 串行移位与锁存，输出 16 位行选位图。
- 定时调度层（`Sources/timer.c`）
  - 提供 Timer0 one-shot 微秒级挂钩，用于下一行扫描启动时机。

### 3.1.2 PWM+DMA 关键函数与职责

`Sources/app/test.c`:
- `Test_PWMAConfig`
  - 配置 PWMA 基本时基、CH1/CH2 输出和引脚模式。
  - CH1 对应 P10，CH2 对应 P12。
- `Test_SelectPwmChannelByRow`
  - 根据行号奇偶切换 `PWMA_DBA` 目标寄存器基址：
  - 偶数行 -> CCR1（CH1）；奇数行 -> CCR2（CH2）。
- `Test_EncodeRowToPwm`
  - 将指定行像素数据编码为 WS2812 时序占空比序列。
  - 使用 `g_testPwmDmaLen` 控制本次有效发送长度。
- `Test_RebuildPwmCache`
  - 重建所有行的编码缓存 `g_testRowPwmBuf`。
- `Test_RebuildSolidPwmCache`
  - 重建纯色缓存 `g_testSolidPwmBuf`，用于四行窗口方案。
- `Test_PWMATDmaTrig`
  - 设置 DMA 地址和传输长度并触发 PWMA DMA 发送。
  - 设置 `g_testPwmDmaBusy = 1` 进入忙状态。
- `PWMAT_DMA_ISR`
  - DMA 中断仅释放忙标志并清状态位，保持 ISR 最短路径。
- `Test_WaitDmaDone`
  - 在关键路径中等待本次 DMA 发送完成后再继续行后处理。

`Sources/timer.c`:
- `TIMER0_StartOneShotUs`
  - 按目标 us 值启动 one-shot，支持长延时分块。
- `TIMER0_StartOneChunk`
  - 小于阈值走 us 预分频，大于阈值走 ms 预分频，提高区间覆盖能力。
- `TIMER0_ISR`
  - 完成 chunk 链式调度，末次到达后调用 us hook（扫描推进触发点）。

`Sources/drv/hc595_drv.c`:
- `HC595_Write16`
  - 输出 16 位行选位图（低电平位为导通位）。
- `HC595_SelectRows`
  - 双行选通封装接口。

### 3.1.3 PWM+DMA 扫描执行时序（单次行周期）

1. `Test_TaskLoop` 判断 `g_testPwmDmaBusy` 与 `g_testRowTimerPending`，确认可进入下一行。
2. 若缓存脏：调用 `Test_RebuildPwmCache` / `Test_RebuildSolidPwmCache`。
3. 根据扫描方案进行行选与通道选择：
   - classic: `HC595_SelectRows` + `Test_SelectPwmChannelByRow`。
   - quad: `Test_SelectRows4` + `Test_SelectPwmChannelByRow`。
4. 调用 `Test_PWMATDmaTrig(..., g_testPwmDmaLen)` 发起发送。
5. `Test_WaitDmaDone` 等待 `PWMAT_DMA_ISR` 清 busy。
6. 执行行后关断（上一行或当前清除行），调用 `TIMER0_StartOneShotUs` 安排下一次换行。

该流程将“行选切换、PWM寄存器目标切换、DMA发送、换行触发”固定在确定性路径中，避免在关键路径插入软件延时。

## 3.2 行扫描与 PMOS 时序

- 扫描总行数 16。
- 两套行扫描发送模式（在 `ws2812_drv` 内切换）：
  - `normal_pair`：每个扫描周期同时点亮两行（0/1、2/3 ...）。
    - CH1/P10 输出前一行 PWM。
    - CH2/P12 输出后一行 PWM。
    - 每周期后行索引步进 `+2`。
  - `legacy_shift`：每个扫描周期只让后一行输出图像 PWM，前一行输出“关闭码”PWM。
    - 扫描窗口按 `0/1 -> 1/2 -> 2/3 -> ...` 滑动。
    - 每周期后行索引步进 `+1`。
    - 前一行在数据段输出 WS2812 bit0 码；仅在 reset 段保持全低。
- 换行节拍由 Timer0 one-shot 控制，关键路径中不插入软件延时。

## 3.3 74HC595 行选控制

- 提供 16 位移位写入接口 `HC595_Write16`。
- 提供双行选通接口 `HC595_SelectRows`。
- 四行窗口由上层组合 16 位位图后调用 `HC595_Write16`。
- `0` 代表对应行导通（位清零选通），`1` 代表关断。

## 3.4 中断与调试统计

- `PWMAT_DMA_ISR`: 仅释放 DMA busy 标志，保持最短 ISR 路径。
- Timer1: 用于行内配置耗时与行总耗时统计。
- 可输出整帧与逐行统计日志，用于评估扫描稳定性。

## 4. 控制指令（USB OUT 回调）

当前命令解析位于 `Sources/usblib.c`。

## 4.1 颜色命令

- 裸 6 位十六进制：`RRGGBB`
  - 示例：`FF0000`
- 显式格式：`RGB=RRGGBB`
  - 示例：`RGB=00FF80`

执行效果：更新当前纯色参数，并重建图案/纯色缓存。

## 4.2 换行间隔命令

- 格式：`T=ddddus` / `T=ddddms` / `T=dddds`
  - `dddd` 为 4 位十进制数
  - 示例：`T=0020us`, `T=0010ms`, `T=0001s`

执行效果：更新 Timer0 one-shot 目标间隔（内部统一换算为 us）。

## 4.3 图案命令

- 格式：`P=dddd`
  - 示例：`P=0000`, `P=0001`, `P=0002`

执行效果：切换图案索引；超范围值按取模映射到现有图案数量。

## 4.4 渲染模式命令

- 格式：`M=dddd`
  - `M=0000` -> 16x64
  - `M=0001` -> 16x8

执行效果：切换有效列数，并联动刷新 DMA 长度与 PWM 缓存。

## 4.5 状态回显

每次命令处理后，打印状态：
- `method`
- `scheme`
- `render`
- `pattern`
- `rgb`
- `interval_us`

命令不匹配时，返回帮助提示字符串。

## 5. 按键控制（EXTI）

- 离线模式下，P32 (INT0): 预置图案循环切换。
- 离线模式下，P33 (INT1) 短按: 当前图案效果循环切换。
- 任意模式下，P33 (INT1) 长按: 在线/离线模式切换。
- 自动仲裁：检测到小智主机有效通信时切到在线模式；通信超时后回到离线模式。

## 6. 当前默认配置

- 默认 AI8051U I2C 后端：保留 START/STOP 框架中断处理，并默认启用 RX/TX 双向 DMA 进行包数据搬运。
- 默认 DMA 方向策略：`RX DMA=on`，`TX DMA=on`；如需回退或做对比测试，可通过 `GpLedMatrixAi8051u_SetDmaMode()` 单独关闭某个方向。
- 默认 DMA 策略切换接口：`GpLedMatrixAi8051u_SetDmaMode()`。
- 默认显示发送模式：`normal_pair`
- 默认渲染模式：16x64
- 默认换行间隔：2000us
- 默认绘制任务周期：32ms（约 31fps）
- 默认颜色：`R=64, G=0, B=0`

## 7. 已知边界与后续建议

- 当前驱动与应用逻辑仍在 `app/test.c` 中耦合，后续可拆分为：
  - `output/`（显示输出驱动）
  - `hal/`（PWMA/DMA/Timer/EXTI 的芯片访问封装）
- 可增加独立“只读查询命令”（例如 `Q`）返回 `active_leds`、`dma_len`、`scan_row` 等关键运行参数，便于上位机监控。

## 8. 新方式闪烁问题复盘（已修正）

- 现象：legacy 模式设置 700us 行间隔时存在闪烁。
- 根因：legacy 单步发送时间变长（包含上一行关闭码 + 尾部 reset-low），间隔与发送耗时裕量不足，DMA 忙时会出现步进抖动。
- 修复：在 `test.c` 对 legacy 间隔应用最小安全值钳制（基于有效列数与 reset 尾长动态计算）。
