# LED Driver Technical Reference

Concise technical reference for `LED端显示驱动`. Derived from thesis architecture doc. See `Doc/Instructions/README.md` for navigation.

## File Map

```
Sources/
  app/app.c                  -- Init, main loop, task registration, Timer0/1 ISR hooks, preset config
  mid/gp_led_action.c        -- Remote action/frame/animation execution, online/offline toggle, Task10ms
  mid/draw_drv.c             -- Local render engine: solid, glyph, pattern, effect processing, DrawDrv_Task
  mid/offline_pattern.c      -- 6 stored offline patterns (16x16, 36B/layer in flash)
  mid/local_display_scheme.c -- Bootup carousel, button-driven local UI logic, Task10ms
  mid/rtc_clock.c            -- Software RTC + 7-seg digit LED display (3x5 font per digit)
  mid/key_ctrl.c             -- Button debounce (3-tick), short/long/combo detection, Task10ms
  mid/mid_task.c             -- 1ms cooperative scheduler: tick ISR + process main-loop, max 8 tasks
  drv/ws2812_drv.c           -- WS2812B PWM+DMA driver, dual-row interleaved scan
  drv/gp_led_matrix_ai8051u.c-- UART2 protocol: byte stream → packet assembly → CRC → dispatch
  drv/hc595_drv.c            -- 74HC595 row selector (3 cascaded, SPI-like protocol)
  drv/timer.c, pwm.c, uart.c -- AiCube HAL: Timer0/1, PWMA, UART2
  drv/exti.c, port.c         -- AiCube HAL: external interrupts, GPIO
  drv/usblib.c               -- AiCube USB: OUT callback, ISP bridge
  drv/gp_led_matrix_usb_debug.c -- USB/button debug: halt scan, white/black cycle, row test
```

## Key Functions

| Function | File | Role |
|---|---|---|
| `APP_Init()` | app.c | Init all subsystems: WS2812, draw, action, protocol, buttons, scheduler, Timer1 |
| `APP_TaskLoop()` | app.c | Main loop: poll UART2 protocol, render pending animation, run cooperative tasks |
| `APP_ApplyPresetMode()` | app.c | Apply one of 8 display presets with specific color/effect params |
| `APP_Timer1ApplyRefreshInterval()` | app.c | Compute Timer1 reload + prescaler for scan mode (normal pair vs legacy shift) |
| `TIMER1_ISR()` | app.c | Timer1 interrupt: call `WS2812DRV_RefreshStep()` |
| `GpLedAction_ApplyAction()` | gp_led_action.c | Dispatch parsed `GpMatrixActionPayload`: local control vs remote display profile |
| `GpLedAction_ApplyDisplayProfileCore()` | gp_led_action.c | Profile → `DrawDrv_RenderConfig`: content type, effect, direction, color |
| `GpLedAction_ApplyFrameRgb332()` | gp_led_action.c | Write 256B RGB332 frame directly to WS2812 image buffer |
| `GpLedAction_ApplyFrameBitmapLayered()` | gp_led_action.c | Parse multi-layer data (36B/layer), composite from layer 0 upward |
| `GpLedAction_CommitAnimation()` | gp_led_action.c | Validate all frames stored, mark active, render frame 0 |
| `GpLedAction_Tick1ms()` | gp_led_action.c | ISR-side: advance animation frame index (non-blocking) |
| `GpLedAction_RenderPendingAnimationFrame()` | gp_led_action.c | Main loop: actual render of next animation frame |
| `GpLedAction_ToggleModeOverride()` | gp_led_action.c | Manual override: toggle online/offline display control |
| `WS2812DRV_Init()` | ws2812_drv.c | Config PWMA (P1.0 CH1, P1.2 CH2), DMA channels, double buffer alloc |
| `WS2812DRV_EncodeAllRows()` | ws2812_drv.c | RGB pixels → PWM compare values via 256x8 LUT, per-row encoding |
| `WS2812DRV_RefreshStep()` | ws2812_drv.c | Timer1 ISR: build dual-row PWM DMA buffer, trigger HC595 row select + DMA start |
| `WS2812DRV_BeginFrameWrite()` | ws2812_drv.c | Lock back buffer for pixel writing |
| `WS2812DRV_SetPixelRgbFast()` | ws2812_drv.c | Set single pixel GRB value (no bounds check, fast path) |
| `WS2812DRV_EndFrameWrite()` | ws2812_drv.c | Unlock, mark buffer ready for encode + display |
| `GpLedMatrixAi8051u_Poll()` | gp_led_matrix_ai8051u.c | Drain UART2 ring buffer, feed protocol state machine |
| `KeyCtrl_ProcessTick()` | key_ctrl.c | Button state machine: 3-tick debounce, short(0.8s)/long/combo(2s) detect |
| `MidTask_Init()` | mid_task.c | Clear all 8 task slots |
| `MidTask_Register(period, hook)` | mid_task.c | Register a task with period (ms) and callback |
| `MidTask_Tick1ms()` | mid_task.c | ISR-side: decrement all tickCounts, increment pendingCount on expiry |
| `MidTask_Process()` | mid_task.c | Main-loop-side: execute each task's hook if pending |
| `APP_DrawFrameTaskProxy()` | app.c | Draw task callback: sync period → check bypass → `DrawDrv_Task()` |
| `APP_KeyTaskProxy()` | app.c | Key task callback: `GpLedAction_Task10ms()` + `KeyCtrl_Task10ms()` + `LocalDisplayScheme_Task10ms()` |
| `APP_OnSchedTickExpired()` | app.c | Timer0 hook: `MidTask_Tick1ms()` + `GpLedAction_Tick1ms()` + re-arm Timer0 |
| `APP_SyncLocalDrawTaskPeriod()` | app.c | Read render config frameIntervalMs, update draw task period via `MidTask_SetPeriod` |
| `GpLedAction_Task10ms()` | gp_led_action.c | Action state machine: profile application, takeover timeout, cache replay |
| `LocalDisplayScheme_Task10ms()` | local_display_scheme.c | Offline carousel orchestration, button UI logic, clock mode transitions |

