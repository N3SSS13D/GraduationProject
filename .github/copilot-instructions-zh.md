# 项目通用编码规范（中文版）

## 项目结构

本仓库当前按 4 类活跃内容组织，目的是让任务只读取必要目录，不默认扫描无关文件：

1. `LED端显示驱动`
   - 路径：`Project/STC51/`，尤其是 `Project/STC51/ws2812_driver/`
2. `AI端接口调度`
   - 路径：`Project/xiaozhi-esp32/main/gp_port/` 及必要的板级文件
3. `蓝牙通信协议`
   - 路径：`Project/Protocols/`
4. `本地绘图脚本`
   - 路径：`Project/Script/`

当前有效说明文档入口：

- `Doc/Instructions/README.md`
- `Doc/Instructions/project_structure.md`
- `Doc/Instructions/problem_tracking.md`
- `Doc/Instructions/bt_version_hc05_uart2_architecture.md`
- `Project/STC51/README.md`
- `Project/xiaozhi-esp32/main/gp_port/README.md`
- `Project/Protocols/README.md`
- `Project/Script/README.md`

处理任务时，只根据结构说明定位相关分类文件；不要默认读取无关分类目录。
当任务明确落在某一类时，先读取该分类 README，获取模块职责、主链路和最小阅读集，再进入更深层实现文件。

## 项目技能

- 在本仓库中进行代码编写、评审或重构时，应用 `.github/skills/karpathy-guidelines/SKILL.md`
- 按任务类别优先使用对应 prompt 或 skill：
   `AI端接口调度` -> `.github/prompts/ws2812-ai-control*.prompt.md` + `.github/skills/karpathy-guidelines/SKILL.md`
   `LED端显示驱动` -> `.github/prompts/ws2812-led-driver*.prompt.md` + `.github/skills/ws2812-led-driver/SKILL.md`
   `蓝牙通信协议` -> `.github/prompts/ws2812-bluetooth-protocol*.prompt.md` + `.github/skills/bluetooth-protocol/SKILL.md`
   `本地绘图脚本` -> `.github/prompts/ws2812-local-scripts*.prompt.md` + `.github/skills/local-drawing-scripts/SKILL.md`
- 具体要求：编码前先明确假设，优先选择能满足需求的最简单改动，严格控制改动范围，并在实现前定义可验证的完成标准
- 如果任务可以通过文档更新或小范围修正解决，不要额外引入新的抽象或推测性的配置项

## 代码风格

- 自写代码遵守规范，外部/第三方代码保留原风格
- 引用文件顺序：先不稳定后稳定，减少编译时间
- 四空格缩进；统一采用 K&R/BSD 花括号风格
- 相对独立的代码块之间加空行（每次只加一行）
- 用空格对齐代码；最长宏名与其值之间的空格不超过 4 个
- 行宽不超过 120 字符；在逻辑边界处合理换行
- 每行只写一条语句，禁止一行多句
- 条件和循环语句必须加花括号 `{}`，即使只有一行代码也不例外
- 开花括号与控制语句写在同一行（K&R 风格）
- 操作符前后空格规则：逗号后加一个空格；单目运算符（`!`、`*`、作解引用用的 `&` 等）前后不加空格；双目运算符（`=`、`==`、`+`、`-`、`&`、`|` 等）前后各加一个空格；`->` 和 `.` 前后不加空格；关键字（`if`、`for`、`while` 等）后加一个空格；函数名与参数列表之间不加空格
- 复杂表达式避免过度依赖运算符优先级，使用 `()` 明确表达意图
- 宏函数按正常语句格式书写，每个续行行末加行续接反斜杠 `\`（告知预处理器宏定义延续到下一行）

## 命名规范

- **宏、常量、枚举值、goto 标签**：纯大写加下划线，前加模块名前缀（例如 `GPIO_MAX_COUNT`）
- **函数、枚举类型、结构体类型、联合体类型**：大驼峰（例如 `GetLevel`）；对外公布的函数接口可在大驼峰前加全大写模块名缩写，用下划线分割（例如 `GPIO_GetLevel`）
- **全局变量**：小驼峰；建议加 `g_` 前缀便于查找（例如 `g_sensorValue`）
- **局部变量**：小驼峰；尽量简短，上下文能看出含义即可；范围越大的变量应命名越精细
- 在 8051/STC 侧 C 代码中，禁止使用 `data` 作为变量名、参数名或字段名；该工具链可能将其识别为存储类关键字
- **函数参数、宏参数、结构体成员、联合体成员**：小驼峰（例如 `bufLen`）
- **文件/文件夹名**：纯小写，下划线分割，格式为 `模块名_功能描述`（例如 `gpio_driver.c`）
- **typedef**：使用匿名类型；指针自嵌套可增加 `tag` 前缀或下划线后缀；禁止对基本数值类型重定义
- 宏以实际功能命名；减少使用函数式宏
- 使用项目标准缩写（例如 `addr`、`buf`、`len`、`src`、`dest`、`ret`、`cfg`、`err`），保持全项目一致

## 代码质量

- 在函数开头统一定义所有局部变量，置于可执行语句之前
- 函数有效行数不超过 **80 行**；超过时将独立子功能封装为新函数
- 函数参数不超过 **5 个**；超过时采用结构体封装传参
- 避免高难度、高技巧、高复杂性的语句
- 函数高内聚、低耦合，每个函数只做一件事，职责清晰
- 禁止使用魔鬼数字，所有字面量改用命名常量或枚举值
- 禁止在宏定义中使用 `return`
- 减少全局变量的使用，尽量使用局部变量
- 避免使用 `extern`；函数在头文件中声明；全局变量用 `static` 修饰，通过 `Get`/`Set` 函数访问
- 只使用 ISO C 标准字符
- 变量在使用前初始化（允许在使用处初始化，不必在定义时）；指针和全局变量必须在定义时初始化
- 每个头文件必须有文件级注释块，至少包含：文件名、作者、创建日期、版本号
- 注释采用 Doxygen 风格，使用 `/* */` 块注释；注释使用英文；禁止嵌套注释；不用的代码直接删除而非注释掉；避免在代码行中间插入注释
- 每次修改代码后，需在改动处适当补充注释，说明代码结构与功能（遵循现有注释风格，保持简洁，避免冗余注释）
- 每次完成编码任务后，需输出 Markdown 格式的总结报告，至少包含：改动概述、影响文件、关键逻辑更新、验证状态
- 自定义函数时，必须确保不调用不存在的函数；并且所有自定义函数在使用前必须在对应头文件中声明
- 每次优化或行为变更后，若 `.github/prompts/`、`.github/skills/`、`Doc/Instructions/`、`Project/Protocols/` 或 `Project/Script/` 下相关内容的前提、流程或任务边界受到影响，必须同步更新
- 每次修改本仓库源码后，自动对 `Project/STC51/ws2812_driver/ws2812_driver.uvproj` 执行 Keil rebuild，分析构建错误并持续修复直到通过；除非用户明确要求，否则不要执行下载/烧录

## WS2812 驱动文档同步

- 当修改 WS2812 扫描/输出时序行为时，需要同步更新 `Doc/Instructions/` 中的当前文档，以及 `Project/STC51/` 下对应 LED 侧说明
- 若扫描模式逻辑发生变化，必须明确记录：通道映射规则、关断行波形类型、复位尾波行为、间隔安全约束
