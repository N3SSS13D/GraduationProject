# LED端显示驱动技术参考（中文版）

面向 `LED端显示驱动` 的简明技术参考。英文原版见 `Doc/Instructions/led_driver_tech_ref.md`。

## 文件映射

```
Sources/
  app/app.c                  -- 初始化、主循环、任务注册、Timer0/1 ISR钩子、预设配置
  mid/gp_led_action.c        -- 远程动作/帧/动画执行、在线/离线切换、Task10ms
  mid/draw_drv.c             -- 本地渲染引擎：纯色/字形/图案/效果处理、DrawDrv_Task
  mid/offline_pattern.c      -- 6幅离线图案（16x16, 36B/层, 存储于Flash）
  mid/local_display_scheme.c -- 启动轮播、按键驱动本地UI逻辑、Task10ms
  mid/rtc_clock.c            -- 软件RTC时钟 + 7段数码管LED显示（3x5字模）
  mid/key_ctrl.c             -- 按键消抖（3次确认）、短按/长按/组合键检测、Task10ms
  mid/mid_task.c             -- 1ms协作式调度器：ISR侧节拍 + 主循环侧执行，最多8任务
  drv/ws2812_drv.c           -- WS2812B PWM+DMA驱动，双行交织扫描
  drv/gp_led_matrix_ai8051u.c-- UART2协议：字节流→封包组装→CRC→命令分发
  drv/hc595_drv.c            -- 74HC595行选择器（3片级联，类似SPI协议）
  drv/timer.c, pwm.c, uart.c -- AiCube HAL：Timer0/1、PWMA、UART2
  drv/exti.c, port.c         -- AiCube HAL：外部中断、GPIO
  drv/usblib.c               -- AiCube USB：OUT回调、ISP桥接
  drv/gp_led_matrix_usb_debug.c -- USB/按键调试：停止扫描、白/黑循环、行测试
```

## 关键函数

| 函数 | 文件 | 功能 |
|---|---|---|
| `APP_Init()` | app.c | 初始化所有子系统：WS2812、绘制、动作、协议、按键、调度器、Timer1 |
| `APP_TaskLoop()` | app.c | 主循环：轮询UART2协议、渲染待处理动画帧、执行协作式任务 |
| `APP_ApplyPresetMode()` | app.c | 应用8种显示预设中的一种（含颜色/效果参数） |
| `APP_Timer1ApplyRefreshInterval()` | app.c | 计算Timer1重载值+预分频器（normal pair vs legacy shift） |
| `TIMER1_ISR()` | app.c | Timer1中断：调用 `WS2812DRV_RefreshStep()` |
| `GpLedAction_ApplyAction()` | gp_led_action.c | 分发 `GpMatrixActionPayload`：本地控制 vs 远程显示配置 |
| `GpLedAction_ApplyDisplayProfileCore()` | gp_led_action.c | 配置→`DrawDrv_RenderConfig`：内容类型、效果、方向、颜色 |
| `GpLedAction_ApplyFrameRgb332()` | gp_led_action.c | 将256B RGB332帧直接写入WS2812图像缓冲 |
| `GpLedAction_ApplyFrameBitmapLayered()` | gp_led_action.c | 解析多层数据（36B/层），从第0层向上叠加合成 |
| `GpLedAction_CommitAnimation()` | gp_led_action.c | 验证所有帧已存储，标记为活跃，渲染第0帧 |
| `GpLedAction_Tick1ms()` | gp_led_action.c | ISR侧：推进动画帧索引（非阻塞） |
| `GpLedAction_RenderPendingAnimationFrame()` | gp_led_action.c | 主循环侧：实际渲染下一动画帧（重量级编码） |
| `GpLedAction_ToggleModeOverride()` | gp_led_action.c | 手动切换：在线/离线显示控制 |
| `WS2812DRV_Init()` | ws2812_drv.c | 配置PWMA（P1.0 CH1, P1.2 CH2）、DMA通道、双缓冲分配 |
| `WS2812DRV_EncodeAllRows()` | ws2812_drv.c | 像素RGB→PWM比较值（通过256×8查找表），逐行编码 |
| `WS2812DRV_RefreshStep()` | ws2812_drv.c | Timer1 ISR：构建双行PWM DMA缓冲，触发HC595行选+DMA启动 |
| `WS2812DRV_BeginFrameWrite()` | ws2812_drv.c | 锁定后台缓冲供像素写入 |
| `WS2812DRV_SetPixelRgbFast()` | ws2812_drv.c | 设置单个像素GRB值（无边界检查，快速路径） |
| `WS2812DRV_EndFrameWrite()` | ws2812_drv.c | 解锁，标记缓冲就绪待编码+显示 |
| `GpLedMatrixAi8051u_Poll()` | gp_led_matrix_ai8051u.c | 排空UART2环形缓冲，驱动协议状态机 |
| `KeyCtrl_ProcessTick()` | key_ctrl.c | 按键状态机：3次消抖，短按(0.8s)/长按/组合键(2s)检测 |
| `MidTask_Init()` | mid_task.c | 清空所有8个任务槽 |
| `MidTask_Register(period, hook)` | mid_task.c | 注册任务（周期ms + 回调函数） |
| `MidTask_Tick1ms()` | mid_task.c | ISR侧：所有任务tickCount递减，到期时pendingCount递增 |
| `MidTask_Process()` | mid_task.c | 主循环侧：若pendingCount>0则执行任务回调 |
| `APP_DrawFrameTaskProxy()` | app.c | 绘制任务回调：同步周期→检查远程接管→`DrawDrv_Task()` |
| `APP_KeyTaskProxy()` | app.c | 按键任务回调：`GpLedAction_Task10ms()`+`KeyCtrl_Task10ms()`+`LocalDisplayScheme_Task10ms()` |
| `APP_OnSchedTickExpired()` | app.c | Timer0钩子：`MidTask_Tick1ms()`+`GpLedAction_Tick1ms()`+重新设置Timer0 |
| `APP_SyncLocalDrawTaskPeriod()` | app.c | 读取渲染配置frameIntervalMs，通过`MidTask_SetPeriod`更新绘制任务周期 |
| `GpLedAction_Task10ms()` | gp_led_action.c | 动作状态机：配置应用、接管超时、缓存位图重放 |
| `LocalDisplayScheme_Task10ms()` | local_display_scheme.c | 离线轮播编排、按键UI逻辑、时钟模式切换 |

