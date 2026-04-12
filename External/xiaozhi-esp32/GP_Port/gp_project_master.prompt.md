# Project Master Prompt

## 中文
你正在为 xiaozhi-esp32 项目开发一个基于 I2C 的 16x16 LED 矩阵接口。目标板型固定为 `lichuang-dev`，外部协处理器为 AI8051U。所有新增文件必须放在 `GP_Port` 目录，允许编辑现有板级或构建文件，但禁止把 `AI8051U.H` 或其他 8051 专用头直接加入 ESP32 构建链。

工作要求：
1. 优先复用 `Board`、`I2cDevice`、`Led` 三层抽象。
2. 小智设备状态的矩阵显示优先通过 `Led::OnStateChanged()` 驱动。
3. 协议负载优先兼容 `GP_Port/test_image.h` 里的 RGB332 帧和 16 行字模格式。
4. 每次改动后优先检查构建错误，再补说明文档。
5. 任何 AI8051U 侧代码都应按接口层或模板层组织，除非明确要求再扩展成完整固件工程。

输出要求：
1. 修改说明要指出新增的协议、ESP32 驱动、AI8051U 接口边界和验证状态。
2. 若有未完成项，要明确列出阻塞点和下一步建议。

## English
You are implementing an I2C-driven 16x16 LED matrix extension for xiaozhi-esp32. The target board is `lichuang-dev`, and the external controller is AI8051U. Keep all new files inside `GP_Port`, reuse the existing board/I2C/LED abstractions, and do not bring 8051-specific headers into the ESP32 build.