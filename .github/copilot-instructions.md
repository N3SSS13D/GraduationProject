# Project General Coding Guidelines

## Project Structure

This repository uses four active categories so tasks can stay narrow and avoid loading unrelated files:

1. `LED-side display driver`
   - Paths: `Project/STC51/`, especially `Project/STC51/ws2812_driver/`
2. `AI-side interface orchestration`
   - Paths: `Project/xiaozhi-esp32/main/gp_port/` and required board files
3. `Bluetooth communication protocol`
   - Paths: `Project/Protocols/`
4. `Local drawing scripts`
   - Paths: `Project/Script/`

Active current-scheme docs live in:

- `Doc/Instructions/README.md`
- `Doc/Instructions/project_structure.md`
- `Doc/Instructions/problem_tracking.md`
- `Doc/Instructions/bt_version_hc05_uart2_architecture.md`
- `Project/STC51/README.md`
- `Project/xiaozhi-esp32/main/gp_port/README.md`
- `Project/Protocols/README.md`
- `Project/Script/README.md`

Use structure information to read only the narrow files required by the task. Do not scan unrelated categories by
default.
When a task falls into one category, read that category README first to get the module map, execution flow, and common
file bundle before opening deeper implementation files.

## Project Skills

- Apply `.github/skills/karpathy-guidelines/SKILL.md` when writing, reviewing, or refactoring code in this repository.
- Use the category-specific prompt or skill that matches the task:
  - `AI-side interface orchestration` -> `.github/prompts/ws2812-ai-control*.prompt.md` + `.github/skills/karpathy-guidelines/SKILL.md`
  - `LED-side display driver` -> `.github/prompts/ws2812-led-driver*.prompt.md` + `.github/skills/ws2812-led-driver/SKILL.md`
  - `Bluetooth communication protocol` -> `.github/prompts/ws2812-bluetooth-protocol*.prompt.md` + `.github/skills/bluetooth-protocol/SKILL.md`
  - `Local drawing scripts` -> `.github/prompts/ws2812-local-scripts*.prompt.md` + `.github/skills/local-drawing-scripts/SKILL.md`
- Concretely: state assumptions before coding, prefer the simplest change that solves the request, keep edits
  surgical, and define explicit verification criteria before implementation.
- For LED-side refresh, animation, or scheduler optimization tasks, use this workflow:
  1. summarize the current implementation and hot path first;
  2. list problems, risks, and candidate optimizations;
  3. choose the smallest feasible change and define validation criteria;
  4. implement one focused slice and validate it before widening scope;
  5. do a second-pass review after validation;
  6. sync docs, prompts, and skills when timing rules, workflow expectations, or architecture assumptions change.
- If a task can be solved with documentation or a small targeted change, do not add new abstractions or speculative
  configuration.

## Code Style

- Self-written code must follow these guidelines; external/third-party code retains its original style
- Order include files from least stable to most stable to reduce compile time
- Use 4-space indentation; follow K&R/BSD brace style consistently throughout the project
- Add a blank line between relatively independent code blocks (one blank line at a time)
- Use spaces for alignment; keep no more than 4 spaces between the longest macro name and its value
- Keep line width at or under 120 characters; break long lines at logical boundaries
- Write one statement per line — never place multiple statements on a single line
- Always use curly braces `{}` for conditional and loop statements, even for single-line bodies
- Place opening curly braces on the same line as the control statement (K&R style)
- Spacing around operators: add a space after each comma; unary operators (`!`, `*`, `&` as dereference, etc.) have
  no surrounding space; binary operators (`=`, `==`, `+`, `-`, `&`, `|`, etc.) have one space on each side; `->` and
  `.` have no surrounding space; add one space after keywords (`if`, `for`, `while`, etc.); no space between a
  function name and its argument list