## 扫描架构

**当前模式**: `SCAN_NORMAL_PAIR`（双行同时扫描）

```
Timer1 ~1ms → RefreshStep(step 0..7):
  Step 0: Row 0 (CH1/P1.0) + Row 1 (CH2/P1.2)
  Step 1: Row 2 (CH1) + Row 3 (CH2)
  ...
  Step 7: Row 14 (CH1) + Row 15 (CH2)
  回到 Step 0 → ~8ms/帧 = ~120Hz 行对刷新率
```

PWM频率：33.1776 MHz / 48 = ~691.2 kHz（每个WS2812位周期~1.447us）。

## 图像数据流

```
远程帧 (UART2)
  → gp_led_matrix_ai8051u.c: 解析 + CRC校验
    → gp_led_action.c: ApplyFrameRgb332 / ApplyFrameBitmapLayered
      → ws2812_drv.c: 图像缓冲 (16×16×3B RGB, GRB顺序)
        → EncodeAllRows(): 像素GRB → PWM占空比值（通过查找表）
          → RefreshStep(): 构建双行DMA缓冲
            → PWMAT-DMA → P1.0/P1.2 → WS2812 LEDs
```

## 片上存储

- 图像缓冲：16 × 16 × 3 = 768 字节 × 2（双缓冲）= 1536 字节
- 动画存储：32 帧 × 36 字节 = 1152 字节
- 离线图案：6 个图案 × 可变层数 × 36 字节（存储在Flash/const区）
- PWM DMA缓冲：~934 字节 × 2（CH1 + CH2，活跃 + 待切换）

## 控制模式决策

`gp_led_action.c` 维护 `remote_override_active_`：
- **TRUE**：远程内容（动作/帧/动画）接管显示，本地离线渲染被抑制
- **FALSE**：本地离线图案、按键驱动的轮播、或RTC时钟显示

可通过长按按键组合或 `SetAction`（携带本地控制标志）手动切换。

---

## 位图编码到PWM波形 — 完整管线

将每像素24位的GRB色彩数据转换为WS2812单线协议波形，完全通过PWM+DMA硬件自动波形生成实现。以下逐阶段追踪数据在每个缓冲区中的变换过程。

### 缓冲区总览

| 缓冲区 | 维度 | 大小 | 数量 | 总计 | 说明 |
|---|---|---|---|---|---|
| **图像缓冲** | 16行 × 16列 × 3通道(GRB) | 768 B | ×2 (前台/后台) | **1536 B** | 像素颜色存储。后台供写入，前台供显示。 |
| **行PWM缓冲** | 16行 × 434 PWM槽位 | 6944 B | ×2 (活跃/待切换) | **13888 B** | LUT编码后的每行PWM比较值。 |
| **双行DMA缓冲** | 1个 (CH1+CH2交织) | 935 B | ×1 | **935 B** | 每RefreshStep构建的交织CH1/CH2 PWM流。 |
| **位展开查找表** | 256项 × 8字节 | 2048 B | ×1 | **2048 B** | 将8位色值映射为8个PWM占空比值。 |

**PWM路径缓冲总计: ~18.4 KB**（全部在xdata外部RAM）。

### 阶段1: 像素写入 → 图像缓冲

入口点:
- **本地渲染**: `DrawDrv_RebuildFrame()` (draw_drv.c:826)
- **远程帧**: `GpLedAction_ApplyFrameRgb332()` / `GpLedAction_ApplyFrameBitmapLayered()` (gp_led_action.c)

`DrawDrv_RebuildFrame()` 执行全帧重建：

