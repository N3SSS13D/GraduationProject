---
name: WS2812 LED系统开发助手（中文）
description: "面向 STC AI8051U + WS2812 复用显示系统，实现一个可增量交付的功能"
argument-hint: "要实现的功能（例如：16x16 文字滚动、8x32 分辨率模式、低抖动调度器、小智AI图案桥接）"
agent: agent
model: "GPT-5 (copilot)"
---
基于用户参数，为该 WS2812 LED 显示项目只实现一个“增量功能”。

项目硬件架构（固定约束）：
- MCU：STC AI8051U
- 供电开关：16 路 PMOS 高侧开关
- 开关控制：2 颗级联 74HC595 控制 PMOS 开关状态
- 信号线：奇数行并联一根 PWM 信号，偶数行并联一根 PWM 信号
- 驱动方式：PWM + DMA
- 扫描策略：每次给相邻两行上电；第一行逐像素熄灭、第二行逐像素点亮；完成后开关窗口移动一位并循环

优先阅读的代码上下文：
- 主驱动源码目录：[STC51/Project/ws2812_driver/Sources](../../STC51/Project/ws2812_driver/Sources)
- 现有底层示例：[STC51/Project/PWMA-DMA/AI8051U-PWMA-DMA-24灯环-1通道.c](../../STC51/Project/PWMA-DMA/AI8051U-PWMA-DMA/WS2812B-PWMA-DMA-24灯环-1通道.c)

当前代码结构（必须优先贴合真实仓库）：
- App 应用层：`Sources/app/`
- Mid 中间层：`Sources/mid/`
- Drv 驱动层：`Sources/drv/`
- 共享头文件与配置：`Sources/inc/`
- 外设入口与厂家库支撑：`Sources/*.c`、`Sources/lib/`

文件落位规则：
- 优先复用当前目录，不为追求“理想分层”而做大规模迁移。
- 新增业务编排优先放 `app/`，算法或状态机优先放 `mid/`，时序与硬件驱动优先放 `drv/`。
- 若任务涉及小智 AI 桥接，先检查 `External/xiaozhi-esp32/GP_Port/` 是否已有可复用协议或接口定义。

执行要求：
0. 开始分析、编写、评审或重构前，先读取并应用 `.github/skills/karpathy-guidelines/SKILL.md`，明确假设、选择最小改动，并先定义验证标准。
1. 先分析现有代码并总结可复用部分。
2. 仅针对本次功能提出最小实现方案。
3. 按现有风格和命名规范直接修改工作区代码。
4. 默认分辨率保持 16x16 兼容，同时支持可配置分辨率。
5. 若功能涉及任务调度，实现轻量协作式（非抢占）mini-OS 风格调度器，适配 8051 资源约束。
6. 若功能涉及小智AI控制，优先将 `voice_color_result` 或等价动作对象映射为显示动作，并与 `GP_Port/gp_led_matrix_protocol.h` 保持兼容。
7. 增补简明技术文档，说明集成方式与使用方法。
8. 每次修改代码后，自动执行当前任务对应的验证流程；涉及本仓库联调顺序时，优先使用 `tools/ws2812_dev_cycle.ps1` 作为统一自动化入口。
9. 若当前改动涉及 `BT_Version` 的 AI8051U 蓝牙调试链路，验证时优先使用 `-RunAi8051BtDebug`，自动打开 AI8051 serial monitor，发送 `BT AT`、`BT AT+UART?` 并读取返回日志。

约束：
- 保证时序关键路径具备确定性。
- 除非有充分理由，避免动态内存分配。
- 优先使用定长缓冲区并进行边界检查。
- 严格依赖方向：`app -> mid -> drv -> 外设入口/厂家库`。
- 不重写与本任务无关的模块。

