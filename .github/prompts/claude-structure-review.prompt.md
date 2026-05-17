---
name: Claude Structure Review
description: "Automated project structure review and prompt synchronization after significant changes"
argument-hint: "Change context (branch name, change scope, affected categories)"
agent: agent
model: "GPT-5 (copilot)"
---
当完成结构性变更后，进行结构审查和文档同步。

## 审查流程

### 1. 变更影响分析
- 列出所有变更的文件（新增/修改/删除），按四分类归类
- 识别连锁影响范围

### 2. 文档一致性检查
逐项检查：
- [ ] `Doc/Instructions/project_structure.md` — 模块入口文件列表是否准确
- [ ] `Doc/Instructions/problem_tracking.md` — 当前约束是否涵盖新变更
- [ ] 各分类 `README.md` — 模块图和阅读组合是否最新
- [ ] `.github/prompts/` 中所有 prompt — 路径引用是否有效
- [ ] `.claude/skills/` 中所有 skill — 内容是否与实现一致
- [ ] `CLAUDE.md` — 入口和约束是否最新

### 3. 孤立引用扫描
- 搜索被删除或重命名的文件路径
- 修复所有断裂引用

### 4. 冗余检查
- 检查内容高度重复的文档对
- 检查未文档化的孤立脚本或工具

## 输出格式

- `变更清单（按分类）`
- `文档更新列表`
- `断裂引用列表`
- `冗余/孤立项列表`
