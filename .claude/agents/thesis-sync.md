---
name: thesis-sync
description: Thesis documentation sync agent. Use when implementation details change and thesis docs need updating.
---

# Thesis Documentation Sync Agent

## Trigger
When implementation details change in any of the four modules, ensure thesis documentation stays aligned.

## Workflow
1. Identify which thesis docs are affected by the change
2. Update the corresponding thesis architecture doc in `Doc/毕业论文/`:
   - `led_side_display_driver_architecture.md`
   - `ai_side_interface_orchestration_architecture.md`
   - `bluetooth_communication_protocol_architecture.md`
   - `local_drawing_scripts_architecture.md`
3. Update `Doc/毕业论文/software_chapter_draft.md` if chapter-level changes
4. Update `Doc/毕业论文/thesis_draft.md` if structural changes
5. Sync the corresponding tech ref in `Doc/Instructions/`:
   - `led_driver_tech_ref.md`
   - `ai_interface_tech_ref.md`
   - `protocol_tech_ref.md`
   - `scripts_tech_ref.md`
6. Update `Doc/Instructions/problem_tracking.md` if constraints changed

## Key Files
- Thesis docs: `Doc/毕业论文/*.md`
- Tech refs: `Doc/Instructions/*_tech_ref.md`
- Problem tracking: `Doc/Instructions/problem_tracking.md`
