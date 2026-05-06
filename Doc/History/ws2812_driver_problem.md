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

## Shared-line weak-light issue (confirmed)

### Phenomenon
- When one row is enabled, another row sharing the same signal path can weakly light.

### Confirmed hardware fix
- Adding a bleeder/discharge resistor on the power side effectively removes residual-charge-induced weak lighting.
- This hardware fix is now considered the primary solution.

### Software coordination strategy
- Keep deterministic row update sequence:
  - full blank before row select update,
  - short delay,
  - then apply new row select.
- Avoid immediate full blank in DMA complete ISR to reduce unnecessary duty loss.

### Trade-off note
- Strong software blanking can suppress ghosting but may reduce brightness.
- Final recommendation: prioritize the hardware resistor fix, and use software blanking only as timing coordination.

## Missing reset-low tail after payload

### Phenomenon
- After row-scan payload transmission, some LEDs do not turn off immediately, showing residual glow/tailing.

### Root cause
1. DMA payload tail previously had only a very short zero guard.
2. WS2812 needs a continuous low period (typically >50us) to latch/reset reliably.

### Fix applied
1. Added a fixed reset-low tail at end of each dual-row DMA packet:
  - `WS2812DRV_RESET_TAIL_SLOTS = 64`
2. Appended CH0/CH2 zero pairs for the full tail window.
3. Kept one extra zero guard pair for DMA boundary robustness.

### Code locations
- `Sources/inc/ws2812_drv.h`: new reset-tail slot macro and tail buffer size expansion.
- `Sources/drv/ws2812_drv.c`: tail append logic in both normal and legacy dual-row packet builders.

## Legacy 700us flicker under real load

### Phenomenon
- Normal mode at 2ms is stable, but legacy mode at 700us can still flicker in practice.

### Root cause
1. Legacy per-step payload is longer (current-row data + off-row off-code + reset tail).
2. Timer interval margin is too small versus actual transfer time, causing jitter when refresh attempts hit DMA busy windows.

### Fix applied
1. Added minimum-safe interval clamp for legacy mode in `Sources/app/app.c`.
2. Clamp floor is computed dynamically from active columns and reset-tail length, plus a safety margin.
