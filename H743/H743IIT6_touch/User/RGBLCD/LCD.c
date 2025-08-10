/*
 * LCD.c
 *
 *  Created on: Jun 16, 2025
 *      Author: 12114
 */


#include "LCD.h"

uint16_t LCD_Buffer0[800][480] __attribute__((section(".sdram_section"), aligned(16)));
uint16_t LCD_Buffer1[800][480] __attribute__((section(".sdram_section"), aligned(16)));


/* 画点函数[800]x[480]屏幕
 * 任意一个点在内存里的的位置 = 行大小*a距离 + 列距离
 *
 * 横屏情况(屏幕x坐标 对应 数组行, 屏幕y坐标 对应 数组列): 起始地址 + 像素大小*(行大小*y + x)
 *
 * 竖屏情况(屏幕x坐标 对应 数组列, 屏幕y坐标 对应 数组行): 起始地址 + 像素大小*(行大小* x 		 + y)
 * 															  *(行大小*(列大小-1-x) + y)
 *    这里用画面逆时针转90°情况: 此时第一个像素在数组[0][479]也就是屏幕横着看的左下角,所以x的计算是数组最大列-坐标
 */
void LTDC_Draw_Point_horizontal(uint16_t x,uint16_t y,uint32_t color,uint32_t Layeraddr)
{
	*(uint16_t*)((uint32_t)Layeraddr + 2*(800*y + x)) = color;
}
void LTDC_Draw_Point_vertical(uint16_t x,uint16_t y,uint32_t color,uint32_t Layeraddr)
{
	*(uint16_t*)((uint32_t)Layeraddr + 2*(800*(479-x) + y)) = color;
}

/*	读点函数
 *
 */
void LTDC_Read_Point_horizontal(uint16_t x,uint16_t y,uint32_t Layeraddr)
{
	return *(uint16_t*)((uint32_t)Layeraddr + 2*(800*y + x));
}
void LTDC_Read_Point_vertical(uint16_t x,uint16_t y,uint32_t Layeraddr)
{
	return *(uint16_t*)((uint32_t)Layeraddr + 2*(800*(479-x) + y));
}

/*	DMA2D操作函数
 *
 */
static inline void DMA2D_Fill( void * pDst, uint32_t width, uint32_t height, uint32_t lineOff, uint32_t pixelFormat,  uint32_t color)
{
    /* DMA2D配置 */
    DMA2D->CR      = 0x00030000UL;                                  // 配置为寄存器到储存器模式
    DMA2D->OCOLR   = color;                                         // 设置填充使用的颜色，格式应该与设置的颜色格式相同
    DMA2D->OMAR    = (uint32_t)pDst;                                // 填充区域的起始内存地址
    DMA2D->OOR     = lineOff;                                       // 行偏移，即跳过的像素，注意是以像素为单位
    DMA2D->OPFCCR  = pixelFormat;                                   // 设置颜色格式
    DMA2D->NLR     = (uint32_t)(width << 16) | (uint16_t)height;    // 设置填充区域的宽和高，单位是像素

    /* 启动传输 */
    DMA2D->CR   |= DMA2D_CR_START;

    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START) {}
}

/*	DMA2D填充函数_横屏
 *
 */
void FillRect_horizontal(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color,uint32_t Layeraddr){
    void* pDist = &(((uint16_t*)Layeraddr)[y*800 + x]);//起始点
    DMA2D_Fill(pDist, w, h, 800 - w, LTDC_PIXEL_FORMAT_RGB565, color);
}
/*	DMA2D填充函数_竖屏
 *
 */
void FillRect_vertical(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color,uint32_t Layeraddr){
    void* pDist = &(((uint16_t*)Layeraddr)[(479-(x+w))*800 + y]);//起始点
    DMA2D_Fill(pDist, h, w, 800 - h, LTDC_PIXEL_FORMAT_RGB565, color);
}


void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t* color,uint32_t buffer)
{
	uint32_t psx,psy,pex,pey;
	uint32_t timeout=0;
	uint16_t offline;
	uint32_t addr;

	// 按屏幕使用方式对坐标进行变换
	if(1) // 横屏
	{
		psx=sx;psy=sy;
		pex=ex;pey=ey;
	}else // 竖屏
	{
		psx=sy;psy=480-ex-1; // 272为屏幕高度像素
		pex=ey;pey=480-sx-1;
	}
	offline=800-(pex-psx+1);// 480为屏幕宽度像素
	addr=((uint32_t)buffer+2*(800*psy+psx)); // 2为像素字节大小，480为屏幕宽度像素
	DMA2D->CR&=~(DMA2D_CR_START); //停止DMA2D
	DMA2D->CR=DMA2D_M2M; //DMA2D配置为内存到内存
	DMA2D->FGMAR=(uint32_t)color; // 需要传输的颜色数据地址
	DMA2D->OMAR=addr; //输出存储器地址
	DMA2D->FGOR=0; // 前景层偏移
	DMA2D->OOR=offline; //行偏移
	DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16); //设定行数寄存器
	DMA2D->CR|=DMA2D_CR_START; //启动 DMA2D
	while((DMA2D->ISR&(DMA2D_FLAG_TC))==0) //等待传输完成
	{
		timeout++;
		if(timeout>0X1FFFFF)break;//超时退出
	}
	DMA2D->IFCR|=DMA2D_FLAG_TC;//清除传输完成标志
}
