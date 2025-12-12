/*
 * lvgl_handler.c
 *
 *  Created on: Dec 8, 2025
 *      Author: 12114
 */
#if 1
#include "lcd_hal/lcd_hal.h"
#include "lvgl.h"
#include "main.h"
#include "tim.h"
#include "elog.h"

extern lv_disp_drv_t *it_disp_drv; // lvgl显示屏句柄
extern lcd_hal_t *lvgl_display;    // 底层lcd

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim6) {
    lv_tick_inc(1); // 给lvgl提供时基

  }
}

// 在dma2d传输完成中断中通知lvgl刷新完成
void HAL_DMA2D_CLUTLoadingCpltCallback(DMA2D_HandleTypeDef *hdma2d) {
  if ((DMA2D->ISR & DMA2D_FLAG_TC) != 0) {
    DMA2D->IFCR = DMA2D_FLAG_TC; // 清除“传输完成”中断标志位
    if (it_disp_drv != NULL) {
      lv_disp_flush_ready(it_disp_drv); // 调用 lv_disp_flush_ready()
      log_i(".");
    }
  } else if ((DMA2D->ISR & DMA2D_FLAG_TE) != 0) { // 处理可能发生的错误中断
    DMA2D->IFCR = DMA2D_FLAG_TE;                  // 清除“传输错误”中断标志位
    if (it_disp_drv != NULL) {
      lv_disp_flush_ready(it_disp_drv);
    }
  }
}

/*	ltdc行中断回调
 * 	需要配置为0行中断(垂直消隐周期)
 */
void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *ltdc){
	if (lvgl_display != NULL) {
	  lcd_hal_swap_buffer(lvgl_display);
	}
}

#endif
