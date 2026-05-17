---
name: Claude Changes Sync
description: "Sync Claude-side structural changes to Copilot prompts and skills"
argument-hint: "Claude change summary (files added/removed/renamed, skill changes, CLAUDE.md updates)"
agent: agent
model: "GPT-5 (copilot)"
---
当 Claude 侧发生结构性变更时，同步到 Copilot 侧。

## 变更类型与对应操作

### 1. 新增/删除文件或目录
- 检查 `.github/prompts/` 中所有 prompt 文件的路径引用是否需要更新

### 2. Skill 内容变更
- Skills 统一在 `.claude/skills/`（EN with YAML frontmatter），Claude Code 和 Copilot 共享
- 若 skill 内容变更，只需更新 `.claude/skills/<name>.md`

### 3. CLAUDE.md 结构性变更
- 若入口文件、构建流程等变化，同步更新相关 README 和 project_structure.md

### 4. 项目结构变更
- 更新 `.github/prompts/README.md` 和 `Doc/Instructions/project_structure.md`

## 当前结构

| 项目 | 位置 |
|---|---|
| 主入口 | `CLAUDE.md` |
| Skills | `.claude/skills/*.md`（统一，共享） |
| Copilot Prompts | `.github/prompts/*.prompt.md`（YAML frontmatter） |
| 项目结构 | `Doc/Instructions/project_structure.md` |

## 输出格式

- `Claude 变更摘要`
- `需同步的文件列表`
- `各文件的具体修改内容`
