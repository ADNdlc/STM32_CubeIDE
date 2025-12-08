/*
 * lvgl_handler.c
 *
 *  Created on: Dec 8, 2025
 *      Author: 12114
 */
#if 0
#include "lvgl_handler.h"
#include "stm32h7xx_it.h"
#include "lvgl.h"

extern lv_disp_drv_t *it_disp_drv; // 显示屏句柄

// 在dma2d传输完成中断中通知lvgl刷新
void HAL_DMA2D_CLUTLoadingCpltCallback(DMA2D_HandleTypeDef *hdma2d)
{
    if ((DMA2D->ISR & DMA2D_FLAG_TC) != 0)
    {
        DMA2D->IFCR = DMA2D_FLAG_TC; // 清除“传输完成”中断标志位
        if (it_disp_drv != NULL)
        {
            lv_disp_flush_ready(it_disp_drv); // 调用 lv_disp_flush_ready()
        }
    }
    else if ((DMA2D->ISR & DMA2D_FLAG_TE) != 0)
    {                                // 处理可能发生的错误中断
        DMA2D->IFCR = DMA2D_FLAG_TE; // 清除“传输错误”中断标志位
        if (it_disp_drv != NULL)
        {
            lv_disp_flush_ready(it_disp_drv);
        }
    }
}
#endif
