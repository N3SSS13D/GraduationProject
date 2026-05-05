#ifndef __DRAW_DRV_H__
#define __DRAW_DRV_H__

typedef enum
{
	DRAWDRV_EFFECT_STATIC = 0,
	DRAWDRV_EFFECT_BREATH = 1,
	DRAWDRV_EFFECT_GRADIENT = 2,
	DRAWDRV_EFFECT_SCROLL_LEFT = 3,
	DRAWDRV_EFFECT_SCROLL_RIGHT = 4,
	DRAWDRV_EFFECT_TEXT_SCROLL_JLU = 5,
	DRAWDRV_EFFECT_FADE_IN = 6,
	DRAWDRV_EFFECT_FADE_OUT = 7,
	DRAWDRV_EFFECT_COLOR_CYCLE = 8
} DrawDrv_Effect_t;

typedef enum
{
	DRAWDRV_CONTENT_PATTERN = 0,
	DRAWDRV_CONTENT_GLYPH = 1,
	DRAWDRV_CONTENT_SOLID = 2
} DrawDrv_ContentType_t;

typedef enum
{
	DRAWDRV_COLOR_SOLID = 0,
	DRAWDRV_COLOR_GRADIENT = 1
} DrawDrv_ColorMode_t;

typedef enum
{
	DRAWDRV_DIR_NORMAL = 0,
	DRAWDRV_DIR_ROTATE_180 = 1,
	DRAWDRV_DIR_ROTATE_CW_90 = 2,
	DRAWDRV_DIR_ROTATE_CCW_90 = 3
} DrawDrv_Direction_t;

typedef enum
{
	DRAWDRV_TIMELINE_PATH_LINEAR = 0,
	DRAWDRV_TIMELINE_PATH_EASE_IN_OUT = 1,
	DRAWDRV_TIMELINE_PATH_BREATH_CURVE = 2
} DrawDrv_TimelinePath_t;

typedef struct
{
	uint8_t fgR;
	uint8_t fgG;
	uint8_t fgB;
	uint8_t bgR;
	uint8_t bgG;
	uint8_t bgB;
	uint8_t brightness;
	DrawDrv_ContentType_t contentType;
	DrawDrv_ColorMode_t colorMode;
	DrawDrv_Direction_t direction;
	uint8_t useGradient;
	uint8_t gradientSpan;
	uint8_t scrollStep;
	uint8_t animStep;
	DrawDrv_Effect_t effect;
	uint16_t timelineDurationMs;
	uint16_t timelineRepeatDelayMs;
	uint8_t timelineRepeatCount;
	DrawDrv_TimelinePath_t timelinePath;
} DrawDrv_RenderConfig_t;

/* 绘图驱动初始化。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_Init(void);

/* 32ms 绘图任务（约31fps）。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_Task32ms(void);

/* 请求下一帧重建（用于模式切换等场景）。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_RequestRebuild(void);

/* 设置渲染参数（前景/背景/渐变/动画）。
 * 参数: cfg 渲染配置
 * 返回: 无
 */
void DrawDrv_SetRenderConfig(const DrawDrv_RenderConfig_t *cfg);

/* 读取当前渲染参数。
 * 参数: cfg 输出配置
 * 返回: 无
 */
void DrawDrv_GetRenderConfig(DrawDrv_RenderConfig_t *cfg);

/* 选择当前图案索引（会循环到有效范围）。
 * 参数: imageIndex 图案索引
 * 返回: 无
 */
void DrawDrv_SetImageIndex(uint8_t imageIndex);

/* 获取当前图案索引。
 * 参数: 无
 * 返回: 当前索引
 */
uint8_t DrawDrv_GetImageIndex(void);

/* 切换到下一张图案（循环）。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_NextImage(void);

/* 设置静态文字显示的字模索引（非滚动文字模式生效）。
 * 参数: glyphIndex 字模索引
 * 返回: 1 成功, 0 失败
 */
uint8_t DrawDrv_SetTextDisplayGlyph(uint8_t glyphIndex);

/* 设置滚动字幕字模序列。
 * 参数: glyphList 字模索引数组
 * 参数: count 数组长度
 * 返回: 1 成功, 0 失败
 */
uint8_t DrawDrv_SetTextScrollSequence(const uint8_t *glyphList, uint8_t count);

#endif
