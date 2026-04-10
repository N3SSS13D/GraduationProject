#ifndef __DRAW_DRV_H__
#define __DRAW_DRV_H__

typedef enum
{
	DRAWDRV_EFFECT_STATIC = 0,
	DRAWDRV_EFFECT_BREATH = 1,
	DRAWDRV_EFFECT_GRADIENT = 2,
	DRAWDRV_EFFECT_SCROLL_LEFT = 3,
	DRAWDRV_EFFECT_SCROLL_RIGHT = 4,
	DRAWDRV_EFFECT_TEXT_SCROLL_JLU = 5
} DrawDrv_Effect_t;

typedef struct
{
	uint8_t fgR;
	uint8_t fgG;
	uint8_t fgB;
	uint8_t bgR;
	uint8_t bgG;
	uint8_t bgB;
	uint8_t useGradient;
	uint8_t gradientSpan;
	uint8_t scrollStep;
	DrawDrv_Effect_t effect;
} DrawDrv_RenderConfig_t;

/* 绘图驱动初始化。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_Init(void);

/* 40ms 绘图任务（25fps）。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_Task40ms(void);

/* 500ms 动画状态更新任务。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_Task500ms(void);

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

#endif
