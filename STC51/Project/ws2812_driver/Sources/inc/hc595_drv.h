#ifndef __HC595_DRV_H__
#define __HC595_DRV_H__

void HC595_Init(void);
void HC595_Write16(uint16_t value);
void HC595_SelectRows(uint8_t rowA, uint8_t rowB);
void HC595_AllOff(void);

#endif
