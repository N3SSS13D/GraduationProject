# AI8051U I2C DMA Integration Reference

## Hybrid Rollout Pattern
Use this rollout order when the project already has a stable interrupt-driven I2C slave implementation.

1. Keep START/STOP, timeout handling, and packet queueing in the existing I2C ISR.
2. Add DMA initialization during driver init, but keep both directions disabled by default.
3. Turn on TX DMA first for reply packets.
4. Leave RX interrupt-driven until hardware validation is complete.
5. Only then consider enabling RX DMA behind a per-direction switch.

## Why Hybrid First
- The protocol parser and buffer layout usually already work.
- The highest-gain, lowest-risk DMA optimization is reply TX because the buffer is already assembled.
- RX DMA often needs extra normalization because some controllers may surface the address byte differently from the byte interrupt path.
- On LED-heavy systems, RX DMA can perturb timing or bus contention more noticeably than TX DMA.

## Minimum Driver Elements
A practical AI8051U I2C DMA driver usually needs:
- one function to configure the DMA registers into a known idle state,
- one function to enable or disable RX and TX directions independently,
- one function to arm TX DMA from `txBuffer`,
- one function to arm RX DMA into `rxBuffer`,
- one function to stop each direction cleanly,
- one function to finalize RX DMA length and normalize the captured buffer,
- one TX DMA ISR for done and overwrite handling,
- one RX DMA ISR for done and RXLOSS handling.

## Common Failure Modes
- `TXOVW` means software wrote or re-triggered TX DMA while the previous transfer had not finished.
- `RXLOSS` means RX FIFO or memory service could not keep up.
- Wrong `AMT` usually creates off-by-one length errors because the hardware uses `AMT + 1` as the actual byte count.
- Forgetting to re-enable the slave RX/TX interrupt fallback path after stopping DMA leaves the I2C block alive but silent.

## Graduation Project Mapping
In this repository, the reference implementation lives in:
- `STC51/Project/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- `STC51/Project/ws2812_driver/Sources/inc/gp_led_matrix_ai8051u.h`
- `STC51/Project/ws2812_driver/Sources/app/test.c`

The active default is:
- RX DMA disabled
- TX DMA enabled
- protocol RX state machine preserved
- reply TX moved to I2CT DMA

## Suggested Bring-up Order
1. Confirm the original interrupt-only path still works.
2. Enable only TX DMA and test read-reply transactions.
3. Check `DMA_I2C_ReadTxDone()` against the expected packet size.
4. Run long-duration LED refresh and verify there is no visible dimming.
5. Enable RX DMA only in a dedicated test build.
6. Compare packet lengths, checksum success rate, and brightness stability against the interrupt-only baseline.
