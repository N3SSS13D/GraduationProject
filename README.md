# WS2812 驱动毕业设计项目

## 项目简介

本项目基于 STC AI8051U，实现 WS2812 显示驱动与 USB 控制链路。

当前分支（`way2`）已完成一轮模块化重构：

- `ws2812_drv` 负责图像缓冲、行编码、双通道 PWM+DMA 发送与 DMA 异常恢复。
- `test.c` 保留为轻量测试调度层，负责固定图案构建和 Timer0 扫描节拍。
- 已针对“PWM 双通道偶发错乱”完成多层防护并沉淀问题文档。

## 当前硬件方案

- MCU: STC AI8051U
- WS2812 输出: PWMA + DMA
- 行选硬件: 两颗级联 74HC595，控制 16 路 PMOS 高侧
- 行通道复用: 偶数行走 CH1/P10，奇数行走 CH2/P12
- 通信: USB CDC OUT 命令解析

## 当前代码结构（与工程一致）

```
GraduationProject/
├── README.md
├── Doc/
│   ├── ws2812_driver_current_implementation.md
│   └── ...
└── STC51/
    └── Project/
        └── ws2812_driver/
            ├── Sources/
            │   ├── app/
            │   │   └── test.c
            │   ├── drv/
                        │   │   ├── ws2812_drv.c
            │   │   └── hc595_drv.c
            │   ├── inc/
            │   │   ├── test.h
                        │   │   ├── ws2812_drv.h
            │   │   ├── timer.h
            │   │   └── ...
            │   ├── main.c
            │   ├── timer.c
            │   ├── exti.c
            │   └── usblib.c
            ├── Objects/
            ├── Listings/
                        ├── problem.md
                        ├── problem_zh.md
            └── ws2812_driver.uvproj
```

## 主要实现状态

已实现：
- `ws2812_drv` 统一驱动接口：
    - 图像缓冲管理（清空/写像素）
    - 全行 PWM 缓冲编码
    - 双行交织发送缓冲生成
    - PWMAT DMA 触发、等待、超时恢复
    - DMA ISR 状态释放接口
- `test.c` 调度链路：
    - 固定 16x8 梯形图案构建
    - Timer0 one-shot 触发行对扫描
    - 行对发送委托到 `WS2812DRV_SendRowPair`
    - `PWMAT_DMA_ISR` 转发到 `WS2812DRV_OnDmaIsr`
- 74HC595 行选原子化（移位 + 锁存期间关中断）
- USB 最小控制闭环（行间隔设置 + 状态回显）
- 通道错乱问题复盘文档（中英文）

说明：
- 当前应用入口仍是 `Sources/app/test.c`，但底层发送能力已下沉到 `Sources/drv/ws2812_drv.c`。
- `README` 只维护当前可运行实现，详细说明见 `Doc/ws2812_driver_current_implementation.md`。
- 每次关键问题与修复会同步追加到 `.github/prompts/ws2812-led-system-dev*.prompt.md` 的迭代总结段。

## 控制指令速查

- 换行间隔：`T=ddddus` / `T=ddddms` / `T=dddds`

命令执行后会打印 `[STATE]` 状态行，包含 `row_interval_us` 与 `pwm_us`。

## 构建与运行

1. 使用 Keil 打开 `STC51/Project/ws2812_driver/ws2812_driver.uvproj`
2. 编译目标并生成固件
3. 通过 STC ISP 工具下载
4. 连接 USB CDC，发送控制命令验证行为

## 参考文档

- 当前实现总结: `Doc/ws2812_driver_current_implementation.md`
- 问题复盘（英文）: `STC51/Project/ws2812_driver/problem.md`
- 问题复盘（中文）: `STC51/Project/ws2812_driver/problem_zh.md`
- 迭代 Prompt（中文）: `.github/prompts/ws2812-led-system-dev.zh-CN.prompt.md`
- 迭代 Prompt（英文）: `.github/prompts/ws2812-led-system-dev.prompt.md`
- 芯片与库资料: `STC51/Tools/`

## 备注

当前工程处于“驱动能力沉淀 + 接口复用”阶段，后续将继续：

- 基于 `ws2812_drv` 增加新图案与业务模块，不破坏发送链路。
- 在保持时序确定性的前提下优化测试覆盖与状态可观测性。

