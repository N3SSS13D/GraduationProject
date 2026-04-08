# PWM channel data disorder issue review

## Phenomenon
- During dual-channel PWM + DMA output, several specific channel groups occasionally showed swapped or disordered data.
- Typical symptom: CH1 payload appears on CH2 for part of one transfer.

## Root causes
- DMA pair boundary mismatch:
  - Dual-channel stream must be consumed as CH1/CH2 pairs.
  - If transmit byte count is odd or not pair-aligned, channel order can shift.
- DMA source address alignment instability:
  - If source buffer start address is not aligned as expected by hardware transfer mode, some groups may become unstable.
- Tail boundary sensitivity:
  - End-of-transfer timing jitter can occasionally affect the last pair boundary.
- Fault accumulation risk:
  - Without timeout/recovery, a rare DMA stall can carry bad state to later frames.

## Solutions applied
- Enforced even transmit length:
  - `alignedNum = num & ~1`, then program `AMTH/AMT` using `alignedNum - 1`.
- Enforced source buffer address alignment:
  - Allocate raw buffer with one extra byte and select aligned pointer at init.
- Added DMA tail guard pair:
  - Append one zero CH1/CH2 pair at the end of each dual-row payload.
- Added strict DMA start sequence:
  - Reset DMA state -> set destination/source -> set busy flag -> enable interrupt and trigger.
- Added timeout and recovery path:
  - Wait loop has upper bound; on timeout force reset DMA/PWMA DMA path and clear busy flag.
- Kept fixed channel mapping rule:
  - Always map even row to CH1 and odd row to CH2 before transmit.

## Practical guidance
- Keep all dual-channel transmit lengths pair-aligned.
- Keep DMA TX source pointer alignment deterministic.
- Use a small tail guard pair for better robustness.
- Include timeout + reset recovery in any polling wait.
- Keep row-to-channel mapping stable in one place only.

## Refactored module entry points
- `Sources/drv/ws2812_drv.c` now provides reusable APIs for:
  - Image buffer update (`ClearImage`, `SetPixelRgb`)
  - Row PWM encoding (`EncodeAllRows`)
  - Dual-row packet build (`BuildDualRowPwmBuffer`)
  - DMA trigger/wait/recovery (`TriggerDualRowDma`, `WaitDmaDone`, `OnDmaIsr`)
  - One-call pair send (`SendRowPair`)
