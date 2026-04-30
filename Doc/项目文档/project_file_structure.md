# GraduationProject 文件结构说明

## 1. 结构目标

本仓库已按 `Doc / Project / Demo` 三个一级目录整理，目标是把“文档、可构建工程、演示资源”分离，减少联调脚本、构建工程与 Demo 素材互相混放的问题。

## 2. 一级目录说明

### 2.1 `Doc/`

保持原结构不变，继续存放：

- 毕业论文、计划书、PPT、参考资料
- 项目架构说明、问题跟踪、阶段总结
- 与 `AI端`、`LED端`、蓝牙链路相关的设计文档

重点文档：

- `Doc/项目文档/problem_tracking.md`：问题与约束总入口
- `Doc/项目文档/project_file_structure.md`：当前文件结构说明
- `Doc/项目文档/bt_version_hc05_uart2_architecture.md`：经典蓝牙链路说明

### 2.2 `Project/`

集中存放所有可构建工程、自建脚本和调试产物。

#### `Project/xiaozhi-esp32/`

`AI端` 主工程目录，原 `External/xiaozhi-esp32/` 已迁移至此。

主要子目录：

- `main/`：ESP32 主组件源码
- `main/boards/lichuang-dev/`：当前活跃板级接入路径
- `main/gp_port/`：原 `GP_Port` 中与 ESP32 编译直接相关的自定义矩阵扩展
- `docs/`：小智侧原有说明文档
- `managed_components/`：ESP-IDF 组件依赖
- `scripts/`：上游工程自带脚本

#### `Project/xiaozhi-esp32/main/gp_port/`

该目录承接原 `GP_Port` 中与 `AI端` 编译直接相关的内容。

- `gp_led_matrix_esp32.h/.cc`：ESP32 侧矩阵动作封装与发送逻辑
- `gp_led_matrix_protocol.h`：矩阵协议公共头
- `transport/`：蓝牙矩阵传输抽象与 HC-05 后端
- `ui/`：调试菜单与预览界面扩展

#### `Project/STC51/`

`LED端` 主工程目录，原 `STC51/` 已迁移至此。

主要子目录：

- `Project/ws2812_driver/`：AI8051U + WS2812 主 Keil 工程
- `Project/ws2812_driver/Sources/app/`：应用层逻辑
- `Project/ws2812_driver/Sources/mid/`：动作执行与中间层
- `Project/ws2812_driver/Sources/drv/`：驱动层
- `Project/ws2812_driver/Sources/inc/`：头文件与协议接口
- `Project/ws2812_driver/legacy_gp_port_mirror/`：从旧 `GP_Port` 迁移出的历史镜像文件，便于追溯来源，不作为当前主编译入口

#### `Project/Script/`

集中存放自建脚本，并按用途分类。

主要子目录：

- `tools/`：仓库级联调脚本
- `mcp/gp_matrix/`：矩阵 MCP 桥接脚本与说明文档
- `media_tools/`：图片转换等辅助脚本

关键文件：

- `Project/Script/tools/ws2812_dev_cycle.py`：主联调脚本
- `Project/Script/tools/ws2812_dev_cycle.ps1`：PowerShell 联调脚本
- `Project/Script/tools/dev_env.ps1`：环境变量注入脚本
- `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`：MCP 桥接入口
- `Project/Script/mcp/gp_matrix/gp_mcp_endpoint_client.py`：桥接客户端实现
- `Project/Script/media_tools/led_image_converter_gui.py`：图片转换工具

#### `Project/Debug/`

集中存放调试和构建产物，避免再次污染仓库根目录。

主要子目录：

- `build/`：原根目录构建缓存
- `xiaozhi-esp32-build/`：原 XiaoZhi 工程构建产物
- `debug_snapshots/`：截图、HTTP 预览、联调日志
- `debug_snapshots/dev_cycle_logs/`：`ws2812_dev_cycle.py` 每轮联调日志

### 2.3 `Demo/`

集中存放演示资源与使用说明。

#### `Demo/Pic/`

承接原 `Pic/` 中的图片、压缩包和实物照片目录。

当前内容包括：

- 实物图片
- 图片压缩包
- 演示素材原图

#### `Demo/操作说明.md`

用于说明 Demo 资源分类、当前素材现状和补充规则。

## 3. 根目录保留项说明

以下内容继续保留在仓库根目录，作为仓库级基础设施，不纳入 `Doc / Project / Demo` 的业务分类目录：

- `.git/`：Git 元数据
- `.github/`：Copilot 说明、Prompt、技能配置
- `.vscode/`：VS Code 工作区、任务与调试配置
- `.venv/`：仓库级 Python 虚拟环境
- `README.md`：仓库导航入口
- `.gitignore`：忽略规则

## 4. GP_Port 拆分结果

原 `GP_Port` 已按职责拆分：

- 编译期 `AI端` 扩展 -> `Project/xiaozhi-esp32/main/gp_port/`
- MCP 脚本与协议说明 -> `Project/Script/mcp/gp_matrix/`
- 8051 相关历史镜像 -> `Project/STC51/Project/ws2812_driver/legacy_gp_port_mirror/`

这样处理后：

- ESP32 编译路径只依赖 `main/gp_port/`
- MCP 与截图脚本统一归到 `Script/`
- `LED端` 历史参考文件不再和 `AI端` 源码混放

## 5. 当前推荐入口

- `AI端` 开发入口：`Project/xiaozhi-esp32/`
- `LED端` 开发入口：`Project/STC51/Project/ws2812_driver/ws2812_driver.uvproj`
- 联调入口：`Project/Script/tools/ws2812_dev_cycle.py`
- MCP 调试入口：`Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py`
- Demo 素材入口：`Demo/Pic/`
