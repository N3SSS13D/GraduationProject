# GraduationProject — Claude 工作指南

## 项目概述

基于蓝牙的 WS2812 LED 矩阵显示系统。四个活跃分类：

| 分类 | 路径 | 说明 |
|---|---|---|
| LED端显示驱动 | `Project/STC51/` | AI8051U MCU，WS2812 矩阵扫描，UART2 协议接收 |
| AI端接口调度 | `Project/xiaozhi-esp32/main/gp_port/` | ESP32-S3，语音/调试动作映射，蓝牙传输 |
| 蓝牙通信协议 | `Project/Protocols/` | 共享协议头与规范文档 |
| 本地绘图脚本 | `Project/Script/` | MCP 桥接、图像转换、自动调试工具 |

## 文件定位规则

- 先根据任务归属分类，只读该分类的相关文件，不要默认扫描无关目录
- 每个分类先读其 README 获取模块图、执行流程、常用文件包，再深入实现文件
- 当前有效文档优先看：
  - `Doc/Instructions/project_structure.md`
  - `Doc/Instructions/problem_tracking.md`
  - `Doc/Instructions/bt_version_hc05_uart2_architecture.md`
  - `Project/STC51/README.md`
  - `Project/xiaozhi-esp32/main/gp_port/README.md`
  - `Project/Protocols/README.md`
  - `Project/Script/README.md`

### 各分类入口文件

**LED端显示驱动：**
- `Sources/app/app.c` — 启动初始化与运行循环
- `Sources/mid/gp_led_action.c` — 远程动作/帧/动画执行
- `Sources/drv/gp_led_matrix_ai8051u.c` — UART2 封包组装、命令分发、ACK/应答
- `Sources/drv/ws2812_drv.c` — 16x16 WS2812 物理扫描输出

**AI端接口调度：**
- `main/gp_port/gp_led_matrix_esp32.h/.cc` — AI端矩阵编排器
- `main/gp_port/transport/` — HC-05/UART 传输后端
- `main/gp_port/ui/` — 本地触摸/调试 UI 与预览缓冲
- `main/boards/lichuang-dev/` — 板级集成

**蓝牙通信协议：**
- `gp_led_matrix_protocol.h` — 共享唯一真相源（线常量、命令ID、负载结构体）
- `gp_led_matrix_protocol_spec.md` — 包级行为规范
- `gp_matrix_pattern_protocol.md` — 主机/脚本绘图契约

**本地绘图脚本：**
- `mcp/gp_matrix/gp_display_mcp_bridge.py` — 主机端绘图桥接
- `mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md` — 工具输入/动画约束/故障避免
- `tools/ws2812_auto_debug.py` — 统一自动调试入口

## 问题解决工作流

对于非平凡的实现、优化或 bug 修复任务：

1. 先总结当前实现切片和确切的控制/数据路径
2. 列出当前问题、风险和候选方案，再开始编辑
3. 选择最小可行变更，并预先定义验证标准
4. 一次只实现一个聚焦的切片，验证后再扩大范围
5. 验证后进行二次审查，若预期行为发生变化则同步更新文档、prompt 和 skill

## 行为准则

- 先陈述假设再编码；如有不确定，直接提问
- 用最少的代码解决问题，不添加未被请求的功能、抽象或"灵活性"
- 精确变更，只改必须改的；不改动相邻代码、注释或格式
- 对多步骤任务，先列出简要计划及其验证标准
- 如果 200 行能写成的代码写了 500 行，重写它

## 代码风格

