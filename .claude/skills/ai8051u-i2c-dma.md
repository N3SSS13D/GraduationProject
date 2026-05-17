---
name: ai8051u-i2c-dma
description: 'Add or review AI8051U I2C DMA support. Use when enabling DMA_I2CT/DMA_I2CR, wiring AI8051U I2C slave RX/TX buffers to DMA, building bidirectional DMA backends, or validating RXLOSS/TXOVW and reply-length behavior.'
argument-hint: 'Target file or module that needs AI8051U I2C DMA support'
user-invocable: true
disable-model-invocation: false
---

# AI8051U I2C DMA

## Category

`LED-side display driver`

## When to Use
- Add I2C DMA support to an AI8051U project.
- Convert an interrupt-only AI8051U I2C slave to a DMA-backed payload path.
- Review whether `DMA_I2CT_*`, `DMA_I2CR_*`, and `DMA_I2C_*` are configured correctly.
- Debug `RXLOSS`, `TXOVW`, ACK error, or DMA length mismatch problems on AI8051U.
- Reuse the graduation-project pattern where START/STOP framing stays in the ISR and payload bytes can move through RX/TX DMA.

## Scope
This skill is for AI8051U firmware projects that already have a working I2C path and want to add DMA without rewriting the whole protocol stack. The active branch defaults to the Bluetooth path. Use this skill only when the task explicitly targets the AI8051U I2C DMA backend.

## Recommended Strategy
1. Keep the existing I2C slave state machine if it is already stable.
2. Add DMA as a backend layer around existing `rxBuffer` and `txBuffer` instead of changing protocol parsing.
3. Prefer a framing-plus-DMA split:
   - START/STOP, timeout, and packet queueing stay in the I2C ISR.
   - Payload bytes move through I2CT/I2CR DMA.
4. Use `SetDmaMode(context, enableRx, enableTx)` to stage direction-by-direction rollout.

## Register Groups
- `DMA_I2CT_CFG/CR/STA/AMT/DONE/TXAH/TXAL`: I2C TX DMA configuration and status.
- `DMA_I2CR_CFG/CR/STA/AMT/DONE/RXAH/RXAL`: I2C RX DMA configuration and status.
- `DMA_I2C_CR/ST1/ST2/ITVH/ITVL`: shared DMA gate, amount/done select, ACK error, and transfer interval.

## Procedure
1. Inspect the existing I2C driver and find the byte-oriented RX/TX entry points.
2. Verify the project already has valid DMA helper macros: `DMA_I2C_SetTxAddress`, `DMA_I2C_SetRxAddress`, `DMA_I2C_SetTxAmount`, `DMA_I2C_SetRxAmount`, `DMA_I2C_EnableTx`, `DMA_I2C_EnableRx`, `DMA_I2C_TriggerTx`, `DMA_I2C_TriggerRx`, `DMA_I2C_EnableDMA`.
3. Add a DMA backend initializer: disable both directions, clear DMA flags, clear RX FIFO, set transfer interval and priorities, leave DMA idle until armed.
4. Add direction-specific runtime control via `SetDmaMode(context, enableRx, enableTx)`.
5. TX DMA: point to `txBuffer`, set amount to `txLength - 1`, enable and trigger. Clear `txPending` when `DMA_I2CT_STA.I2CTIF` indicates completion.
6. RX DMA: point to `rxBuffer`, set amount to `rxBufferSize - 1`, trigger on START. On STOP or completion, read `DMA_I2CR_DONE`, normalize packet, queue into parser.
7. Fault handling: clear `TXOVW` on TX overwrite, clear `RXLOSS` on RX loss, stop affected DMA direction cleanly, re-enable ISR fallback.
8. Add validation logs for DMA TX/RX done length, `RXLOSS`, `TXOVW`, ACK error, final reply length.

## Validation Checklist
- TX DMA reply length matches the protocol packet length.
- Repeated-start read transactions do not leave TX DMA active after STOP.
- RX DMA does not duplicate or drop the address byte.
- No visible LED dimming or scan jitter appears when DMA is enabled.
- `RXLOSS` and `TXOVW` are both cleared and counted.
- Timeout and STOP still reset the I2C transfer state correctly.

## Repository References
- `Project/STC51/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- `Project/STC51/ws2812_driver/Sources/inc/gp_led_matrix_ai8051u.h`
- `Project/STC51/ws2812_driver/Sources/app/app.c`

Active default config: RX DMA enabled, TX DMA enabled, protocol framing ISR retained, existing `rxBuffer`/`txBuffer` used for both directions.

## DMA Enable Sequence
1. Confirm original interrupt-only path still works.
2. Enable RX DMA and TX DMA for packet payload transfer.
3. Compare `DMA_I2C_ReadTxDone()` and `DMA_I2C_ReadRxDone()` against expected packet sizes.
4. Confirm address byte is not retained as a spurious first payload byte.
5. Compare packet length, checksum success rate, DMA fault counters against interrupt-only baseline.

## Reuse Reference
See [integration reference](.github/skills/ai8051u-i2c-dma/references/integration.md) for a concrete AI8051U DMA integration pattern.