## Scan Architecture

**Mode**: `SCAN_NORMAL_PAIR` (dual-row simultaneous)

```
Timer1 ~1ms → RefreshStep(step 0..7):
  Step 0: Row 0 (CH1/P1.0) + Row 1 (CH2/P1.2)
  Step 1: Row 2 (CH1) + Row 3 (CH2)
  ...
  Step 7: Row 14 (CH1) + Row 15 (CH2)
  Back to Step 0 → ~8ms/frame = ~120Hz row-pair rate
```

PWM frequency: 33.1776 MHz / 48 = ~691.2 kHz (~1.447us per WS2812 bit period).

## Image Data Flow

```
Remote Frame (UART2)
  → gp_led_matrix_ai8051u.c: parse + CRC validate
    → gp_led_action.c: ApplyFrameRgb332 / ApplyFrameBitmapLayered
      → ws2812_drv.c: image buffer (16×16×3B RGB GRB-ordered)
        → EncodeAllRows(): pixel GRB → PWM duty values via LUT
          → RefreshStep(): build dual-row DMA buffer
            → PWMAT-DMA → P1.0/P1.2 → WS2812 LEDs
```

## Bitmap-to-PWM Encoding Pipeline

The complete encoding chain transforms 24-bit-per-pixel GRB data into the WS2812 single-wire protocol waveform using PWM+DMA automatic waveform generation. This section traces the data through every buffer and transformation stage.

### Buffer Overview

| Buffer | Dimensions | Size | Qty | Total | Description |
|---|---|---|---|---|---|
| **Image Buffer** | 16 rows × 16 cols × 3 ch (GRB) | 768 B | ×2 (front/back) | **1536 B** | Pixel color storage. Back-buffer for writes, front for active display. |
| **Row PWM Buffer** | 16 rows × 434 PWM slots | 6944 B | ×2 (active/pending) | **13888 B** | Per-row PWM compare values after LUT encoding. |
| **Dual-Row DMA Buffer** | 1 single buffer (interleaved CH1+CH2) | 935 B | ×1 | **935 B** | Interleaved CH1/CH2 PWM stream built per RefreshStep. |
| **Bit Expand LUT** | 256 entries × 8 bytes | 2048 B | ×1 | **2048 B** | Maps 8-bit color value to 8 PWM duty values. |

**Total PWM-path buffer allocation: ~18.4 KB** (all in xdata, external RAM).

### Stage 1: Pixel Writing → Image Buffer

Entry points:
- **Local rendering**: `DrawDrv_RebuildFrame()` (draw_drv.c:826)
- **Remote frame**: `GpLedAction_ApplyFrameRgb332()` / `GpLedAction_ApplyFrameBitmapLayered()` (gp_led_action.c)

The `DrawDrv_RebuildFrame()` function performs a full-frame rebuild:

