#ifndef __BIGNUMBER_H
#define __BIGNUMBER_H

void LCD_ShowSingleBigNum(uint16_t x, uint16_t y, uint8_t Num, uint8_t Size, uint8_t Mode);
void LCD_ShowBigNum(uint16_t x, uint16_t y, uint32_t Num, uint8_t Len, uint8_t Size, uint8_t Mode);
void LCD_ShowBigEmpty(uint16_t x, uint16_t y, uint8_t Size, uint8_t Mode);

#endif
