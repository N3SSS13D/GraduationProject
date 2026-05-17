---
name: WS2812 结构化代码审查（中文）
description: "按四类结构审查项目改动，聚焦 LED端、AI端、协议和脚本边界"
argument-hint: "审查目标（commit、文件、模块或功能描述）"
agent: agent
model: "GPT-5 (copilot)"
---
对指定目标执行代码审查。

行为准则详见：`.claude/skills/karpathy-guidelines.md`
分类布局和常用阅读组合详见：`Doc/Instructions/project_structure.md`

优先级：功能缺陷 → 时序风险 → 协议一致性 → 缓冲区边界 → 验证缺口

分类入口：
- `LED端`：`app.c` → `gp_led_action.c` → `gp_led_matrix_ai8051u.c` → `ws2812_drv.c`
- `AI端`：`board` → `ui/debug state` → `gp_led_matrix_esp32` → `transport`
- `协议`：`gp_led_matrix_protocol.h` + `*_spec.md` 字段对齐
- `脚本`：主机绘图应对接 AI端 预览/上传接口

## 输出格式

- `发现`
- `待确认问题 / 假设`
- `变更摘要`
- `测试与验证缺口`
