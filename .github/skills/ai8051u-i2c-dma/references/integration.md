# AI8051U I2C DMA Integration Reference

## DMA Rollout Pattern
Use this rollout order when the project already has a stable interrupt-driven I2C slave implementation.

1. Keep START/STOP, timeout handling, and packet queueing in the existing I2C ISR.
2. Add DMA initialization during driver init, but keep both directions disabled by default.
3. Enable both RX DMA and TX DMA for packet payload movement once the framing ISR is stable.
4. Keep a per-direction switch so you can temporarily disable one side during bring-up.

## Why Keep ISR Framing
- The protocol parser and buffer layout usually already work.
- START/STOP and timeout boundaries are still easier to reason about in the I2C ISR.
- RX DMA often needs a small normalization step because some controllers may surface the address byte differently from the byte interrupt path.
- Once framing stays stable, enabling both DMA directions reduces byte-by-byte ISR load without forcing a protocol rewrite.

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
- `Project/STC51/Project/ws2812_driver/Sources/drv/gp_led_matrix_ai8051u.c`
- `Project/STC51/Project/ws2812_driver/Sources/inc/gp_led_matrix_ai8051u.h`
- `Project/STC51/Project/ws2812_driver/Sources/app/test.c`

The active default is:
- RX DMA enabled
- TX DMA enabled
- protocol framing ISR preserved
- both directions use the existing `rxBuffer` / `txBuffer`

## Suggested Bring-up Order
1. Confirm the original interrupt-only path still works.
2. Enable RX DMA and TX DMA for packet payload movement.
3. Check `DMA_I2C_ReadTxDone()` and `DMA_I2C_ReadRxDone()` against the expected packet sizes.
4. Confirm the address byte is not retained as a fake first payload byte.
5. Compare packet lengths, checksum success rate, and DMA fault counters against the interrupt-only baseline.
