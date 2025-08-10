/*
 * LCD.h
 *
 *  Created on: Jun 16, 2025
 *      Author: 12114
 */

#ifndef RGBLCD_LCD_H_
#define RGBLCD_LCD_H_

#include "stm32H7xx_hal.h"

extern uint16_t LCD_Buffer0[800][480];
extern uint16_t LCD_Buffer1[800][480];



void LTDC_Draw_Point_horizontal(uint16_t x,uint16_t y,uint32_t color,uint32_t Layeraddr);
void LTDC_Draw_Point_vertical(uint16_t x,uint16_t y,uint32_t color,uint32_t Layeraddr);

void LTDC_Read_Point_horizontal(uint16_t x,uint16_t y,uint32_t Layeraddr);
void LTDC_Read_Point_vertical(uint16_t x,uint16_t y,uint32_t Layeraddr);


void FillRect_horizontal(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color,uint32_t Layeraddr);
void FillRect_vertical(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color,uint32_t Layeraddr);


void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t* color,uint32_t buffer);

#endif /* RGBLCD_LCD_H_ */