```
DrawDrv_RebuildFrame():
  WS2812DRV_BeginFrameWrite()               // 锁定后台缓冲，禁用脏检查
  for row 0..15:
    for col 0..15:
      // 1. 内容查找 (图案/字形/纯色/时钟)
      packed = GetPatternPixel(index, row, col)  // 或 GetJluTextPixel / GetSolidPixel / RtcClock_GetPixel

      // 2. RGB332解码 → 8位R, G, B通道
      DrawDrv_DecodeRgb332(packed, &r, &g, &b)   // RRRGGGBB → R×8, G×8, B×8

      // 3. 颜色重映射 (前景/背景颜色配置)
      DrawDrv_ApplyColorConfig(isFg, &r, &g, &b)

      // 4. 效果管线 (背景像素跳过效果):
      //    - 渐变 (基于位置的色彩混合)
      //    - 呼吸 (正弦波亮度调制)
      //    - 淡入/淡出 (逐像素亮度斜坡)
      //    - 颜色循环 (色相旋转)

      // 5. 全局亮度
      DrawDrv_ApplyBrightness(&r, &g, &b)

      // 6. 按GRB顺序写入后台缓冲
      WS2812DRV_SetPixelRgbFast(row, col, r, g, b)
        → g_ws2812ImageBuf[BACK][row][col][0] = g   // 绿先 (WS2812 GRB顺序)
        → g_ws2812ImageBuf[BACK][row][col][1] = r
        → g_ws2812ImageBuf[BACK][row][col][2] = b

  WS2812DRV_EndFrameWrite()                 // 标记图像脏，解锁
  WS2812DRV_EncodeAllRows()                 // 触发编码 (阶段2)
```

**色彩通道顺序**: WS2812 要求 GRB 而非 RGB。像素写入始终按 G→R→B 顺序存储。

**图像缓冲结构** (ws2812_drv.c:21):
```c
static uint8_t xdata g_ws2812ImageBuf[2][16][16][3];
//  [buffer_idx: 0=前台/活跃, 1=后台/写入] [行] [列] [通道: 0=G, 1=R, 2=B]
```

**脏标志追踪**: 慢速模式下逐像素比较新旧值并置位 `g_ws2812ImageDirty`。快速模式（`BeginFrameWrite`）跳过逐像素比较，在 `EndFrameWrite` 时统一标记脏。

### 阶段2: 像素编码 → 行PWM缓冲

触发: `WS2812DRV_EncodeAllRows()` 在每次帧写入后调用 (draw_drv.c:926 或 gp_led_action 的帧处理函数)。

```
WS2812DRV_EncodeAllRows():
  if not image_dirty → skip (自上次编码后无变化)
  if pwm_swap_pending → skip (上次编码尚未被扫描器消费)

  buildIdx = activeIdx XOR 1              // 选择非活跃PWM缓冲
  for row 0..15:
    WS2812DRV_EncodeRowToPwmBuffer(buildIdx, row)

  // 原子交换 (ISR安全):
  DisableGlobalInt()
  pendingPwmBufIdx = buildIdx
  pwmSwapPending = 1                      // 向RefreshStep发出信号: 新数据就绪
  imageDirty = 0
  EnableGlobalInt()
```

核心编码函数 `WS2812DRV_EncodeRowToPwmBuffer()` (ws2812_drv.c:226):

```
WS2812DRV_EncodeRowToPwmBuffer(bufIdx, row):
  // --- A阶段: 复位前缀 (48个槽位) ---
  // 目的: 行数据前拉低数据线, 提高解码稳定性
  pwmIdx = 48
  for i in 0..47:
    rowPwmBuf[bufIdx][row][i] = 0         // PWM比较值 = 0 (输出低电平)

  // --- B阶段: 像素数据 (16列 × 3通道 × 8位 = 384个槽位) ---
  for col 0..15:
    for channel in [G, R, B]:             // WS2812顺序: 绿 → 红 → 蓝
      byteVal = imageBuf[BACK][row][col][channel]  // 读取8位颜色值
      lutRow = bitExpandLut[byteVal]              // 查表获得8个PWM占空比值
      for bitIdx 0..7:
        rowPwmBuf[bufIdx][row][pwmIdx] = lutRow[bitIdx]
        pwmIdx++

  // --- C阶段: 尾部归零 (2个槽位) ---
  // 目的: 确保PWM输出在最后数据位后恢复低电平
  for remaining (pwmIdx..433):
    rowPwmBuf[bufIdx][row][pwmIdx] = 0
```

**每行总计: 48 + 384 + 2 = 434 个PWM槽位。**

**位展开查找表** (ws2812_drv.c:119, 初始化时构建一次):

```c
static uint8_t xdata g_ws2812BitExpandLut[256][8];
// 对每个可能的字节值 (0..255), 存储8个PWM占空比值。

WS2812DRV_InitBitExpandLut():
  for dat in 0..255:
    for bitIdx in 0..7:
      if (dat & (0x80 >> bitIdx)):       // MSB优先的位顺序
        lut[dat][bitIdx] = 36            // WS2812_PWM_DUTY_BIT1 (T1H ~0.70us)
      else:
        lut[dat][bitIdx] = 12            // WS2812_PWM_DUTY_BIT0 (T0H ~0.35us)
```

