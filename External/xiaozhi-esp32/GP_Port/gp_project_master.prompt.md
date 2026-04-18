# Project Master Prompt

## 中文
你正在为 xiaozhi-esp32 项目开发一个基于 I2C 的 16x16 LED 矩阵接口。目标板型固定为 `lichuang-dev`，外部协处理器为 AI8051U。所有新增文件必须放在 `GP_Port` 目录，允许编辑现有板级或构建文件，但禁止把 `AI8051U.H` 或其他 8051 专用头直接加入 ESP32 构建链。

当前仓库状态：

- 已有共享协议头 `gp_led_matrix_protocol.h`。
- 已有 ESP32 侧驱动骨架 `gp_led_matrix_esp32.h/.cc`。
- 已有 AI8051U 接口设计头 `gp_led_matrix_ai8051u.h` 与设计文档。
- 已有 MCP 调试圆点工具、桥接测试脚本和项目总览文档。

下一阶段主目标：

- 将小智 AI 的语音结果稳定映射为 LED 动作对象。
- 先通过本地 I2C 自定义协议把动作下发到 AI8051U，不依赖外部 MCP 桥接。
- 由 AI8051U 执行 WS2812 显示动作，实现语音控制 LED 显示闭环。
- 将原有测试动画整理为可调用预设，例如 `diamond`、`cross`、`python_demo`、`scroll_subtitle`，并允许语音指令直接引用这些预设名。

工作要求：
1. 优先复用 `Board`、`I2cDevice`、`Led` 三层抽象。
2. 当前矩阵内容应以显式图像更新为准，不应由待机、聆听、说话等设备状态自动覆盖。
3. 协议负载优先兼容 `GP_Port/test_image.h` 里的 RGB332 帧和 16 行字模格式。
4. 语音颜色/动画类需求优先对齐 `voice_color_result` 一类高层动作对象，避免直接耦合 8051 内部实现细节。
5. 语音显示默认优先使用纯色满屏；只有明确出现预设名时才切换到预设图案/字幕显示。
6. 预设图案一旦被指定，只显示该图案本身，不再沿用旧测试逻辑轮播其他图案。
7. 图案背景色必须通过独立背景色指令或显式字段更新；纯色显示命令不能隐式改写背景色。
8. 小智侧调试信息应能展示最近一次实际下发的图像内容摘要，而不仅是链路在线状态。
9. 语音大模型通过官方 MCP 端点调用本地工具，与设备触摸按键反向发起调用不是同一条链路；涉及 `Snap` 按键时，优先使用本地 HTTP 上传而不是继续依赖反向 MCP `tools/call`。
10. 每次小智端和 MCP 脚本修改后，自动终止旧 MCP 脚本，并通过 ESP-IDF 执行编译、下载、监视，然后重新运行 MCP 脚本。
11. 每次改动后优先检查构建错误，再补说明文档和命令示例。
12. 任何 AI8051U 侧代码都应按接口层或模板层组织，除非明确要求再扩展成完整固件工程。

输出要求：
1. 修改说明要指出新增的协议、ESP32 驱动、AI8051U 接口边界和验证状态。
2. 若有未完成项，要明确列出阻塞点和下一步建议。

## English
You are implementing an I2C-driven 16x16 LED matrix extension for xiaozhi-esp32. The target board is `lichuang-dev`, and the external controller is AI8051U. Keep all new files inside `GP_Port`, reuse the existing board/I2C/LED abstractions, and do not bring 8051-specific headers into the ESP32 build.

Current status:

- The shared protocol header, ESP32 driver skeleton, AI8051U interface header, MCP debug tooling, and bridge-validation assets already exist.
- The next milestone is a full path from XiaoZhi voice results to AI8051U-executed WS2812 display actions over a custom I2C protocol.