---
name: ai8051u-i2c-dma
description: 'Add or review AI8051U I2C DMA support. Use when enabling DMA_I2CT/DMA_I2CR, wiring AI8051U I2C slave RX/TX buffers to DMA, building hybrid interrupt-plus-DMA backends, or validating RXLOSS/TXOVW and reply-length behavior.'
argument-hint: 'Target file or module that needs AI8051U I2C DMA support'
user-invocable: true
disable-model-invocation: false
---

# AI8051U I2C DMA

## When to Use
- Add I2C DMA support to an AI8051U project.
- Convert an interrupt-only AI8051U I2C slave to a hybrid interrupt + DMA backend.
- Review whether `DMA_I2CT_*`, `DMA_I2CR_*`, and `DMA_I2C_*` are configured correctly.
- Debug `RXLOSS`, `TXOVW`, ACK error, or DMA length mismatch problems on AI8051U.
- Reuse the graduation-project pattern where command receive stays interrupt-driven and reply transmit uses TX DMA.

## Scope
This skill is for AI8051U firmware projects that already have a working I2C path and want to add DMA without rewriting the whole protocol stack.

## Recommended Strategy
1. Keep the existing I2C slave state machine if it is already stable.
2. Add DMA as a backend layer around existing `rxBuffer` and `txBuffer` instead of changing protocol parsing.
3. Prefer hybrid rollout first:
   - RX path stays interrupt-driven.
   - TX path moves to I2CT DMA.
4. Enable RX DMA only after hardware validation shows no visible LED brightness or scan-timing regression.

## Register Groups
- `DMA_I2CT_CFG/CR/STA/AMT/DONE/TXAH/TXAL`: I2C TX DMA configuration and status.
- `DMA_I2CR_CFG/CR/STA/AMT/DONE/RXAH/RXAL`: I2C RX DMA configuration and status.
- `DMA_I2C_CR/ST1/ST2/ITVH/ITVL`: shared DMA gate, amount/done select, ACK error, and transfer interval.

## Procedure
1. Inspect the existing I2C driver and find the byte-oriented RX/TX entry points.
2. Verify the project already has valid `ai8051u_def.h` helpers for:
   - `DMA_I2C_SetTxAddress`
   - `DMA_I2C_SetRxAddress`
   - `DMA_I2C_SetTxAmount`
   - `DMA_I2C_SetRxAmount`
   - `DMA_I2C_EnableTx`
   - `DMA_I2C_EnableRx`
   - `DMA_I2C_TriggerTx`
   - `DMA_I2C_TriggerRx`
   - `DMA_I2C_EnableDMA`
3. Add a DMA backend initializer that:
   - disables both directions,
   - clears DMA flags,
   - clears RX FIFO,
   - sets transfer interval and priorities,
   - leaves DMA idle until explicitly armed.
4. Add direction-specific runtime control, ideally by a function like `SetDmaMode(context, enableRx, enableTx)`.
5. For hybrid TX DMA:
   - keep packet build logic unchanged,
   - when the master starts a read, point TX DMA to `txBuffer`,
   - set amount to `txLength - 1`,
   - enable TX DMA and trigger it,
   - clear `txPending` when `DMA_I2CT_STA.I2CTIF` indicates completion.
6. For optional RX DMA:
   - point RX DMA at `rxBuffer`,
   - set amount to `rxBufferSize - 1`,
   - trigger RX on START,
   - on STOP or RX DMA completion, read `DMA_I2CR_DONE`,
   - normalize the packet if the address byte was captured into the buffer,
   - queue the completed packet into the existing parser path.
7. Keep DMA fault handling explicit:
   - clear `TXOVW` on TX overwrite,
   - clear `RXLOSS` on RX loss,
   - stop the affected DMA direction cleanly,
   - re-enable the matching I2C slave interrupt fallback path.
8. Add validation logs or counters for:
   - DMA TX done length,
   - DMA RX done length,
   - `RXLOSS`,
   - `TXOVW`,
   - ACK error,
   - final reply length.

## Validation Checklist
- TX DMA reply length matches the protocol packet length.
- Repeated-start read transactions do not leave TX DMA active after STOP.
- RX DMA, if enabled, does not duplicate or drop the address byte.
- No visible LED dimming or scan jitter appears when DMA is enabled.
- `RXLOSS` and `TXOVW` are both cleared and counted.
- Timeout and STOP still reset the I2C transfer state correctly.

## Reuse Reference
See [integration reference](./references/integration.md) for a concrete hybrid pattern and rollout guidance.