PWM波形映射 (PWM时钟 = 33.1776 MHz / 48 = 691.2 kHz, 周期 ≈ 1.447 µs):

| WS2812 符号 | PWM 比较值 | 高电平时间 | 物理含义 |
|---|---|---|---|
| 0码 (T0H) | 12 | 12/48 × 48clk ≈ 0.35 µs | 位 = 0 |
| 1码 (T1H) | 36 | 36/48 × 48clk ≈ 1.04 µs | 位 = 1 |
| 低电平 (复位/关闭) | 0 | 0 µs | 数据线低 |

PWM周期为48个时钟周期。比较值 (0..47) 设置占空比——输出保持高电平的时钟周期数。值12 (25%) 和36 (75%) 分别产生WS2812的0码和1码脉冲宽度。

**关键优化**: 256×8查找表避免运行时逐位计算PWM值。一次查表替代每个颜色字节的8次条件分支。LUT总大小: 2048字节——对于实时编码每帧256像素 × 24位 = 6144位而言，是值得的权衡。

**行PWM缓冲结构** (ws2812_drv.c:22):
```c
static uint8_t xdata g_ws2812RowPwmBuf[2][16][434];
//  [bufIdx: 0=活跃, 1=待切换] [行] [pwmSlot: 0..433]
```

**PWM交换协议**: 活跃缓冲供应扫描器 (RefreshStep ISR)。待切换缓冲接收新编码数据。交换在扫描帧边界 (scanRowIdx == 0) 由 `WS2812DRV_RefreshStep()` 执行。这确保整帧一致显示，无撕裂。

### 阶段3: 行PWM → 双行DMA缓冲

触发: `WS2812DRV_RefreshStep()` (Timer1 ISR每~1ms调用) 调用 `WS2812DRV_FillDualRowPwmBuffer()`。

```
WS2812DRV_FillDualRowPwmBuffer(dualBuf, bufIdx, rowA, rowB):
  // --- A阶段: 交织CH1(rowA)和CH2(rowB)的PWM槽位 ---
  outIdx = 0
  for idx in 0..433:
    dualBuf[outIdx] = rowPwmBuf[bufIdx][rowA][idx]   // CH1 (偶数行, P1.0)
    outIdx++
    dualBuf[outIdx] = rowPwmBuf[bufIdx][rowB][idx]   // CH2 (奇数行, P1.2)
    outIdx++
  // 输出: 868字节交织数据

  // --- B阶段: 复位尾部 (32对槽位 = 64字节) ---
  // 目的: 数据完成后拉低两线, 使最后一颗灯珠锁存
  for tail in 0..31:
    dualBuf[outIdx] = 0   // CH1低
    outIdx++
    dualBuf[outIdx] = 0   // CH2低
    outIdx++
  // 输出: +64字节 = 932字节

  // --- C阶段: DMA保护对 (2字节) ---
  // 目的: 保证DMA边界鲁棒性
  dualBuf[outIdx] = 0
  outIdx++
  dualBuf[outIdx] = 0
  outIdx++
  // 总输出: 934字节

  return 934   // outIdx
```

交织格式至关重要: PWMAT-DMA的CH1+CH2突发模式读取连续字节，交替写入CCR1 (P1.0) 和 CCR2 (P1.2)。所以字节序列 [CH1_0, CH2_0, CH1_1, CH2_1, ...] 同时在P1.0和P1.2上产生两路并行PWM输出流。

**双行缓冲分配** (ws2812_drv.c:23-26):
```c
// 原始分配 (含对齐字节):
static uint8_t xdata g_ws2812DualRowPwmBufRaw[868 + 66 + 1];  // = 935字节
// 对齐指针 (保证偶地址):
static uint8_t xdata *g_ws2812DualRowPwmBuf;
// 初始化时: 若原始地址为奇数, 指针 = raw+1 以确保偶地址对齐
```

**偶地址对齐**: PWMAT-DMA要求源缓冲起始地址为偶数 (`addr & 1 == 0`)。奇数地址触发 `g_ws2812DmaOddAddrCount++` 并中止传输。原始缓冲中的+1字节确保935字节分配内始终存在一个偶对齐的934字节窗口。

### 阶段4: DMA传输 → PWM输出到LED

触发: `WS2812DRV_TriggerDualRowDma()` 在填充双行缓冲之后。

