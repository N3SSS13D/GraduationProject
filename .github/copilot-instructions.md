# Project General Coding Guidelines

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
- Spacing around operators: add a space after each comma; unary operators (`!`, `*`, `&` as dereference, etc.) have no surrounding space; binary operators (`=`, `==`, `+`, `-`, `&`, `|`, etc.) have one space on each side; `->` and `.` have no surrounding space; add one space after keywords (`if`, `for`, `while`, etc.); no space between a function name and its argument list
- Avoid relying on operator precedence for complex expressions; use parentheses `()` to make intent explicit
- Write macro functions in normal statement format and end every continued line with a line-continuation backslash `\` (not a newline escape — this tells the preprocessor the macro continues on the next line)

## Naming Conventions
- Use ALL_CAPS with underscores for macros, constants, enum values, and goto labels; always add a module-name prefix (e.g., `GPIO_MAX_COUNT`)
- Use PascalCase for functions, enum type names, struct type names, and union type names (e.g., `GetLevel`); module-exported interface functions may prepend an ALL_CAPS module abbreviation separated by an underscore (e.g., `GPIO_GetLevel`)
- Use lowerCamelCase for global variables; optionally prefix with `g_` to improve searchability (e.g., `g_sensorValue`)
- Use lowerCamelCase for local variables; keep names brief — context should reveal meaning; variables with wider scope deserve more descriptive names
- Use lowerCamelCase for function parameters, macro parameters, struct members, and union members (e.g., `bufLen`)
- Use all-lowercase with underscores for file and folder names; follow the pattern `module_feature` (e.g., `gpio_driver.c`)
- Use anonymous types for typedef; self-referencing pointer types may add a `tag` prefix or a trailing underscore; never redefine basic numeric types via typedef
- Name macros after their actual purpose; minimize the use of function-like macros
- Use the standard project abbreviations (e.g., `addr`, `buf`, `len`, `src`, `dest`, `ret`, `cfg`, `err`) for consistency across the codebase

## Code Quality
- Define all local variables at the top of each function before any executable statements
- Keep functions at or under **80 effective lines**; extract independent sub-tasks into dedicated helper functions when this limit is exceeded
- Limit function parameters to **5 or fewer**; pass additional data via a dedicated struct
- Avoid highly complex, trick-dependent, or hard-to-read statements
- Design functions for high cohesion and low coupling; each function should have a single, clearly defined responsibility
- Never use magic numbers; replace all numeric literals with named constants or enum values
- Do not use `return` inside macro definitions
- Minimize the use of global variables; prefer local variables
- Avoid `extern`; declare all functions in header files and protect global-variable access through `Get`/`Set` functions; mark file-scope globals with `static`
- Use only ISO C standard characters in source files
- Initialize variables before their first use (initialization at point of use is acceptable); always initialize pointers and global variables at their definition
- Add a file-level comment block to every header file containing at minimum: filename, author, creation date, and version number
- Write all comments in Doxygen style using `/* */` block syntax; write comments in English; never nest comments; delete unused code rather than commenting it out; avoid inserting comments in the middle of a code line
