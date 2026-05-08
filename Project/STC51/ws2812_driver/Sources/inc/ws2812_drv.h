#ifndef __WS2812_DRV_H__
#define __WS2812_DRV_H__

#define WS2812DRV_ROW_NUM                    16
#define WS2812DRV_COL_NUM_MAX                16
#define WS2812DRV_COL_NUM_8                  8
#define WS2812DRV_COL_NUM_16                 16
#define WS2812DRV_PIXEL_CHANNELS             3
#define WS2812DRV_ROW_RESET_PREFIX_SLOTS     48
#define WS2812DRV_PWM_NUM_MAX                (WS2812DRV_ROW_RESET_PREFIX_SLOTS + WS2812DRV_COL_NUM_MAX * 24 + 2)
#define WS2812DRV_PWM_NUM_DUAL_MAX           (WS2812DRV_PWM_NUM_MAX * 2)
#define WS2812DRV_RESET_TAIL_SLOTS           32

#define WS2812DRV_PWM_DUTY_BIT0              12
#define WS2812DRV_PWM_DUTY_BIT1              36
#define WS2812DRV_ROW_SWITCH_SETTLE_US       3
#define WS2812DRV_DMA_TAIL_GUARD_BYTES       (WS2812DRV_RESET_TAIL_SLOTS * 2 + 2)

typedef enum
{
	WS2812DRV_MODE_16X8 = 0,
	WS2812DRV_MODE_16X16 = 1
} WS2812DRV_DisplayMode_t;

typedef enum
{
	WS2812DRV_SCAN_NORMAL_PAIR = 0,
	WS2812DRV_SCAN_LEGACY_SHIFT = 1
} WS2812DRV_ScanMode_t;

/* 初始化 WS2812 驱动。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_Init(void);

/* 清空图像后缓冲区。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_ClearImage(void);

/* 设置单个像素 RGB 颜色（写入后缓冲，GRB 顺序编码）。
 * 参数: row 行号, col 列号, r 红色, g 绿色, b 蓝色
 * 返回: 无
 */
void WS2812DRV_SetPixelRgb(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b);

/* 开始一帧快速写入（关闭逐像素脏检查）。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_BeginFrameWrite(void);

/* 快速写单像素 RGB（无比较，适合整帧重建）。
 * 参数: row 行号, col 列号, r 红色, g 绿色, b 蓝色
 * 返回: 无
 */
void WS2812DRV_SetPixelRgbFast(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b);

/* 结束一帧快速写入并标记图像脏。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_EndFrameWrite(void);

/* 将后缓冲图像编码为待刷新的 PWM 行缓冲。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_EncodeAllRows(void);

/* 构建双行交织 DMA 数据包。
 * 参数: rowA 前一行, rowB 后一行
 * 返回: 有效发送字节数
 */
uint16_t WS2812DRV_BuildDualRowPwmBuffer(uint8_t rowA, uint8_t rowB);

/* 获取双行交织 DMA 缓冲指针。
 * 参数: 无
 * 返回: DMA 缓冲指针
 */
uint8_t xdata *WS2812DRV_GetDualRowPwmBuffer(void);

/* 更新行选（采用先全关再选通策略）。
 * 参数: rowA 前一行, rowB 后一行
 * 返回: 无
 */
void WS2812DRV_SelectRows(uint8_t rowA, uint8_t rowB);

/* 触发双行 DMA 发送。
 * 参数: txBuf 发送缓冲地址, num 发送长度
 * 返回: 无
 */
void WS2812DRV_TriggerDualRowDma(uint8_t xdata *txBuf, uint16_t num);

/* 等待 DMA 完成（带超时恢复）。
 * 参数: 无
 * 返回: 1 成功, 0 失败
 */
bit WS2812DRV_WaitDmaDone(void);

/* 关闭 PWM 双通道输出。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_StopPwmDualChannels(void);

/* 一次性发送指定双行。
 * 参数: rowA 前一行, rowB 后一行
 * 返回: 1 成功, 0 失败
 */
bit WS2812DRV_SendRowPair(uint8_t rowA, uint8_t rowB);

/* 刷新一步（供 Timer1 ISR 周期调用）。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_RefreshStep(void);

/* DMA 中断回调。
 * 参数: 无
 * 返回: 无
 */
void WS2812DRV_OnDmaIsr(void);

/* 查询 DMA 忙状态。
 * 参数: 无
 * 返回: 1 忙, 0 空闲
 */
bit WS2812DRV_IsDmaBusy(void);

/* 查询 PWM 缓冲区交换是否待处理。
 * 参数: 无
 * 返回: 1 待处理, 0 已完成
 */
bit WS2812DRV_IsPwmSwapPending(void);

/* 设置显示模式（16x8 / 16x16）。
 * 参数: mode 显示模式
 * 返回: 1 成功, 0 失败
 */
uint8_t WS2812DRV_SetDisplayMode(WS2812DRV_DisplayMode_t mode);

/* 获取当前显示模式。
 * 参数: 无
 * 返回: 当前显示模式
 */
WS2812DRV_DisplayMode_t WS2812DRV_GetDisplayMode(void);

/* 获取当前有效列数。
 * 参数: 无
 * 返回: 有效列数（8 或 16）
 */
uint8_t WS2812DRV_GetActiveCols(void);

/* 设置扫描模式。
 * 参数: mode 扫描模式
 * 返回: 1 成功, 0 失败
 */
uint8_t WS2812DRV_SetScanMode(WS2812DRV_ScanMode_t mode);

/* 获取当前扫描模式。
 * 参数: 无
 * 返回: 当前扫描模式
 */
WS2812DRV_ScanMode_t WS2812DRV_GetScanMode(void);

/* 切换扫描模式（正常双行/旧方案滑窗）。
 * 参数: 无
 * 返回: 切换后的扫描模式
 */
WS2812DRV_ScanMode_t WS2812DRV_ToggleScanMode(void);

/* 获取最近一次扫描输出行A。
 * 参数: 无
 * 返回: 行号A
 */
uint8_t WS2812DRV_GetLastScanRowA(void);

/* 获取最近一次扫描输出行B。
 * 参数: 无
 * 返回: 行号B
 */
uint8_t WS2812DRV_GetLastScanRowB(void);

/* 获取最近一次扫描DMA发送长度。
 * 参数: 无
 * 返回: DMA发送长度
 */
uint16_t WS2812DRV_GetLastScanTxLen(void);

/* 获取刷新步执行累计次数。
 * 参数: 无
 * 返回: 刷新步累计次数
 */
uint16_t WS2812DRV_GetRefreshCounter(void);

/* 获取DMA触发累计次数。
 * 参数: 无
 * 返回: DMA触发累计次数
 */
uint16_t WS2812DRV_GetDmaTriggerCounter(void);

/* 获取DMA完成累计次数。
 * 参数: 无
 * 返回: DMA完成累计次数
 */
uint16_t WS2812DRV_GetDmaDoneCounter(void);

/* 获取DMA等待超时累计次数。
 * 参数: 无
 * 返回: DMA等待超时累计次数
 */
uint16_t WS2812DRV_GetDmaTimeoutCounter(void);

/* 获取DMA源地址未对齐累计次数。
 * 参数: 无
 * 返回: DMA源地址未对齐累计次数
 */
uint16_t WS2812DRV_GetDmaOddAddrCounter(void);

/* 获取最近一次DMA发送源地址。
 * 参数: 无
 * 返回: 最近一次DMA发送源地址
 */
uint16_t WS2812DRV_GetLastTxAddr(void);

#endif
