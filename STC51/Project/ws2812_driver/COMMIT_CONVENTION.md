# Git 提交信息规范

本项目采用 [Conventional Commits](https://www.conventionalcommits.org/) 规范来编写提交信息。

## 提交信息格式

```
<类型>(<范围>): <主题>

<详细描述>

<脚注>
```

## 类型（Type）

提交信息的类型必须是以下之一：

- **feat**: 新功能（feature）
- **fix**: 修复bug
- **docs**: 文档变更
- **style**: 代码格式（不影响代码运行的变动，如空格、格式化等）
- **refactor**: 重构（既不是新增功能，也不是修复bug）
- **perf**: 性能优化
- **test**: 增加测试
- **chore**: 构建过程或辅助工具的变动
- **build**: 构建系统或外部依赖的变更
- **ci**: CI配置文件和脚本的变更

## 范围（Scope）

可选，表示影响的范围。本项目建议使用以下范围：

- `driver`: 驱动层相关
- `app`: 应用层相关
- `hal`: 硬件抽象层相关
- `utils`: 工具函数相关
- `config`: 配置文件相关
- `usb`: USB相关

## 主题（Subject）

- 使用祈使句，首字母小写
- 结尾不加句号
- 不超过50个字符
- 简洁明了地描述本次提交

## 详细描述（Body）

可选，用于详细说明：

- 提交的动机
- 与之前行为的对比
- 解决的问题

## 脚注（Footer）

可选，用于：

- 关闭的issue：`Closes #123`
- 破坏性变更：`BREAKING CHANGE: <描述>`

## 示例

### 示例1：新功能
```
feat(driver): 添加WS2812驱动支持

实现了WS2812 LED灯带的驱动功能，支持RGB颜色控制
和亮度调节。

Closes #10
```

### 示例2：修复bug
```
fix(app): 修复LED颜色显示错误

修复了RGB颜色通道顺序错误的问题，现在颜色显示正确。

Fixes #15
```

### 示例3：文档更新
```
docs: 更新README文档

添加了项目编译说明和使用示例。
```

### 示例4：重构
```
refactor(hal): 重构GPIO初始化函数

将GPIO初始化逻辑提取为独立函数，提高代码可维护性。
```

### 示例5：性能优化
```
perf(driver): 优化WS2812数据传输速度

使用DMA方式传输数据，提升LED刷新率至30fps。
```

## 使用提交模板

项目已包含 `.gitmessage` 模板文件，可以通过以下方式配置：

```bash
# 配置git使用模板
git config commit.template .gitmessage
```

配置后，使用 `git commit` 时会自动加载模板。

## 快速提交示例

```bash
# 新功能
git commit -m "feat(driver): 添加WS2812驱动支持"

# 修复bug
git commit -m "fix(app): 修复LED颜色显示错误"

# 文档更新
git commit -m "docs: 更新README文档"
```