```
DrawDrv_RebuildFrame():
  WS2812DRV_BeginFrameWrite()               // Lock back-buffer, disable dirty checks
  for row 0..15:
    for col 0..15:
      // 1. Content lookup (pattern/glyph/solid/clock)
      packed = GetPatternPixel(index, row, col)  // or GetJluTextPixel / GetSolidPixel / RtcClock_GetPixel

      // 2. RGB332 decode → 8-bit R, G, B channels
      DrawDrv_DecodeRgb332(packed, &r, &g, &b)   // RRRGGGBB → R×8, G×8, B×8

      // 3. Color remapping (foreground/background color config)
      DrawDrv_ApplyColorConfig(isFg, &r, &g, &b)

      // 4. Effect pipeline (background pixels skip effects):
      //    - Gradient (position-based color mix)
      //    - Breath (sine-wave brightness modulation)
      //    - Fade In/Out (per-pixel brightness ramp)
      //    - Color Cycle (hue rotation)

      // 5. Global brightness
      DrawDrv_ApplyBrightness(&r, &g, &b)

      // 6. Write to back-buffer in GRB order
      WS2812DRV_SetPixelRgbFast(row, col, r, g, b)
        → g_ws2812ImageBuf[BACK][row][col][0] = g   // Green first (WS2812 GRB order)
        → g_ws2812ImageBuf[BACK][row][col][1] = r
        → g_ws2812ImageBuf[BACK][row][col][2] = b

  WS2812DRV_EndFrameWrite()                 // Mark image dirty, unlock
  WS2812DRV_EncodeAllRows()                 // Trigger encoding (Stage 2)
```

**Color channel order**: WS2812 expects GRB, not RGB. Pixel writes always store G→R→B.

**Image buffer structure** (ws2812_drv.c:21):
```c
static uint8_t xdata g_ws2812ImageBuf[2][16][16][3];
//  [buffer_idx: 0=front/active, 1=back/write] [row][col][channel: 0=G, 1=R, 2=B]
```

**Dirty tracking**: Individual pixel writes in slow mode compare old/new values and set `g_ws2812ImageDirty`. Fast mode (`BeginFrameWrite`) skips per-pixel comparison and marks dirty at `EndFrameWrite`.

### Stage 2: Pixel Encoding → Row PWM Buffer

Trigger: `WS2812DRV_EncodeAllRows()` is called after every frame write (draw_drv.c:926 or gp_led_action frame handlers).

```
WS2812DRV_EncodeAllRows():
  if not image_dirty → skip (no changes since last encode)
  if pwm_swap_pending → skip (previous encode not yet consumed by scanner)

  buildIdx = activeIdx XOR 1              // Select the non-active PWM buffer
  for row 0..15:
    WS2812DRV_EncodeRowToPwmBuffer(buildIdx, row)

  // Atomically swap (ISR-safe):
  DisableGlobalInt()
  pendingPwmBufIdx = buildIdx
  pwmSwapPending = 1                      // Signal to RefreshStep: new data ready
  imageDirty = 0
  EnableGlobalInt()
```

The core encoding function `WS2812DRV_EncodeRowToPwmBuffer()` (ws2812_drv.c:226):

```
WS2812DRV_EncodeRowToPwmBuffer(bufIdx, row):
  // --- Phase A: Reset prefix (48 slots) ---
  // Purpose: hold data lines low before row data to improve decoding stability
  pwmIdx = 48
  for i in 0..47:
    rowPwmBuf[bufIdx][row][i] = 0         // PWM compare = 0 (output low)

  // --- Phase B: Pixel data (16 cols × 3 ch × 8 bits = 384 slots) ---
  for col 0..15:
    for channel in [G, R, B]:             // WS2812 order: Green → Red → Blue
      byteVal = imageBuf[BACK][row][col][channel]  // Read 8-bit color value
      lutRow = bitExpandLut[byteVal]              // Look up 8 PWM duty values
      for bitIdx 0..7:
        rowPwmBuf[bufIdx][row][pwmIdx] = lutRow[bitIdx]
        pwmIdx++

  // --- Phase C: Trailing zeros (2 slots) ---
  // Purpose: ensure PWM output returns low after last data bit
  for remaining (pwmIdx..433):
    rowPwmBuf[bufIdx][row][pwmIdx] = 0
```

**Total per row: 48 + 384 + 2 = 434 PWM slots.**

**Bit Expand Lookup Table** (ws2812_drv.c:119, built once at init):

```c
static uint8_t xdata g_ws2812BitExpandLut[256][8];
// For each possible byte value (0..255), stores 8 PWM duty values.

WS2812DRV_InitBitExpandLut():
  for dat in 0..255:
    for bitIdx in 0..7:
      if (dat & (0x80 >> bitIdx)):       // MSB-first bit order
        lut[dat][bitIdx] = 36            // WS2812_PWM_DUTY_BIT1 (T1H ~0.70us)
      else:
        lut[dat][bitIdx] = 12            // WS2812_PWM_DUTY_BIT0 (T0H ~0.35us)
```

PWM waveform mapping (PWM clock = 33.1776 MHz / 48 = 691.2 kHz, period ≈ 1.447 µs):

| WS2812 Symbol | PWM Compare Value | High Time | Physical Meaning |
|---|---|---|---|
| 0-code (T0H) | 12 | 12/48 × 48clk ≈ 0.35 µs | Bit = 0 |
| 1-code (T1H) | 36 | 36/48 × 48clk ≈ 1.04 µs | Bit = 1 |
| Low (reset/off) | 0 | 0 µs | Data line low |

