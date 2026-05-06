# 本地绘图脚本

## 分类

`本地绘图脚本`

## 适用范围

`Project/Script/` 下的脚本端和 MCP 任务：

- `Project/Script/mcp/gp_matrix/`
- `Project/Script/tools/`
- `Project/Script/media_tools/`

## 文件定位

- MCP 绘图任务：`Project/Script/mcp/gp_matrix/`
- 构建/烧录/监视器工作流任务：`Project/Script/tools/`
- 仅在脚本负载契约依赖共享协议字段时读取 `Project/Protocols/`

## 模块快速图

- `Project/Script/mcp/gp_matrix/gp_display_mcp_bridge.py` — 规范的主机端绘图桥接，供 LLM 工具和本地预览/上传流程使用
- `Project/Script/mcp/gp_matrix/gp_matrix_drawing_mcp_usage.md` — 工具输入、允许格式、动画约束和故障避免的当前使用契约
- `Project/Script/mcp/gp_matrix/gp_mcp_endpoint_client.py` — MCP 桥接入口的兼容启动器
- `Project/Script/media_tools/led_image_converter_gui.py` — 16x16 素材和固件友好导出的手动图像/文本转换助手
- `Project/Script/tools/ws2812_auto_debug.py` — 统一 AI + LED 自动调试工作流自动化

## 常见主机流程

- LLM / 工具请求 → `gp_display_mcp_bridge.py`
- 桥接输出 → AI端预览/上传接口
- AI端板/编排器 → 蓝牙上传
- LED端渲染在下游；脚本不应假设直接 LED端封包注入

## 常用阅读组合

- `MCP 绘图 / 动画规则` → `gp_matrix_drawing_mcp_usage.md` + `gp_display_mcp_bridge.py` + `Project/Protocols/gp_matrix_pattern_protocol.md`
- `手动素材转换` → `Project/Script/media_tools/led_image_converter_gui.py`
- `构建 / 监视器自动化` → `Project/Script/tools/ws2812_auto_debug.py`

## 问题解决工作流

对于脚本、MCP 和工具任务：

1. 先总结当前脚本流程、边界和入口点
2. 陈述操作风险、故障案例和候选修复，再编辑
3. 优先选择保持主机流程有界的最小可行变更
4. 先验证一个聚焦的工作流切片，再扩大范围
5. 当操作预期或工作流规则变化时，同步文档和 prompt/skill 指导

## 要求

1. 保持 LLM 工具名和参数名自描述
2. 保持负载和动画文档与 `Project/Protocols/` 下的活跃协议文档一致
3. 不将脚本专属规则移到顶层 prompt；保持在对应脚本分类或文档中
4. 若脚本工作流变更影响当前使用指导，更新 `Project/Script/README.md` 和最近的脚本文档
5. 保持默认自动调试链严格顺序：
   - 仅 Keil 重新构建 `Project/STC51/ws2812_driver/ws2812_driver.uvproj`
   - 延迟 20s，然后打开 AI8051U 串口监视器（默认 `COM15`）
   - ESP-IDF `build flash monitor` for `Project/xiaozhi-esp32`
6. 执行前验证工具路径并暴露可配置覆盖：
   - `S:\Embedded\Keil`
   - `S:\Embedded\ESP\v5.4.3\esp-idf`
7. 当自动化入口变更时移除旧脚本引用，避免死文档和过时任务绑定
