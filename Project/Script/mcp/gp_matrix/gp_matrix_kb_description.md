# GP Matrix Display 知识库说明
本知识库为小智AI提供 GP Matrix Display MCP 桥接服务的完整工具参考。知识库详细记录了5个MCP绘图工具的参数规范、调用示例、动画模板和常见错误修复方案。
**覆盖范围：** 16x16 LED点阵屏的图案绘制、文字显示、动画播放和自然语言渲染。
**核心工具：** `draw_frame`（位图提交）、`draw_python`（代码绘图）、`show_text`（文字显示）、`draw_animation`（多帧动画）、`render_prompt`（自然语言渲染）。
**图像格式：** 层叠位图（BITMAP_LAYERED），每层36字节（1字节头 + 32字节位图 + 3字节RGB888颜色），支持多色叠加。
当用户提出与LED矩阵显示相关的请求时，大模型应先查阅本知识库中的工具约定和参数约束，再通过MCP `tools/call` 接口调用对应工具。