```
WS2812DRV_TriggerDualRowDma(txBuf, num):
  // 验证: 偶地址, 偶数传输量, 最少2字节
  addr = (uint16_t)txBuf
  if addr & 1 → 递增 oddAddrCount, 中止
  alignedNum = num & ~1                   // 强制偶数 (保持CH1/CH2对对齐)
  if alignedNum < 2 → 中止

  // 配置PWMAT-DMA:
  // - DMA从txBuf (源) 流式读取字节
  // - PWM外设以交替CH1/CH2比较值的形式消费它们 (目的地)
  // - 使用突发模式: 连续字节 → 交替的CH1/CH2 CCR寄存器
  PWMA_DBA = 0x0D                         // 基址 = CCR1H
  PWMA_DBL = 0x01                         // 突发长度 = 1 (每次触发一个CH1+CH2对)
  PWMA_DER = 0x01                         // CH1欠载时使能DMA请求
  PWMA_DMACR = 0x14                       // DMA请求时机: 更新前2个时钟

  DMA_PWMAT_TXAH/TXAL = addr              // 源地址
  DMA_PWMAT_AMTH/AMT = alignedNum - 1     // 传输计数

  dmaBusy = 1
  DMA_PWMAT_CFG = IE | IP | PTY          // 中断使能, 高优先级
  DMA_PWMAT_CR = ENPWMAT | TRIG           // 使能 + 触发
```

**DMA操作**: PWMAT-DMA通道自动将 `alignedNum` 字节从源缓冲 (xdata) 传输到PWMA外设。每个字节按交替顺序到达CCR1 (CH1/P1.0) 或 CCR2 (CH2/P1.2)，由PWM DMA突发逻辑控制。PWM外设立即将每个新比较值应用到输出波形。

**传输示例** (行对0+1的前8字节):
```
byte[0]=PWM(R0_col0_G_bit7) → PWMA_CCR1 → P1.0 输出
byte[1]=PWM(R1_col0_G_bit7) → PWMA_CCR2 → P1.2 输出
byte[2]=PWM(R0_col0_G_bit6) → PWMA_CCR1 → P1.0 输出  (< 1.447µs内更新)
byte[3]=PWM(R1_col0_G_bit6) → PWMA_CCR2 → P1.2 输出
...
每个字节是一个WS2812位周期的PWM比较值。
```

**DMA完成**: `WS2812DRV_OnDmaIsr()` 清除 `dmaBusy` 并递增 `dmaDoneCount`。`WS2812DRV_WaitDmaDone()` 轮询 `dmaBusy`，超时值为60000次循环迭代，避免永久卡死。

### 阶段5: 刷新步进 (ISR编排)

`WS2812DRV_RefreshStep()` 由Timer1 ISR每~1000µs调用，编排行对的所有阶段:

```
WS2812DRV_RefreshStep():
  if dmaBusy → return                       // 上次DMA仍活跃, 跳过此步

  // 帧边界处的PWM缓冲交换
  if scanRowIdx == 0 AND pwmSwapPending:
    activePwmBufIdx = pendingPwmBufIdx       // 原子更新 (8051无ISR嵌套)
    pwmSwapPending = 0

  // 确定行对
  if NORMAL_PAIR 模式:
    rowA = scanRowIdx                        // 0, 2, 4, ..., 14
    rowB = scanRowIdx + 1                    // 1, 3, 5, ..., 15
    txLen = BuildDualRowPwmBuffer(rowA, rowB)
    scanRowIdx += 2
  else (LEGACY_SHIFT):
    rowA = scanRowIdx                        // 0..15 滑动窗
    rowB = scanRowIdx + 1 (回绕)
    txLen = BuildDualRowLegacyBuffer(rowA, rowB)
    scanRowIdx += 1

  if scanRowIdx >= 16 → scanRowIdx = 0       // 帧结束回绕

  refreshCount++
  WS2812DRV_SelectRows(rowA, rowB)           // 更新HC595行选
  lastScanRowA = rowA; lastScanRowB = rowB; lastScanTxLen = txLen
  WS2812DRV_TriggerDualRowDma(dualBuf, txLen) // 启动DMA (阶段4)
```

**行选序列** (`WS2812DRV_SelectRows`):
```
1. WS2812DRV_BlankOutputs()                // PWM双通道输出0, HC595全关
2. delay_us(1)                              // 线路放电稳定
3. HC595_SelectRows(rowA, rowB)            // 设置新行选
4. delay_us(3)                              // DMA启动前行选稳定
```

**完整扫描周期** (NORMAL_PAIR模式):
```
Step 0: Row 0 (CH1/P1.0) + Row 1 (CH2/P1.2) → ~1000µs DMA + 等待
Step 1: Row 2 (CH1/P1.0) + Row 3 (CH2/P1.2) → ~1000µs
...
Step 7: Row 14 (CH1/P1.0) + Row 15 (CH2/P1.2) → ~1000µs
回到Step 0 → 帧完成 (~8ms = 125Hz行对速率, ~30fps可见帧率)
```

### 编码管线总览图