The PWM period is 48 clock cycles. The compare value (0..47) sets the duty cycle — number of cycles the output stays HIGH before going LOW. Values 12 (25%) and 36 (75%) produce the WS2812 0-code and 1-code pulse widths respectively.

**Key optimization**: The 256×8 LUT avoids computing each bit's PWM value at runtime. A single table lookup replaces 8 conditional branches per color byte. Total LUT size: 2048 bytes — a worthwhile trade for real-time encoding of 256 pixels × 24 bits = 6144 bits per frame.

**Row PWM buffer structure** (ws2812_drv.c:22):
```c
static uint8_t xdata g_ws2812RowPwmBuf[2][16][434];
//  [bufIdx: 0=active, 1=pending] [row] [pwmSlot: 0..433]
```

**PWM swap protocol**: The active buffer feeds the scanner (RefreshStep ISR). The pending buffer receives newly encoded data. The swap happens at the scan frame boundary (scanRowIdx == 0) in `WS2812DRV_RefreshStep()`. This ensures a complete frame is displayed consistently without tearing.

### Stage 3: Row PWM → Dual-Row DMA Buffer

Trigger: `WS2812DRV_RefreshStep()` (called from Timer1 ISR every ~1ms) calls `WS2812DRV_FillDualRowPwmBuffer()`.

```
WS2812DRV_FillDualRowPwmBuffer(dualBuf, bufIdx, rowA, rowB):
  // --- Phase A: Interleave CH1(rowA) and CH2(rowB) PWM slots ---
  outIdx = 0
  for idx in 0..433:
    dualBuf[outIdx] = rowPwmBuf[bufIdx][rowA][idx]   // CH1 (even row, P1.0)
    outIdx++
    dualBuf[outIdx] = rowPwmBuf[bufIdx][rowB][idx]   // CH2 (odd row, P1.2)
    outIdx++
  // Output: 868 bytes interleaved

  // --- Phase B: Reset tail (32 slot-pairs = 64 bytes) ---
  // Purpose: hold both lines low after data to latch the last pixel
  for tail in 0..31:
    dualBuf[outIdx] = 0   // CH1 low
    outIdx++
    dualBuf[outIdx] = 0   // CH2 low
    outIdx++
  // Output: +64 bytes = 932 bytes

  // --- Phase C: DMA guard pair (2 bytes) ---
  // Purpose: ensure DMA boundary robustness
  dualBuf[outIdx] = 0
  outIdx++
  dualBuf[outIdx] = 0
  outIdx++
  // Total output: 934 bytes

  return 934   // outIdx
```

The interleaving format is critical: PWMAT-DMA in CH1+CH2 burst mode reads consecutive bytes and alternates writing to CCR1 (P1.0) and CCR2 (P1.2). So the byte sequence [CH1_0, CH2_0, CH1_1, CH2_1, ...] produces two parallel PWM output streams on P1.0 and P1.2 simultaneously.

**Dual-row buffer allocation** (ws2812_drv.c:23-26):
```c
// raw allocation with alignment byte:
static uint8_t xdata g_ws2812DualRowPwmBufRaw[868 + 66 + 1];  // = 935 bytes
// aligned pointer (guaranteed even address):
static uint8_t xdata *g_ws2812DualRowPwmBuf;
// At init: if raw address is odd, pointer = raw+1 to ensure even alignment
```

**Even-address alignment**: PWMAT-DMA requires the source buffer to start on an even address (`addr & 1 == 0`). An odd address triggers `g_ws2812DmaOddAddrCount++` and aborts the transfer. The +1 byte in the raw buffer ensures an even-aligned 934-byte window always exists within the 935-byte allocation.

### Stage 4: DMA Transfer → PWM Output to LEDs

Trigger: `WS2812DRV_TriggerDualRowDma()` after filling the dual-row buffer.

```
WS2812DRV_TriggerDualRowDma(txBuf, num):
  // Validate: even address, even transfer count, minimum 2 bytes
  addr = (uint16_t)txBuf
  if addr & 1 → increment oddAddrCount, abort
  alignedNum = num & ~1                   // Force even (pair alignment)
  if alignedNum < 2 → abort

  // Configure PWMAT-DMA:
  // - DMA streams bytes from txBuf (source)
  // - PWM peripheral consumes them as CH1/CH2 compare values (destination)
  // - Uses burst mode: consecutive bytes → alternating CH1/CH2 CCR registers
  PWMA_DBA = 0x0D                         // Base address = CCR1H
  PWMA_DBL = 0x01                         // Burst length = 1 (CH1+CH2 pair per trigger)
  PWMA_DER = 0x01                         // Enable DMA request on CH1 underflow
  PWMA_DMACR = 0x14                       // DMA request time: 2 clock before update

  DMA_PWMAT_TXAH/TXAL = addr              // Source address
  DMA_PWMAT_AMTH/AMT = alignedNum - 1     // Transfer count

  dmaBusy = 1
  DMA_PWMAT_CFG = IE | IP | PTY          // Interrupt enable, high priority
  DMA_PWMAT_CR = ENPWMAT | TRIG           // Enable + trigger
```

