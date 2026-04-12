# WS2812 Graduation Project

## 1. 项目概述

本项目基于 STC AI8051U（8051 架构），实现 16x16/16x8 WS2812 点阵显示、USB 命令控制、按键模式切换与动画渲染。

当前分支 `way2` 的核心方向：

- 渲染层与发送层解耦（`draw_drv` + `ws2812_drv`）
- USB 控制统一为 `PLAY` 参数模型
- 支持静态图案、静态字模、滚动字幕、渐变/呼吸/淡入淡出等效果
- 保持行扫描链路时序稳定并持续做性能优化

## 2. 硬件与链路

- MCU: STC AI8051U
- LED: WS2812（矩阵扫描）
- PWM 输出: PWMA CH1/CH2
- 发送: PWMAT DMA
- 行选: 74HC595 + PMOS 高侧
- 控制输入: USB CDC、P3.2(INT0) 低电平按键

## 3. 代码结构

```text
GraduationProject/
|-- README.md
|-- Doc/
|   |-- usb_play_v2_guide.md
|   |-- ws2812_driver_current_implementation.md
|   `-- xiaozhi_esp32_porting_summary.md
|-- External/
|   `-- xiaozhi-esp32/          # 小智 AI 参考快照（不保留原 Git 元数据）
`-- STC51/
    `-- Project/
        `-- ws2812_driver/
            |-- Sources/
            |   |-- app/
            |   |   `-- test.c              # 应用层参数封装、预设模式
            |   |-- drv/
            |   |   |-- ws2812_drv.c        # 图像/行PWM编码/DMA发送
            |   |   `-- hc595_drv.c         # 行选驱动
            |   |-- mid/
            |   |   |-- draw_drv.c          # 渲染重建与动画步进
            |   |   `-- key_ctrl.c          # 按键去抖与模式切换
            |   |-- inc/
            |   |   |-- test_image.h        # 图案+字模统一资源头文件
            |   |   |-- draw_drv.h
            |   |   |-- test.h
            |   |   `-- ws2812_drv.h
            |   |-- usblib.c                # PLAY 命令解析与下发
            |   |-- exti.c
            |   `-- main.c
            `-- ws2812_driver.uvproj
```

## 4. 功能说明

### 4.1 显示内容

- 图案显示：内置 diamond/cross/python_demo
- 字模显示：支持静态字模索引显示与滚动字模序列
- 方向旋转：0/180/CW90/CCW90

### 4.2 动效与颜色

- 静态、呼吸、渐变、左滚、右滚、JLU 文字滚动、淡入、淡出、颜色循环
- 前景/背景 RGB888
- 亮度控制（0..255）
- 滚动步进按“像素/帧”定义，最小 1 px/frame

### 4.3 交互控制

- USB `PLAY` 命令统一参数入口
- P3.2(INT0) 低电平按键，循环切换 4 组预设模式

## 5. USB 命令

项目已使用 PLAY v2 模型，详细参数与样例见：

- `Doc/usb_play_v2_guide.md`

关键参数：

- `CT` 内容类型、`FX` 效果、`DIR` 方向
- `SPD` 滚动速度（像素/帧，最小 1）
- `BR` 亮度
- `GI` 静态字模索引
- `SQ` 滚动字模序列

## 6. 本轮性能优化

本版本新增了图像重建与编码路径的加速策略：

1. 快速帧写入路径
   - 新增 `WS2812DRV_BeginFrameWrite/SetPixelRgbFast/EndFrameWrite`
   - 整帧重建时跳过逐像素比较，减少 CPU 分支与重复 dirty 判断

2. 行编码位展开查表
   - 新增 8-bit -> 8 PWM 占空序列 LUT
   - 替代每 bit 的移位判断，降低编码 CPU 占用

3. M2M DMA 参与缓冲清零（带回退）
   - 优先尝试 M2M DMA 对后图像缓冲做块清零
   - 若 M2M 异常/超时，自动回退到 CPU 清零路径

## 7. 构建与验证

1. Keil 打开：`STC51/Project/ws2812_driver/ws2812_driver.uvproj`
2. Build 生成固件
3. STC ISP 下载
4. 串口发送 PLAY 命令验证显示、速度和按键模式切换

### 7.1 快速验证建议

- 先发送静态图案命令确认链路：`PLAY CT=0 FX=0 IMG=2 BR=180`
- 再发送静态字模命令确认 GI：`PLAY CT=1 FX=0 GI=2 BR=180`
- 最后发送滚动序列命令确认 SQ/SPD：`PLAY CT=1 FX=5 SPD=1 SQ=0,1,2,3`

## 8. 参考文档

- `Doc/ws2812_driver_current_implementation.md`
- `Doc/项目文档/xiaozhi_esp32_porting_summary.md`
- `Doc/usb_play_v2_guide.md`
- `STC51/Project/ws2812_driver/problem.md`
- `STC51/Project/ws2812_driver/problem_zh.md`

## 9. 提交边界建议

为避免污染仓库，建议默认不提交以下本地临时/环境文件：

- Keil 本地视图状态：`*.uvgui.*`、`*.uvopt`
- Python 运行缓存：`__pycache__/`、`*.pyc`
- 临时导出图片（若非明确资源需求）：`Pic/` 下的中间产物


