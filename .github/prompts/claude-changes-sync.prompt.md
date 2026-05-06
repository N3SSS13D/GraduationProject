---
name: Claude Changes Sync
description: "Sync Claude-side structural changes to Copilot prompts and skills"
argument-hint: "Claude change summary (files added/removed/renamed, skill changes, CLAUDE.md updates)"
agent: agent
model: "GPT-5 (copilot)"
---
当 Claude 侧发生结构性变更时，使用此 prompt 将变更内容同步到 Copilot 侧。

## 变更类型与对应操作

### 1. 新增/删除文件或目录
- 记录变更的文件路径和操作类型（新增/删除/重命名）
- 检查 `.github/prompts/` 中所有 prompt 文件的路径引用是否需要更新
- 检查 `.github/skills/` 中所有 SKILL.md 的路径引用是否需要更新

### 2. Skill 内容变更
- 若 `.claude/skills/` 中某 skill 内容发生实质性变更，同步更新 `.github/skills/<name>/SKILL.md`
- 注意：Claude skill 为中文、平铺 `.md` 格式；Copilot skill 为英文、子目录 `SKILL.md` + YAML frontmatter 格式
- 只同步技术内容（模块路径、工作流、约束条件），不同步格式差异

### 3. CLAUDE.md 结构性变更
- 若 CLAUDE.md 中的分类入口文件列表、构建流程、命名规范等发生变化，同步更新 `copilot-instructions.md` 或 `copilot-instructions-zh.md`
- 同样，只同步技术内容

### 4. 项目结构变更
- 若四分类目录结构或 README 入口点变化，更新 `.github/prompts/README.md` 和 `Doc/Instructions/project_structure.md`

## 不需要同步的内容
- `.claude/settings.json` 的本地配置变更
- Claude 专属的行为准则表述差异（两种格式本就不同）
- 纯语言/措辞优化

## 当前 Claude 侧与 Copilot 侧的已知差异

| 项目 | Claude 侧 | Copilot 侧 |
|---|---|---|
| 主入口 | `CLAUDE.md`（中文） | `.github/copilot-instructions.md`（EN）/ `-zh.md`（ZH） |
| Skills 目录 | `.claude/skills/*.md`（扁平、中文） | `.github/skills/<name>/SKILL.md`（子目录、EN） |
| Prompts 目录 | 无独立 prompts（规则内联在 CLAUDE.md） | `.github/prompts/*.prompt.md`（YAML frontmatter） |
| 项目结构 | `Doc/Instructions/project_structure.md` | 同样文件，共享 |

## 输出格式

- `Claude 变更摘要`
- `需同步的 Copilot 文件列表`
- `各文件的具体修改内容`
- `无需同步的内容（及原因）`