```
            ┌─────────────────────────────────────────────────────────┐
            │                   阶段1: 像素写入                        │
            │  ┌──────────┐    ┌───────────┐    ┌──────────────────┐  │
            │  │  内容    │ → │  效果     │ → │  图像缓冲         │  │
            │  │  查找    │    │  管线     │    │  2×[16][16][3]   │  │
            │  │ (图案/   │    │ (渐变/    │    │  GRB顺序          │  │
            │  │  字形/   │    │  呼吸/    │    │  每份768B        │  │
            │  │  纯色)   │    │  淡入...) │    │  xdata            │  │
            │  └──────────┘    └───────────┘    └──────┬───────────┘  │
            └──────────────────────────────────────────┼──────────────┘
                                                       │
            ┌──────────────────────────────────────────┼──────────────┐
            │                   阶段2: LUT编码          │              │
            │  ┌───────────────────────────────────────▼────────────┐  │
            │  │  WS2812DRV_EncodeAllRows()                         │  │
            │  │    对每行:                                         │  │
            │  │      对每像素GRB字节:                               │  │
            │  │        byteVal → 256×8 LUT → 8个PWM占空比值       │  │
            │  │    48B复位前缀 + 384B数据 + 2B尾部 = 434B          │  │
            │  └───────────────────────┬────────────────────────────┘  │
            │                          │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  行PWM缓冲  2×[16][434] = 13888B xdata             │  │
            │  │  [活跃] 供应扫描器; [待切换] 接收新数据              │  │
            │  └───────────────────────┬────────────────────────────┘  │
            └──────────────────────────┼───────────────────────────────┘
                                       │
            ┌──────────────────────────┼───────────────────────────────┐
            │     阶段3: 交织          │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  WS2812DRV_FillDualRowPwmBuffer()                  │  │
            │  │    434槽位 × 2行交织 = 868B                        │  │
            │  │    + 32复位尾对 (64B)                               │  │
            │  │    + 1 DMA保护对 (2B)                               │  │
            │  │    = 934B 总输出                                    │  │
            │  └───────────────────────┬────────────────────────────┘  │
            │                          │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  双行DMA缓冲  935B原始 (934B可用)                  │  │
            │  │  原始分配内偶对齐指针                               │  │
            │  │  CH1_CH2_CH1_CH2_..._CH1_CH2_00_00_..._00_00_00  │  │
            │  └───────────────────────┬────────────────────────────┘  │
            └──────────────────────────┼───────────────────────────────┘
                                       │
            ┌──────────────────────────┼───────────────────────────────┐
            │   阶段4+5: DMA + 扫描    │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  WS2812DRV_RefreshStep()  [Timer1 ISR, ~1000µs]   │  │
            │  │    1. 帧起始处交换PWM缓冲 (scanIdx=0)              │  │
            │  │    2. 构建双行缓冲 → 行选 → 触发DMA               │  │
            │  │    3. PWMAT-DMA: 934B xdata → CCR1/CCR2 自动     │  │
            │  │    4. DMA ISR → dmaBusy=0, dmaDoneCount++         │  │
            │  └───────────────────────┬────────────────────────────┘  │
            │                          │                               │
            │  ┌───────────────────────▼────────────────────────────┐  │
            │  │  物理输出: P1.0(CH1) + P1.2(CH2)                   │  │
            │  │  WS2812 0/1码 → 16×16 LED矩阵                     │  │
            │  └────────────────────────────────────────────────────┘  │
            └──────────────────────────────────────────────────────────┘
```

### 编码时序分析

| 阶段 | 位置 | 时序上下文 | 耗时 |
|---|---|---|---|
| 像素写入 (256 px) | `DrawDrv_RebuildFrame()` | 主循环 (MidTask回调) | ~2-5 ms (取决于效果) |
| LUT编码 (16行) | `WS2812DRV_EncodeAllRows()` | 主循环 (帧写入后) | ~1.5 ms (256×3×8次LUT查找) |
| 双行交织 | `FillDualRowPwmBuffer()` | Timer1 ISR | ~200 µs (868次交织循环) |
| DMA传输 (934 B) | 硬件 (PWMAT-DMA) | 后台 (ISR触发后) | ~1.35 ms (934B / 691kHz) |
| **每帧总计** | | | **~5-8 ms (主循环) + ~1.5 ms/行对 (ISR)** |

主循环编码时间 (~5-8ms) 决定了最大可持续帧率。在默认32ms绘制任务周期下，编码仅消耗主循环约15-25%的CPU时间。远程帧绕过绘制引擎直接编码，延迟降为仅LUT编码 + DMA传输时间 (~3ms)。

---

## 任务调度架构

### 硬件定时器基础

系统使用两个硬件定时器提供实时基础：

| 定时器 | 类型 | 间隔 | 用途 |
|---|---|---|---|
| **Timer0** | 16位自动重载，1T模式 | ~500us（单次触发，自复位） | 调度节拍发生器 |
| **Timer1** | 16位，预分频器可配置 | ~1000us（每行对） | WS2812 扫描步进触发 |

**Timer0** 以自复位单次触发模式运行：
1. `APP_OnSchedTickExpired()` 注册为微秒级钩子函数
2. 每次触发时，调用 `MidTask_Tick1ms()` + `GpLedAction_Tick1ms()`，然后通过 `TIMER0_StartOneShotUs(500)` 重新设置
3. `g_timerTickMs` 计数器每1ms递增（供ISR计时使用，调度器不直接使用）

**Timer1** 驱动显示扫描：
1. ISR调用 `WS2812DRV_RefreshStep()` 执行每行对步进
2. 支持周期数分割以处理超长间隔（>65535个定时器节拍）
3. 调试模式下自动选择预分频器，正常模式固定