**DMA operation**: The PWMAT-DMA channel automatically transfers `alignedNum` bytes from the source buffer (xdata) to the PWMA peripheral. Each byte goes to either CCR1 (CH1/P1.0) or CCR2 (CH2/P1.2) in alternating order, controlled by the PWM DMA burst logic. The PWM peripheral immediately applies each new compare value to the output waveform.

**Transfer example** (first 8 bytes for row pair 0+1):
```
byte[0]=PWM(R0_col0_G_bit7) → PWMA_CCR1 → P1.0 output
byte[1]=PWM(R1_col0_G_bit7) → PWMA_CCR2 → P1.2 output
byte[2]=PWM(R0_col0_G_bit6) → PWMA_CCR1 → P1.0 output  (PWM updates in < 1.447µs)
byte[3]=PWM(R1_col0_G_bit6) → PWMA_CCR2 → P1.2 output
...
Each byte is the PWM compare value for one WS2812 bit period.
```

**DMA completion**: `WS2812DRV_OnDmaIsr()` clears `dmaBusy` and increments `dmaDoneCount`. `WS2812DRV_WaitDmaDone()` polls `dmaBusy` with a timeout of 60000 loop iterations to avoid permanent stalls.

### Stage 5: Refresh Step (ISR Orchestration)

`WS2812DRV_RefreshStep()` is called from Timer1 ISR every ~1000µs. It orchestrates all stages for one row-pair:

```
WS2812DRV_RefreshStep():
  if dmaBusy → return                       // Previous DMA still active, skip this step

  // PWM buffer swap at frame boundary
  if scanRowIdx == 0 AND pwmSwapPending:
    activePwmBufIdx = pendingPwmBufIdx       // Atomically update (no ISR nesting on 8051)
    pwmSwapPending = 0

  // Determine row pair
  if NORMAL_PAIR mode:
    rowA = scanRowIdx                        // 0, 2, 4, ..., 14
    rowB = scanRowIdx + 1                    // 1, 3, 5, ..., 15
    txLen = BuildDualRowPwmBuffer(rowA, rowB)
    scanRowIdx += 2
  else (LEGACY_SHIFT):
    rowA = scanRowIdx                        // 0..15 sliding window
    rowB = scanRowIdx + 1 (wraps)
    txLen = BuildDualRowLegacyBuffer(rowA, rowB)
    scanRowIdx += 1

  if scanRowIdx >= 16 → scanRowIdx = 0       // Wrap at end of frame

  refreshCount++
  WS2812DRV_SelectRows(rowA, rowB)           // Update HC595 row selection
  lastScanRowA = rowA; lastScanRowB = rowB; lastScanTxLen = txLen
  WS2812DRV_TriggerDualRowDma(dualBuf, txLen) // Fire DMA (Stage 4)
```

**Row selection sequence** (`WS2812DRV_SelectRows`):
```
1. WS2812DRV_BlankOutputs()                // PWM both channels to 0, HC595 all-off
2. delay_us(1)                              // Line discharge settling
3. HC595_SelectRows(rowA, rowB)            // Set new row selection
4. delay_us(3)                              // Row switch settling before DMA starts
```

**Complete scan cycle** (NORMAL_PAIR mode):
```
Step 0: Row 0 (CH1/P1.0) + Row 1 (CH2/P1.2) → ~1000µs DMA + wait
Step 1: Row 2 (CH1/P1.0) + Row 3 (CH2/P1.2) → ~1000µs
...
Step 7: Row 14 (CH1/P1.0) + Row 15 (CH2/P1.2) → ~1000µs
Back to Step 0 → frame complete (~8ms = 125Hz row-pair rate, ~30fps visible)
```

### Encoding Pipeline Summary

