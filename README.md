# WS2812 LED驱动项目

## 项目简介

本项目是一个基于STC AI8051U单片机的WS2812 LED灯带驱动系统毕业设计项目。项目采用Keil MDK开发环境，实现了WS2812 LED灯带的驱动控制功能，支持USB通信接口。

## 技术栈

- **主控芯片**: STC AI8051U (32位8051内核)
- **开发环境**: Keil MDK-ARM (uVision)
- **编程语言**: C语言
- **通信接口**: USB CDC (虚拟串口)
- **目标设备**: WS2812 RGB LED灯带

## 项目结构

```
GraduationProject/
├── Doc/                                    # 项目文档目录
│   ├── WS2812BLED数据手册.pdf             # WS2812 LED数据手册
│   ├── 代码规范.pdf                        # 代码编写规范文档
│   └── 题目.docx                           # 毕业设计题目文档
│
├── STC51/                                  # STC51项目主目录
│   ├── Project/                            # 项目工程目录
│   │   └── ws2812_driver/                 # WS2812驱动项目
│   │       ├── Sources/                   # 源代码目录
│   │       │   ├── app/                   # 应用层代码（待实现）
│   │       │   ├── cfg/                   # 配置文件目录
│   │       │   ├── driver/                # 驱动层代码（待实现）
│   │       │   ├── hal/                   # 硬件抽象层代码（待实现）
│   │       │   ├── utils/                 # 工具函数目录（待实现）
│   │       │   ├── inc/                   # 头文件目录
│   │       │   │   ├── ai8051u_def.h      # AI8051U芯片定义
│   │       │   │   ├── config.h           # 项目配置文件
│   │       │   │   ├── def.h              # 通用定义文件
│   │       │   │   └── usblib.h          # USB库头文件
│   │       │   ├── lib/                   # 库文件目录
│   │       │   │   ├── stc_usb_cdc_32g.lib    # USB CDC库（32位）
│   │       │   │   └── stc_usb_hid_32g.lib    # USB HID库（32位）
│   │       │   ├── main.c                 # 主程序文件
│   │       │   ├── usblib.c               # USB库实现文件
│   │       │   └── output/                # 输出文件目录
│   │       ├── Objects/                   # 编译输出目录
│   │       │   ├── ws2812_driver.hex      # 编译生成的HEX文件
│   │       │   └── ...                    # 其他编译中间文件
│   │       ├── Output/                    # 输出文件目录（备份）
│   │       ├── Listings/                  # 列表文件目录
│   │       ├── ws2812_driver.uvproj       # Keil项目文件
│   │       ├── ws2812_driver.uvopt        # Keil选项文件
│   │       └── COMMIT_CONVENTION.md       # Git提交规范文档
│   │
│   └── Tools/                              # 开发工具目录
│       ├── AI8051U_uCOS-II_Xsmall+Large/  # uCOS-II实时操作系统库
│       ├── AI8051U.pdf                    # AI8051U芯片手册
│       ├── AiCube-ISP-v6.96G.exe         # STC ISP下载工具
│       ├── STC_USB_LIBRARY/               # STC USB库文件
│       │   ├── ai_usb.h                   # USB库头文件
│       │   ├── stc32_stc8_usb.h           # STC32/STC8 USB定义
│       │   ├── 库文件/                    # USB库文件集合
│       │   │   ├── STC-CDC库文件/         # CDC虚拟串口库
│       │   │   └── STC-HID库文件/         # HID设备库
│       │   └── 范例程序/                  # USB库使用示例
│       └── 实验箱AI8051U-SCH.pdf          # 实验箱原理图
│
└── README.md                               # 项目说明文档（本文件）
```

## 目录说明

### 核心目录

#### `Sources/` - 源代码目录
- **`main.c`**: 主程序入口，包含系统初始化、主循环等核心逻辑
- **`app/`**: 应用层代码目录，用于存放业务逻辑代码
- **`driver/`**: 驱动层代码目录，用于存放WS2812等外设驱动代码
- **`hal/`**: 硬件抽象层代码目录，用于存放硬件相关抽象接口
- **`utils/`**: 工具函数目录，用于存放通用工具函数
- **`inc/`**: 头文件目录
  - `config.h`: 项目配置，包含系统时钟、USB配置等
  - `def.h`: 通用宏定义和类型定义
  - `ai8051u_def.h`: AI8051U芯片相关定义
  - `usblib.h`: USB库接口定义
