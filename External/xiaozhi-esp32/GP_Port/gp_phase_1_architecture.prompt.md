# Phase 1 Prompt: Architecture And Scope

## 中文
请梳理当前 `AI端 -> LED端` 蓝牙主线的文件边界和调用关系，并更新 `GP_Port` 目录下的架构说明文档。

重点路径：

- `main/application.cc`
- `main/boards/lichuang-dev/`
- `GP_Port/gp_led_matrix_esp32.h/.cc`
- `GP_Port/transport/`
- `GP_Port/ui/`

输出必须明确：

- `AI端` 动作对象从哪里生成。
- 蓝牙链路从哪里接入。
- 哪些文件属于传输、界面、驱动、板级接入。

## English
Review the current Bluetooth-focused `AI side -> LED side` architecture and update the GP_Port architecture notes with clear file boundaries and call flow.