```
            ┌─────────────────────────────────────────────────────────┐
            │                   STAGE 1: Pixel Writing                │
            │  ┌──────────┐    ┌───────────┐    ┌──────────────────┐  │
            │  │  Content  │ → │  Effect    │ → │  Image Buffer     │  │
            │  │  Lookup   │    │  Pipeline  │    │  2×[16][16][3]   │  │
            │  │ (pattern/ │    │ (gradient/ │    │  GRB order       │  │
            │  │  glyph/   │    │  breath/   │    │  768B each       │  │
            │  │  solid)   │    │  fade...)  │    │  xdata           │  │
            │  └──────────┘    └───────────┘    └──────┬───────────┘  │
            └──────────────────────────────────────────┼──────────────┘
                                                       │
            ┌──────────────────────────────────────────┼──────────────┐
            │                   STAGE 2: LUT Encoding  │              │
            │  ┌───────────────────────────────────────▼────────────┐  │
            │  │  WS2812DRV_EncodeAllRows()                         │  │
            │  │    for each row:                                   │  │
            │  │      for each pixel GRB byte:                      │  │
            │  │        byteVal → 256×8 LUT → 8 PWM duty values     │  │
            │  │    48B reset prefix + 384B data + 2B tail = 434B   │  │
            │  └───────────────────────┬────────────────────────────┘  │
            │                          │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  Row PWM Buffer  2×[16][434] = 13888B xdata       │  │
            │  │  [active] feeds scanner; [pending] receives new    │  │
            │  └───────────────────────┬────────────────────────────┘  │
            └──────────────────────────┼───────────────────────────────┘
                                       │
            ┌──────────────────────────┼───────────────────────────────┐
            │     STAGE 3: Interleave  │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  WS2812DRV_FillDualRowPwmBuffer()                  │  │
            │  │    434 slots × 2 rows interleaved = 868B           │  │
            │  │    + 32 reset tail pairs (64B)                     │  │
            │  │    + 1 DMA guard pair (2B)                         │  │
            │  │    = 934B total output                             │  │
            │  └───────────────────────┬────────────────────────────┘  │
            │                          │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  Dual-Row DMA Buffer  935B raw (934B usable)       │  │
            │  │  Even-aligned pointer within raw allocation        │  │
            │  │  CH1_CH2_CH1_CH2_..._CH1_CH2_00_00_..._00_00_00   │  │
            │  └───────────────────────┬────────────────────────────┘  │
            └──────────────────────────┼───────────────────────────────┘
                                       │
            ┌──────────────────────────┼───────────────────────────────┐
            │    STAGE 4+5: DMA + Scan │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  WS2812DRV_RefreshStep()  [Timer1 ISR, ~1000µs]   │  │
            │  │    1. Swap PWM buf at frame start (scanIdx=0)      │  │
            │  │    2. Build dual-row buf → SelectRows → TriggerDMA │  │
            │  │    3. PWMAT-DMA: 934B xdata → CCR1/CCR2 auto      │  │
            │  │    4. DMA ISR → dmaBusy=0, dmaDoneCount++         │  │
            │  └───────────────────────┬────────────────────────────┘  │
            │                          │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  Physical Output: P1.0(CH1) + P1.2(CH2)            │  │
            │  │  WS2812 0/1 codes → 16×16 LED matrix               │  │
            │  └────────────────────────────────────────────────────┘  │
            └──────────────────────────────────────────────────────────┘
```

### Encoding Timing Analysis

| Stage | Location | Timing Context | Duration |
|---|---|---|---|
| Pixel Writing (256 px) | `DrawDrv_RebuildFrame()` | Main loop (MidTask callback) | ~2-5 ms (depends on effects) |
| LUT Encoding (16 rows) | `WS2812DRV_EncodeAllRows()` | Main loop (after frame write) | ~1.5 ms (256×3×8 LUT lookups) |
| Dual-Row Interleave | `FillDualRowPwmBuffer()` | Timer1 ISR | ~200 µs (868 interleave loops) |
| DMA Transfer (934 B) | Hardware (PWMAT-DMA) | Background (after ISR trigger) | ~1.35 ms (934B / 691kHz) |
| **Total per frame** | | | **~5-8 ms (main loop) + ~1.5 ms/row-pair (ISR)** |

The main-loop encoding time (~5-8ms) determines the maximum sustainable frame rate. At the default 32ms draw task period, encoding uses ~15-25% of available CPU in the main loop. Remote frames bypass the draw engine and encode directly, reducing latency to just the LUT encoding + DMA transfer time (~3ms).

## On-Chip Storage

- Image buffer: 16 × 16 × 3 = 768 bytes × 2 (double buffer) = 1536 bytes
- Animation storage: 32 frames × 36 bytes = 1152 bytes
- Offline patterns: 6 patterns × variable layers × 36 bytes (in flash/const)
- PWM DMA buffer: ~934 bytes × 2 (CH1 + CH2, active + pending)

## Control Mode Decision

`gp_led_action.c` maintains `remote_override_active_`:
- **TRUE**: Remote content (action/frame/animation) owns display. Local offline rendering suppressed.
- **FALSE**: Local offline patterns, button-driven carousel, or RTC clock displayed.

Manual toggle via long-press button combo or `SetAction` with local control flag.

## Task Scheduling Architecture

### Hardware Timer Foundation

Two hardware timers provide the real-time basis for the system:

| Timer | Type | Interval | Purpose |
|---|---|---|---|
| **Timer0** | 16-bit auto-reload, 1T mode | ~500us (one-shot, self-rearming) | Scheduler tick generator |
| **Timer1** | 16-bit, prescaler-configurable | ~1000us (per row-pair) | WS2812 scan step trigger |

