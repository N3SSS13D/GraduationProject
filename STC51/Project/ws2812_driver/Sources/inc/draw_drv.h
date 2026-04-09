#ifndef __DRAW_DRV_H__
#define __DRAW_DRV_H__

void DrawDrv_Init(void);
void DrawDrv_Task40ms(void);
void DrawDrv_Task500ms(void);
uint8_t DrawDrv_EnableSingleRowDebug(uint8_t row);
void DrawDrv_DisableSingleRowDebug(void);

#endif