当前仓库阶段：
- `STC51/Project/ws2812_driver` 已具备稳定的 PWM + DMA、74HC595/PMOS 行扫描与 USB 调试命令基础。
- `External/xiaozhi-esp32/GP_Port/` 已具备协议头、ESP32 驱动骨架、AI8051U 接口设计、MCP 调试工具与联调脚本。
- 稳定版主目标已完成“小智 AI -> I2C 自定义协议 -> AI8051U -> WS2812 显示动作”闭环；`BT_Version` 当前本地联调主目标是保持 AI8051U 侧 `UART2(P4.2/P4.3) + HC-05` 链路稳定，新增 `P4.1 -> HC-05 PIO11` AT 模式控制，系统主时钟固定为 `33.1776MHz`，上电默认 `9600 8N1`，使用 USB 仅做结构化 `BT SEND/BT STATUS` 蓝牙调试命令转发与 UART2 收发日志监视，并继续保留 ESP32 侧传输抽象与经典蓝牙 SPP 链接限制记录。
- 在 `BT_Version` 上，当前 AI8051U 串口测试应直接使用 `P4.2/P4.3` 上的 UART2，保持 `gp_led_matrix_ai8051u` 为主运行路径，避免把 Timer2 复用为本地计时调试资源，并在执行 `AT+UART=...` 后让 AI8051U 本地 UART2 波特率同步切换到新的目标值。

输出格式：
- "Assumptions"
- "Plan"
- "Files changed"
- "Verification"（实际执行的构建/测试/静态检查；若未执行需说明原因）
- "Next steps"

验证补充要求：

- 只要修改了源代码，就不要停在“建议如何测试”；应直接执行可用的自动化验证。
- STC51 侧源码修改后，至少执行一次 `STC51/Project/ws2812_driver/ws2812_driver.uvproj` 的 Keil 重编译，直到构建成功。
- 若改动影响小智、MCP、AI8051U 联调边界，同步执行 `tools/ws2812_dev_cycle.ps1` 对应流程，并把实际验证结果写入 "Verification"。

如果需求存在歧义，只提最少且必要的问题后继续推进。

---

## 迭代问题总结（持续追加）

### 2026-04-08

- 问题：双通道 PWM+DMA 下出现特定通道组偶发错乱。
- 根因：发送长度配对、DMA 源地址对齐、尾部边界抖动与异常恢复不足共同导致。
- 解决：
	- 强制偶数长度发送；
	- 固定 DMA 缓冲对齐地址；
	- 增加 CH1/CH2 尾部零保护对；
	- 增加 DMA 超时恢复；
	- 固定偶数行->CH1、奇数行->CH2 映射。
- 结构演进：将图像缓冲和 PWM+DMA 发送能力下沉到 `ws2812_drv`，`test.c` 仅保留调度与测试图案。

### 2026-04-12

- 问题：需要将 `xiaozhi-esp32` 中已完成的小智 AI / MCP / 调试圆点能力整理进毕业设计仓库，作为后续 8051 接口层与显示联动开发参考。
- 结论：
	- 已将 `xiaozhi-esp32` 快照导入 `External/xiaozhi-esp32/`。
	- 快照包含 `GP_Port/` 下的 LED 矩阵协议、AI8051U 接口设计、调试圆点显示、MCP 工具与桥接测试脚本。
	- 官方 `wss://api.xiaozhi.me/mcp/?token=...` 桥接端实测会主动发 `initialize`，因此测试脚本必须支持 server 模式。
	- `lichuang-dev` 当前调试界面已收敛为稳定的单页次级菜单，保持 `Back / Debug Menu / S` 固定标题栏，并同时提供圆点预览、链路状态、触摸控制与摘要信息区域。
- 后续建议：
	- 将 `voice_color_result` 结构映射为 STC 侧 WS2812 动作参数；
	- 在 `Sources/app/` 增加 AI 输入适配层；
	- 在 `Sources/drv/` 或 `Sources/mid/` 增加颜色/动画动作接口；
	- 稳定版继续围绕 I2C 自定义协议收敛；`BT_Version` 当前优先围绕 AI8051U `UART2 + HC-05` 传输闭环与协议回包路径做最小实现与验证。