**Timer0** operates as a self-rearming one-shot:
1. `APP_OnSchedTickExpired()` is registered as the microsecond-level hook
2. On each fire, it calls `MidTask_Tick1ms()` + `GpLedAction_Tick1ms()`, then re-arms via `TIMER0_StartOneShotUs(500)`
3. A `g_timerTickMs` counter increments every 1ms (used by ISR for timing, not by scheduler directly)

**Timer1** drives the display scan:
1. ISR calls `WS2812DRV_RefreshStep()` for each row-pair step
2. Supports cycle-count splitting for very long intervals (>65535 timer ticks)
3. Prescaler auto-selected in debug mode, fixed in normal mode

### MidTask Cooperative Scheduler

A lightweight cooperative scheduler designed for bare-metal systems. Located in `Sources/mid/mid_task.c`.

**Design philosophy**: No priority, no preemption, minimal RAM. Tasks are polled in registration order by the main loop. Sufficient for systems with few tasks and modest real-time requirements.

#### Task Control Block

```c
typedef struct {
    uint8_t  pendingCount;   // Number of pending executions (saturates at 0xFF)
    uint16_t tickCount;      // Countdown counter (decremented each ms)
    uint16_t period;         // Task period in ms (constant after init or SetPeriod)
    MidTaskHook_t hook;      // Callback function pointer
} MidTaskComponent_t;
```

#### API

| Function | Description |
|---|---|
| `MidTask_Init()` | Clear all task slots |
| `MidTask_Register(periodMs, hook)` | Register a task (returns 1 on success) |
| `MidTask_RegisterWithId(periodMs, hook)` | Register, returns `taskId` (0..7) or `MIDTASK_INVALID_ID` (0xFF) |
| `MidTask_SetPeriod(taskId, periodMs)` | Change task period at runtime (resets tickCount and pendingCount) |
| `MidTask_Tick1ms()` | ISR-side: decrement tickCount for all tasks, increment pendingCount on expiry |
| `MidTask_Process()` | Main-loop-side: call each task's hook if pendingCount > 0 |

**Constraints**: Max 8 tasks (`MIDTASK_MAX_COUNT`), period must be >0, hook must be non-null.

#### Tick-and-Process Split

```
Timer0 ISR (every ~500us, real-time):
  MidTask_Tick1ms():
    for each task:
      if tickCount > 0:
        tickCount--
        if tickCount == 0:
          tickCount = period       // reset countdown
          if pendingCount < 0xFF:
            pendingCount++          // mark as pending

Main Loop (background, non-blocking):
  MidTask_Process():
    for each task:
      if pendingCount > 0:
        pendingCount--
        task.hook()                // execute the task callback
```

This split ensures the ISR only does lightweight counter updates, while the actual task work (which may involve heavy rendering) runs in the main loop context where it cannot block the scan ISR.

#### Spill Protection

If the main loop runs slower than a task's period, `pendingCount` accumulates. It saturates at 0xFF to prevent wrap-around. When the main loop catches up, it will drain the backlog one execution per call to `MidTask_Process()`. If the backlog grows too large (persistently >1), task executions are effectively dropped to maintain real-time safety.

### Registered Tasks

Two tasks are registered in `APP_Init()` (app.c):

#### Task 0: Key Task — `APP_KeyTaskProxy()` (10ms period)

Executes three sub-tasks in sequence inside one proxy callback:

1. **`GpLedAction_Task10ms()`** (`gp_led_action.c`)
   - Processes action state machine transitions
   - Handles display profile application completion
   - Manages remote content takeover timeout
   - Checks for cached bitmap replay requests

2. **`KeyCtrl_Task10ms()`** (`key_ctrl.c`)
   - Button debounce state machine (3-tick confirmation)
   - Detects: P32 short press (next pattern), P32 long press ≥0.8s (toggle text/clock)
   - Detects: P33 short press (next effect), P33 long press ≥0.8s (next color theme)
   - Detects: P32+P33 simultaneous press ≥2s (toggle USB debug mode)
   - Input: external interrupts INT0/INT1 set flags; task polls and debounces

3. **`LocalDisplayScheme_Task10ms()`** (`local_display_scheme.c`)
   - Manages offline startup carousel (auto-plays patterns at 2s intervals)
   - Orchestrates button-driven local UI: pattern cycling, effect switching, color themes
   - Coordinates clock display mode transitions
   - Manages "last AI bitmap" slot for recent remote frames

**Rationale for 10ms period**: Button debounce needs ~30ms confirmation (3 ticks × 10ms). 10ms is fast enough for responsive UI without wasting CPU on idle polling.

