---
name: Claude Structure Review
description: "Automated project structure review and prompt synchronization after significant changes"
argument-hint: "Change context (branch name, change scope, affected categories)"
agent: agent
model: "GPT-5 (copilot)"
---
当完成一轮涉及多文件的结构性变更后，使用此 prompt 自动进行结构审查和文档同步。

## 审查流程

### 1. 变更影响分析
- 列出所有变更的文件（新增/修改/删除），按四分类归类
- 识别变更的连锁影响范围（哪些文档、prompt、skill 引用了被修改的路径）

### 2. 文档一致性检查
按以下清单逐项检查：
- [ ] `Doc/Instructions/project_structure.md` — 模块入口文件列表是否仍然准确
- [ ] `Doc/Instructions/problem_tracking.md` — 当前有效约束是否涵盖新变更
- [ ] 各分类 `README.md`（`Project/STC51/`, `Project/xiaozhi-esp32/main/gp_port/`, `Project/Protocols/`, `Project/Script/`）— 模块图和常用阅读组合是否最新
- [ ] `.github/prompts/` 中所有 prompt 文件 — 文件路径引用是否有效
- [ ] `.github/skills/` 中所有 SKILL.md — 文件路径引用是否有效
- [ ] `.claude/skills/` 中所有 skill — 内容是否与 `.github/skills/` 对应项保持技术一致
- [ ] `CLAUDE.md` — 入口文件、工作流和约束是否最新
- [ ] `copilot-instructions.md` / `copilot-instructions-zh.md` — 同上

### 3. 孤立引用扫描
- 在所有文档中搜索被删除或重命名的文件路径
- 搜索模式：`Sources/`, `Project/`, `Doc/`, `tools/`, `mcp/`
- 修复所有断裂引用

### 4. 冗余检查
- 检查是否存在内容高度重复的文档对
- 检查是否有未文档化的孤立脚本或工具
- 检查 `.github/` 与 `.claude/` 之间是否有技术内容漂移

## 输出格式

- `变更清单（按分类）`
- `文档更新列表（需修改的文件及具体内容）`
- `断裂引用列表（文件:行号 → 建议修复）`
- `冗余/孤立项列表（建议操作）`
- `同步状态（Claude ↔ Copilot 一致性）`
