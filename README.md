# WS2812 驱动毕业设计项目

## 项目简介

本项目基于 STC AI8051U，实现 WS2812 显示驱动与 USB 控制链路。当前分支重点在“可稳定运行的行扫描驱动路径”，包含 PWM + DMA 数据发送、74HC595 行选控制、按键切换扫描策略、USB 参数调节。

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
            │   │   └── hc595_drv.c
            │   ├── inc/
            │   │   ├── test.h
            │   │   ├── timer.h
            │   │   └── ...
            │   ├── main.c
            │   ├── timer.c
            │   ├── exti.c
            │   └── usblib.c
            ├── Objects/
            ├── Listings/
            └── ws2812_driver.uvproj
```

## 主要实现状态

已实现：
- WS2812 行数据编码与 PWM DMA 发送
- 16 行扫描调度（Timer0 one-shot 驱动）
- 双扫描方案（classic/quad）
- 74HC595 双行/多行选通
- USB 控制命令（颜色、间隔、图案、渲染模式）
- P32/P33 外部中断功能切换
- 帧级与行级时序日志

说明：
- 当前主流程集中在 `Sources/app/test.c`。
- `README` 只维护“当前可运行实现”，详细功能见文档 `Doc/ws2812_driver_current_implementation.md`。

## 控制指令速查

- 颜色：`RRGGBB` 或 `RGB=RRGGBB`
- 换行间隔：`T=ddddus` / `T=ddddms` / `T=dddds`
- 图案选择：`P=dddd`
- 渲染模式：`M=0000` (16x64) / `M=0001` (16x8)

命令执行后会打印 `[STATE]` 状态行，包含 `method/scheme/render/pattern/rgb/interval_us`。

## 构建与运行

1. 使用 Keil 打开 `STC51/Project/ws2812_driver/ws2812_driver.uvproj`
2. 编译目标并生成固件
3. 通过 STC ISP 工具下载
4. 连接 USB CDC，发送控制命令验证行为

## 参考文档

- 当前实现总结: `Doc/ws2812_driver_current_implementation.md`
- 芯片与库资料: `STC51/Tools/`

## 备注

当前工程处于“驱动能力完善”阶段，后续会继续优化分层边界和模块解耦。