### MidTask 协作式调度器

轻量级协作式调度器，专为裸机系统设计。位于 `Sources/mid/mid_task.c`。

**设计理念**：无优先级、无抢占、极小RAM开销。任务由主循环按注册顺序轮询执行。适用于任务数量少、实时性要求适中的系统。

#### 任务控制块

```c
typedef struct {
    uint8_t  pendingCount;   // 待执行次数（饱和上限0xFF）
    uint16_t tickCount;      // 倒计时计数器（每ms递减）
    uint16_t period;         // 任务周期（ms，初始化后保持不变，SetPeriod可修改）
    MidTaskHook_t hook;      // 回调函数指针
} MidTaskComponent_t;
```

#### API

| 函数 | 说明 |
|---|---|
| `MidTask_Init()` | 清空所有任务槽 |
| `MidTask_Register(periodMs, hook)` | 注册任务（成功返回1） |
| `MidTask_RegisterWithId(periodMs, hook)` | 注册，返回 `taskId`（0..7）或 `MIDTASK_INVALID_ID`（0xFF） |
| `MidTask_SetPeriod(taskId, periodMs)` | 运行时修改任务周期（重置tickCount和pendingCount） |
| `MidTask_Tick1ms()` | ISR侧：所有任务tickCount递减，到期时pendingCount递增 |
| `MidTask_Process()` | 主循环侧：若pendingCount>0则调用任务回调 |

**约束**：最多8个任务（`MIDTASK_MAX_COUNT`），周期必须>0，回调必须非空。

#### 节拍与执行的分离机制

```
Timer0 ISR (每 ~500us，实时上下文):
  MidTask_Tick1ms():
    遍历所有任务:
      若 tickCount > 0:
        tickCount--
        若 tickCount == 0:
          tickCount = period       // 重置倒计时
          若 pendingCount < 0xFF:
            pendingCount++          // 标记为待执行

主循环 (后台，非阻塞):
  MidTask_Process():
    遍历所有任务:
      若 pendingCount > 0:
        pendingCount--
        task.hook()                // 执行任务回调
```

这种分离确保ISR只做轻量级的计数器更新，而实际的任务工作（可能涉及重量级渲染）在主循环上下文中运行，不会阻塞扫描ISR。

#### 溢出保护

若主循环运行速度慢于任务周期，`pendingCount` 会累积。它饱和于 0xFF 以防止回绕。当主循环赶上时，每次调用 `MidTask_Process()` 执行一次并清掉一个待执行计数。如果积压持续过大（持续 >1），任务执行实际上会被丢弃以保持实时安全性。

### 已注册任务

`APP_Init()` (app.c) 中注册了两个任务：

#### 任务0：按键任务 — `APP_KeyTaskProxy()`（10ms周期）

在一个代理回调中依次执行三个子任务：

1. **`GpLedAction_Task10ms()`** (`gp_led_action.c`)
   - 处理动作状态机转换
   - 处理显示配置应用完成
   - 管理远程内容接管超时
   - 检查缓存位图重放请求

2. **`KeyCtrl_Task10ms()`** (`key_ctrl.c`)
   - 按键消抖状态机（3次节拍确认）
   - 检测：P32短按（下一图案）、P32长按≥0.8s（文字/时钟切换）
   - 检测：P33短按（下一效果）、P33长按≥0.8s（下一颜色主题）
   - 检测：P32+P33同时按≥2s（切换USB调试模式）
   - 输入来源：外部中断INT0/INT1置标志位；任务轮询并消抖

3. **`LocalDisplayScheme_Task10ms()`** (`local_display_scheme.c`)
   - 管理离线启动轮播（以2s间隔自动播放图案）
   - 编排按键驱动的本地UI：图案循环、效果切换、颜色主题
   - 协调时钟显示模式切换
   - 管理"最近AI位图"槽位（用于最近的远程帧）

**10ms周期的理由**：按键消抖需要~30ms确认（3次 × 10ms）。10ms对于响应式UI足够快，同时不会在空闲轮询上浪费CPU。

#### 任务1：绘制帧任务 — `APP_DrawFrameTaskProxy()`（默认32ms，动态可调）

执行本地图像渲染和动画处理：

1. **`APP_SyncLocalDrawTaskPeriodFromDriver()`**
   - 读取当前 `DrawDrv_RenderConfig.frameIntervalMs`
   - 通过 `DrawDrv_NormalizeFrameIntervalMs()` 进行规范化
   - 若发生变化，调用 `MidTask_SetPeriod()` 更新绘制任务周期
   - 实现动态帧率调整（例如：慢动画→更长的周期）

2. **检查 `GpLedAction_ShouldBypassDrawScheduler()`**
   - 当远程内容（动作/帧/动画）接管显示时返回true
   - 当被旁路时，本地绘制引擎完全跳过——远程帧直接写入图像缓冲
   - 这是远程控制优先于本地渲染的机制