- **`lib/`**: 静态库文件目录，包含STC官方USB库

#### `Objects/` - 编译输出目录
- 包含编译生成的HEX文件、OBJ文件、列表文件等

#### `Output/` - 输出文件目录
- 编译输出的备份目录

### 配置文件

#### `ws2812_driver.uvproj`
Keil MDK项目文件，包含：
- 目标芯片配置（AI8051U-32Bit Series）
- 编译选项设置
- 文件组织结构
- 链接器配置

#### `config.h`
系统配置文件，主要配置项：
- 系统时钟频率：40MHz
- USB通信配置
- 编译选项

## 系统架构

项目采用分层架构设计：

```
┌─────────────────┐
│   Application   │  应用层（app/）
│      Layer      │  业务逻辑实现
├─────────────────┤
│    Driver       │  驱动层（driver/）
│      Layer      │  外设驱动实现（WS2812等）
├─────────────────┤
│      HAL        │  硬件抽象层（hal/）
│      Layer      │  硬件接口抽象
├─────────────────┤
│   Hardware      │  硬件层
│      Layer      │  AI8051U + WS2812
└─────────────────┘
```

## 开发环境配置

### 必需软件
1. **Keil MDK-ARM** (uVision)
   - 版本要求：支持MCS-251工具链
   - 用于项目编译和调试

2. **STC ISP下载工具** (AiCube-ISP)
   - 位置：`STC51/Tools/AiCube-ISP-v6.96G.exe`
   - 用于程序下载到单片机

### 编译配置
- **工具链**: MCS-251
- **目标芯片**: AI8051U-32Bit Series
- **系统时钟**: 40MHz
- **内存配置**: 
  - IRAM: 0-0x7FF
  - XRAM: 0x10000-0x17FFF
  - IROM: 0xFF0000-0xFFFFFF

## 功能特性

### 已实现功能
- ✅ 系统初始化（GPIO、USB等）
- ✅ USB CDC虚拟串口通信
- ✅ 基础延时函数（微秒级、毫秒级）

### 待实现功能
- ⏳ WS2812 LED驱动实现
- ⏳ RGB颜色控制
- ⏳ LED灯带效果（流水灯、渐变等）
- ⏳ USB命令解析与控制

## 代码规范

项目遵循以下代码规范：
- 参考 `Doc/代码规范.pdf` 中的详细规范
- Git提交遵循 `COMMIT_CONVENTION.md` 中的Conventional Commits规范

### 提交信息格式
```
<类型>(<范围>): <主题>

<详细描述>

<脚注>
```

**类型**: feat, fix, docs, style, refactor, perf, test, chore, build, ci  
**范围**: driver, app, hal, utils, config, usb

## 使用说明

### 编译项目
1. 使用Keil MDK打开 `ws2812_driver.uvproj`
2. 选择目标配置
3. 点击编译按钮（F7）或使用菜单 Build → Build Target

### 下载程序
1. 使用STC ISP工具打开生成的HEX文件
2. 连接USB线到开发板
3. 点击下载按钮进行程序烧录

### 运行调试
- 通过USB CDC虚拟串口与PC通信
- 使用串口调试工具查看输出信息

## 参考资料

- **WS2812数据手册**: `Doc/WS2812BLED数据手册.pdf`
- **AI8051U芯片手册**: `STC51/Tools/AI8051U.pdf`
- **实验箱原理图**: `STC51/Tools/实验箱AI8051U-SCH.pdf`
- **USB库使用说明**: `STC51/Tools/STC_USB_LIBRARY/库文件使用说明.txt`

## 开发计划

### 第一阶段：基础驱动
- [ ] 实现WS2812时序驱动
- [ ] 实现单LED控制
- [ ] 实现RGB颜色设置

### 第二阶段：功能扩展
- [ ] 实现多LED灯带控制
- [ ] 实现常用LED效果（流水灯、呼吸灯等）
- [ ] USB命令接口实现

### 第三阶段：优化与测试
- [ ] 性能优化
- [ ] 稳定性测试
- [ ] 文档完善

## 版本历史

- **V1.0** (2025-12-13)
  - 项目初始化
  - 基础框架搭建
  - USB通信基础功能

## 许可证

本项目为毕业设计项目，仅供学习研究使用。

## 联系方式

如有问题或建议，请联系项目开发者。

---

**注意**: 本项目仍在开发中，部分功能尚未实现。请参考代码注释和文档进行开发。

