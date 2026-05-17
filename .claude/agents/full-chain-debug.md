---
name: full-chain-debug
description: Full-chain debug agent. Use when an issue manifests at the LED display but root cause could be at any layer.
---

# Full-Chain Debug Agent

## Trigger
When an issue manifests at the LED display but the root cause could be at any layer (script → AI-side → protocol → LED-side).

## Workflow
1. LED-side logs check: `[GP_RX]`, `[GP_TX]`, `[GP_DROP]`, `[GP_SYNC]`, `[GP_CRC]`
2. AI-side transport logs check: `[BT_MON]` sniff window, `ReadReply()` return status
3. Protocol validation: compare sent vs received bytes, check CRC errors, verify sequence numbers
4. Script-side validation: verify JSON output format matches contract in `gp_matrix_pattern_protocol.md`
5. Isolate: one module at a time, start from closest to symptom
6. Verify fix at each layer before declaring complete

## Key Log Patterns
- LED-side errors: `[GP_SYNC]` (lost sync), `[GP_CRC]` (CRC mismatch), `[GP_DROP]` (buffer overflow)
- AI-side errors: `[BT_SEND]` (TX fail), `ReadReply timeout` (no ACK), `link_verified_=false`
- Script errors: MCP tool exception, WebSocket disconnect, format mismatch

## Verification
- After fix, run end-to-end test: voice/touch command → correct LED output
- Check `Doc/Instructions/end_to_end_data_flow.md` for reference trace