- 自写代码遵循以下规范；外部/第三方代码保持原风格
- include 文件按稳定性从低到高排序
- 4 空格缩进；K&R/BSD 大括号风格
- 相对独立的代码块之间加一个空行
- 行宽不超过 120 字符；长行在逻辑边界处断行
- 每行只写一条语句
- 条件和循环语句始终使用花括号 `{}`，即使只有一行
- 左花括号放在控制语句同一行（K&R 风格）
- 运算符空格：逗号后加空格；一元运算符无空格；二元运算符两侧各一个空格；`->` 和 `.` 无空格；关键字后加一个空格；函数名与参数列表之间无空格
- 复杂表达式用括号 `()` 明确意图，不依赖运算符优先级
- 宏函数用正常语句格式编写，续行以 `\` 结尾

## 命名规范

- 宏、常量、枚举值、goto 标签：`ALL_CAPS` 加下划线，带模块名前缀（如 `GPIO_MAX_COUNT`）
- 函数、枚举类型名、结构体类型名、联合体类型名：`PascalCase`（如 `GetLevel`）；模块导出接口可加 `ALL_CAPS` 模块缩写和下划线前缀（如 `GPIO_GetLevel`）
- 全局变量：`lowerCamelCase`；可选 `g_` 前缀（如 `g_sensorValue`）
- 局部变量：`lowerCamelCase`；保持简短；作用域越宽命名越具描述性
- 8051/STC 端 C 代码中禁用 `data` 作为变量名/参数名/字段名（工具链可能将其解析为存储类关键字）
- 函数参数、宏参数、结构体成员、联合体成员：`lowerCamelCase`（如 `bufLen`）
- 文件和文件夹名：全小写加下划线，`module_feature` 模式（如 `gpio_driver.c`）
- 使用标准项目缩写：`addr`, `buf`, `len`, `src`, `dest`, `ret`, `cfg`, `err`

## 代码质量

- 所有局部变量在函数开头定义，在可执行语句之前
- 函数不超过 80 有效行；超出则提取独立子任务为辅助函数
- 函数参数不超过 5 个；超出则用专用结构体传递
- 不使用魔数；所有数值字面量替换为命名常量或枚举值
- 宏定义中不使用 `return`
- 最小化全局变量；优先使用局部变量
- 避免 `extern`；所有函数在头文件中声明；全局变量通过 `Get`/`Set` 函数访问；文件作用域全局变量标记 `static`
- 源文件只使用 ISO C 标准字符
- 变量使用前初始化（使用点初始化可接受）；指针和全局变量在定义时必须初始化
- 每个头文件包含文件级注释块：文件名、作者、创建日期、版本号
- 所有注释使用 Doxygen 风格 `/* */` 块语法；注释用英文；不嵌套注释；删除无用代码而不注释掉
- 每次代码变更后在修改处添加适当的解释注释（遵循已有注释风格，保持简洁）
- 添加自定义函数时，只使用已有/有效的 API，确保每个自定义函数在对应头文件中声明
- 每次优化或行为变更后，同步更新相关文档

## WS2812 驱动文档同步

- 更改 WS2812 扫描/输出时序行为时，同步更新 `Doc/Instructions/` 和 `Project/STC51/` 下的文档
- 扫描模式逻辑变更时，明确记录：通道映射规则、关闭行波形类型、复位尾行为、间隔安全约束

## 构建与验证

- STC51 源码修改后，对 `Project/STC51/ws2812_driver/ws2812_driver.uvproj` 运行 Keil 重新构建，分析构建错误，持续修复直到构建成功。除非用户明确要求，否则不执行下载/烧录步骤
- AI端 (ESP32) 修改后，运行 ESP-IDF `build flash monitor`
- 默认自动调试链顺序：
  1. 仅 Keil 重新构建 `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
  2. 等待 20s，打开 AI8051U 串口监视器（默认 `COM15`，可调）
  3. ESP-IDF `build flash monitor` for `Project/xiaozhi-esp32`
- 工具路径：
  - Keil: `S:\Embedded\Keil`
  - ESP-IDF: `S:\Embedded\ESP\v5.4.3\esp-idf`

## 项目 Skills

可用 skills（通过 `/skill-name` 调用）：

| Skill | 用途 |
|---|---|
| `karpathy-guidelines` | 通用编码行为准则（写、审阅、重构时自动应用） |
| `ws2812-led-driver` | LED端显示驱动变更 |
| `bluetooth-protocol` | 蓝牙通信协议变更 |
| `local-drawing-scripts` | 本地绘图脚本与 MCP 工具变更 |
| `ai8051u-i2c-dma` | AI8051U I2C DMA 支持 |

任务分类 → Skill 映射：
- `LED端显示驱动` → `/ws2812-led-driver`
- `AI端接口调度` → 入口文件: `Project/xiaozhi-esp32/main/gp_port/gp_led_matrix_esp32.cc`, `transport/gp_led_matrix_transport.cc`, `ui/gp_debug_display.cc`, `boards/lichuang-dev/lichuang_dev_board.cc`
- `蓝牙通信协议` → `/bluetooth-protocol`
- `本地绘图脚本` → `/local-drawing-scripts`

## 输出格式

每个任务完成后提供简洁总结：
- 变更概述
- 涉及文件
- 验证状态