#### Task 1: Draw Frame Task — `APP_DrawFrameTaskProxy()` (default 32ms, dynamic)

Executes local image rendering and animation processing:

1. **`APP_SyncLocalDrawTaskPeriodFromDriver()`**
   - Reads current `DrawDrv_RenderConfig.frameIntervalMs`
   - Normalizes value via `DrawDrv_NormalizeFrameIntervalMs()`
   - If changed, calls `MidTask_SetPeriod()` to update the draw task period
   - Enables dynamic frame rate adjustment (e.g., slow animation → longer period)

2. **Check `GpLedAction_ShouldBypassDrawScheduler()`**
   - Returns true when remote content (action/frame/animation) owns the display
   - When bypassed, the local draw engine is skipped entirely — remote frames write directly to the image buffer
   - This is the mechanism that gives priority to remote control over local rendering

3. **`DrawDrv_Task()`** (`draw_drv.c`)
   - Executes one step of local rendering:
     - Advances effect timelines (gradient, breath, scroll, fade, color cycle, etc.)
     - Rebuilds the frame when content or effect state changes
     - Writes output to WS2812 image buffer via `WS2812DRV_BeginFrameWrite/SetPixelRgbFast/EndFrameWrite`
   - Different effects have different per-step behaviors:
     - **Solid/Pattern**: Render once, no per-step update needed
     - **Gradient**: Update gradient position each step
     - **Breath**: Update brightness sine-wave phase each step
     - **Scroll (left/right)**: Shift the glyph window by `scrollStep` pixels each step
     - **Fade In/Out**: Adjust per-pixel brightness each step
     - **Color Cycle**: Rotate hue index each step
     - **Row Reveal/Hide**: Change visible row count each step

**Rationale for 32ms default**: 32ms ≈ 31.25fps, matching visual smoothness requirements. The period is dynamically adjusted based on the active effect's `frameIntervalMs` to balance smoothness vs CPU usage.

### Main Loop Execution Order

`APP_TaskLoop()` executes continuously after init (app.c:711):

```
while (1):
  1. GpLedMatrixUsbDebug_Run()         // USB debug mode (blocking row test when active)
  2. GpLedMatrixAi8051u_Poll()         // Drain UART2 ring buffer, parse protocol packets
     ↓ Commands dispatched to gp_led_action handlers
  3. GpLedAction_RenderPendingAnimationFrame()  // Deferred render from ISR-ticked animation frame advance
     ↓ Converts animation frame index → actual image buffer write (heavy encoding)
  4. MidTask_Process()                 // Execute pending cooperative tasks
     ↓ Task 0 (10ms): KeyTask → actions + buttons + local scheme
     ↓ Task 1 (~32ms): DrawFrame → local rendering + animation
```

**Ordering rationale**:
- Protocol polling (step 2) runs before rendering to apply any newly-received remote frames immediately
- Animation rendering (step 3) runs before cooperative tasks so remote animation frames are displayed in the same loop iteration
- Cooperative tasks (step 4) run last — local rendering fills the remaining time and is skipped if remote content is active

### Timer ISR Execution Flow

```
Timer1 ISR (every ~1000us, highest priority):
  WS2812DRV_RefreshStep():
    - Build dual-row PWM DMA buffer (CH1=even row, CH2=odd row)
    - Configure HC595 row selection for current step
    - Start PWMAT-DMA transfer on both channels
    - Advance step index (0..7, wraps to 0)
  → Total ISR time: << 100us (just buffer setup + DMA trigger)

Timer0 ISR (every ~500us, via one-shot re-arming):
  g_timerTickMs++
  if one-shot counter expired:
    call APP_OnSchedTickExpired():
      MidTask_Tick1ms()              // Update all task deadlines (O(n), n≤8)
      GpLedAction_Tick1ms()           // Advance animation frame index (constant time)
      TIMER0_StartOneShotUs(500)      // Re-arm for next tick
  → Total ISR time: < 20us (pure counter updates)
```

Timer1 ISR has strict real-time constraints — it must complete before the next row-pair step. DMA does the heavy lifting of actual PWM signal generation, so the ISR only needs to set up the next transfer.

### Task Period Management

The draw frame task period is dynamically managed:

```
User/AI changes effect
  → DrawDrv_SetRenderConfig() updates frameIntervalMs
    → On next APP_DrawFrameTaskProxy() call:
      APP_SyncLocalDrawTaskPeriodFromDriver()
        → Reads current frameIntervalMs
        → Normalizes via DrawDrv_NormalizeFrameIntervalMs()
        → If changed: MidTask_SetPeriod(taskId, newPeriodMs)
          → Reset: tickCount = newPeriod, pendingCount = 0
          → Task now fires at the new cadence
```

This allows effects that need faster updates (e.g., scroll at 16ms) to run at a higher rate, while static displays (solid color) can drop to a much lower rate to save CPU.