- Avoid relying on operator precedence for complex expressions; use parentheses `()` to make intent explicit
- Write macro functions in normal statement format and end every continued line with a line-continuation backslash `\`
  (not a newline escape — this tells the preprocessor the macro continues on the next line)

## Naming Conventions

- Use ALL_CAPS with underscores for macros, constants, enum values, and goto labels; always add a module-name prefix
  (e.g., `GPIO_MAX_COUNT`)
- Use PascalCase for functions, enum type names, struct type names, and union type names (e.g., `GetLevel`);
  module-exported interface functions may prepend an ALL_CAPS module abbreviation separated by an underscore
  (e.g., `GPIO_GetLevel`)
- Use lowerCamelCase for global variables; optionally prefix with `g_` to improve searchability
  (e.g., `g_sensorValue`)
- Use lowerCamelCase for local variables; keep names brief — context should reveal meaning; variables with wider scope
  deserve more descriptive names
- Do not use `data` as a variable name, parameter name, or field name in 8051/STC-side C code; this toolchain may
  parse it as a storage-class keyword
- Use lowerCamelCase for function parameters, macro parameters, struct members, and union members
  (e.g., `bufLen`)
- Use all-lowercase with underscores for file and folder names; follow the pattern `module_feature`
  (e.g., `gpio_driver.c`)
- Use anonymous types for typedef; self-referencing pointer types may add a `tag` prefix or a trailing underscore;
  never redefine basic numeric types via typedef
- Name macros after their actual purpose; minimize the use of function-like macros
- Use the standard project abbreviations (e.g., `addr`, `buf`, `len`, `src`, `dest`, `ret`, `cfg`, `err`) for
  consistency across the codebase

## Code Quality

- Define all local variables at the top of each function before any executable statements
- Keep functions at or under **80 effective lines**; extract independent sub-tasks into dedicated helper functions
  when this limit is exceeded
- Limit function parameters to **5 or fewer**; pass additional data via a dedicated struct
- Avoid highly complex, trick-dependent, or hard-to-read statements
- Design functions for high cohesion and low coupling; each function should have a single, clearly defined
  responsibility
- Never use magic numbers; replace all numeric literals with named constants or enum values
- Do not use `return` inside macro definitions
- Minimize the use of global variables; prefer local variables
- Avoid `extern`; declare all functions in header files and protect global-variable access through `Get`/`Set`
  functions; mark file-scope globals with `static`
- Use only ISO C standard characters in source files
- Initialize variables before their first use (initialization at point of use is acceptable); always initialize
  pointers and global variables at their definition
- Add a file-level comment block to every header file containing at minimum: filename, author, creation date, and
  version number
- Write all comments in Doxygen style using `/* */` block syntax; write comments in English; never nest comments;
  delete unused code rather than commenting it out; avoid inserting comments in the middle of a code line
- After each code change, add appropriate explanatory comments for structure and behavior in modified code
  (follow existing comment style, keep comments concise, and avoid redundant comments)
- After each completed coding task, provide a Markdown-formatted summary report that includes: change overview,
  affected files, key logic updates, and verification status
- When adding custom functions, only use existing/valid APIs and ensure each custom function is declared in the
  corresponding header file before use
- After each optimization or behavior change, synchronously update the related prompt files under `.github/prompts/`,
  category skills under `.github/skills/`, and current docs under `Doc/Instructions/`, `Project/Protocols/`, or
  `Project/Script/` when their assumptions, workflow, or task boundaries are affected
- After each source-code modification for this repository, automatically run a Keil rebuild for
  `Project/STC51/ws2812_driver/ws2812_driver.uvproj` in the current VS Code workspace, analyze build errors, and
  continue fixing until the build succeeds; do not perform download/flash steps unless the user explicitly asks

## WS2812 Driver Documentation Sync

- When changing WS2812 scan/output timing behavior, update the current active docs in `Doc/Instructions/` and the
  related LED-side docs under `Project/STC51/` in the same task
- If scan mode logic changes, explicitly document: channel mapping rule, off-row waveform type, reset tail behavior,
  and interval safety constraints
