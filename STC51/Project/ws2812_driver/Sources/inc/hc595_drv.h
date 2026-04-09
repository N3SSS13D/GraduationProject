#ifndef __HC595_DRV_H__
#define __HC595_DRV_H__

/* 初始化 74HC595 控制引脚。
 * 参数: 无
 * 返回: 无
 */
void HC595_Init(void);

/* 写入 16 位行选位图（0 为导通，1 为关断）。
 * 参数: value 16 位位图
 * 返回: 无
 */
void HC595_Write16(uint16_t value);

/* 选通两行。
 * 参数: rowA 第一行, rowB 第二行
 * 返回: 无
 */
void HC595_SelectRows(uint8_t rowA, uint8_t rowB);

/* 关闭所有行选。
 * 参数: 无
 * 返回: 无
 */
void HC595_AllOff(void);

#endif
