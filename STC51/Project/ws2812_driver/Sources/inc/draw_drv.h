#ifndef __DRAW_DRV_H__
#define __DRAW_DRV_H__

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

/* 开启单行调试显示。
 * 参数: row 行号
 * 返回: 1 成功, 0 失败
 */
uint8_t DrawDrv_EnableSingleRowDebug(uint8_t row);

/* 关闭单行调试显示。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_DisableSingleRowDebug(void);

/* 请求下一帧重建（用于模式切换等场景）。
 * 参数: 无
 * 返回: 无
 */
void DrawDrv_RequestRebuild(void);

#endif
