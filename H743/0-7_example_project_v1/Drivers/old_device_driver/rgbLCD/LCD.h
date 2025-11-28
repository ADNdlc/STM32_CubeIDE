/*
 * LCD.h
 *
 *  Created on: Jun 16, 2025
 *      Author: 12114
 */

#ifndef RGBLCD_LCD_H_
#define RGBLCD_LCD_H_

#include "stm32H7xx_hal.h"

uint16_t* get_BackBuf(void);
uint16_t* get_FrontBuf(void);

void LTDC_Draw_Point_horizontal(uint16_t x,uint16_t y,uint32_t color,uint32_t Layeraddr);
void LTDC_Draw_Point_vertical(uint16_t x,uint16_t y,uint32_t color,uint32_t Layeraddr);

uint16_t* LTDC_Read_Point_horizontal(uint16_t x,uint16_t y,uint32_t Layeraddr);
uint16_t* LTDC_Read_Point_vertical(uint16_t x,uint16_t y,uint32_t Layeraddr);


void FillRect_horizontal(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color,uint32_t Layeraddr);
void FillRect_vertical(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color,uint32_t Layeraddr);

void DMA2D_Copy(void * pSrc,
				void * pDst,
				uint32_t xSize,
				uint32_t ySize,
				uint32_t OffLineSrc,
				uint32_t OffLineDst,
				uint32_t PixelFormat);

void DMA2D_Copy_IT(void * pSrc,
				void * pDst,
				uint32_t xSize,
				uint32_t ySize,
				uint32_t OffLineSrc,
				uint32_t OffLineDst,
				uint32_t PixelFormat);

//void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t* color,uint32_t buffer);

#endif /* RGBLCD_LCD_H_ */