3. **`DrawDrv_Task()`** (`draw_drv.c`)
   - 执行一步本地渲染：
     - 推进效果时间线（渐变、呼吸、滚动、淡入淡出、颜色循环等）
     - 当内容或效果状态改变时重建帧
     - 通过 `WS2812DRV_BeginFrameWrite/SetPixelRgbFast/EndFrameWrite` 将输出写入WS2812图像缓冲
   - 不同效果有不同的每步行为：
     - **纯色/图案**：渲染一次，无需每步更新
     - **渐变**：每步更新渐变位置
     - **呼吸**：每步更新亮度正弦波相位
     - **滚动（左/右）**：每步将字形窗口移动 `scrollStep` 像素
     - **淡入/淡出**：每步调整每像素亮度
     - **颜色循环**：每步旋转色相索引
     - **行显/行隐**：每步改变可见行数

**32ms默认周期的理由**：32ms ≈ 31.25fps，匹配视觉流畅度要求。周期根据活跃效果的 `frameIntervalMs` 动态调整，以在流畅度和CPU使用率之间取得平衡。

### 主循环执行顺序

初始化之后 `APP_TaskLoop()` 持续执行 (app.c:711)：

```
while (1):
  1. GpLedMatrixUsbDebug_Run()         // USB调试模式（激活时进入阻塞行测试循环）
  2. GpLedMatrixAi8051u_Poll()         // 排空UART2环形缓冲，解析协议包
     ↓ 命令分发给 gp_led_action 处理函数
  3. GpLedAction_RenderPendingAnimationFrame()  // ISR侧推进的动画帧的延迟渲染
     ↓ 将动画帧索引→实际的图像缓冲写入（重量级编码）
  4. MidTask_Process()                 // 执行待处理的协作式任务
     ↓ 任务0 (10ms): KeyTask → 动作 + 按键 + 本地方案
     ↓ 任务1 (~32ms): DrawFrame → 本地渲染 + 动画
```

**排序理由**：
- 协议轮询（步骤2）在渲染之前运行，以便立即应用任何新接收的远程帧
- 动画渲染（步骤3）在协作式任务之前运行，使远程动画帧在同一循环迭代中显示
- 协作式任务（步骤4）最后运行——本地渲染填充剩余时间，若远程内容活跃则被跳过

### 定时器ISR执行流程

```
Timer1 ISR (每 ~1000us，最高优先级):
  WS2812DRV_RefreshStep():
    - 构建双行PWM DMA缓冲（CH1=偶行, CH2=奇行）
    - 配置HC595行选择（当前步进）
    - 在两通道上启动PWMAT-DMA传输
    - 推进步进索引（0..7，循环回到0）
  → ISR总耗时：<< 100us（仅缓冲设置 + DMA触发）

Timer0 ISR (每 ~500us，通过单次触发自复位):
  g_timerTickMs++
  若单次触发计数器已到期:
    调用 APP_OnSchedTickExpired():
      MidTask_Tick1ms()              // 更新所有任务截止时间（O(n), n≤8）
      GpLedAction_Tick1ms()           // 推进动画帧索引（常数时间）
      TIMER0_StartOneShotUs(500)      // 重新设置下一次节拍
  → ISR总耗时：< 20us（纯计数器更新）
```

Timer1 ISR有严格的实时约束——必须在下一个行对步进之前完成。DMA承担了实际PWM信号生成的重负载，因此ISR只需设置下一次传输。

### 任务周期动态管理

绘制帧任务周期可动态调整：

```
用户/AI改变效果
  → DrawDrv_SetRenderConfig() 更新 frameIntervalMs
    → 在下次 APP_DrawFrameTaskProxy() 调用时：
      APP_SyncLocalDrawTaskPeriodFromDriver()
        → 读取当前 frameIntervalMs
        → 通过 DrawDrv_NormalizeFrameIntervalMs() 规范化
        → 若变更：MidTask_SetPeriod(taskId, newPeriodMs)
          → 重置：tickCount = newPeriod, pendingCount = 0
          → 任务现在以新的节奏触发
```

这允许需要更快更新的效果（如滚动16ms）以更高频率运行，而静态显示（纯色）可以降低到更低频率以节省CPU。

---

## 系统总体时序图

```
Timer0 (500us自复位)        Timer1 (1000us)           主循环
     │                          │                      │
     ├─ ISR: tick++             │                      │
     ├─ MidTask_Tick1ms()       │                      │
     ├─ GpLedAction_Tick1ms()   │                      │
     └─ 重新设置                │                      │
     │                          ├─ ISR: RefreshStep()  │
     │                          │  构建DMA缓冲          │
     │                          │  启动DMA传输          │
     │                          │  步进索引++           │
     │                          └─ 返回                │
     │                                                 ├─ GpLedMatrixAi8051u_Poll()
     │                                                 │  收包 + 解析 + 分发
     │                                                 ├─ GpLedAction_RenderPendingAnimationFrame()
     │                                                 │  动画帧渲染（重量级）
     │                                                 └─ MidTask_Process()
     │                                                     ├─ Task0(10ms): KeyTask
     │                                                     └─ Task1(32ms): DrawFrame
     │                          │                      │
     ...                        ...                    ...（循环）
```
