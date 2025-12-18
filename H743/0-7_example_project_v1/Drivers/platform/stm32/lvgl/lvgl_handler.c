/*
 * lvgl_handler.c
 *
 *  Created on: Dec 8, 2025
 *      Author: 12114
 */
#if 1
#include "elog.h"
#include "lcd_hal/lcd_hal.h"
#include "ltdc.h"
#include "lvgl.h"
#include "main.h"
#include "tim.h"

extern lv_disp_drv_t *it_disp_drv; // lvgl显示屏句柄
extern lcd_hal_t *lvgl_display;    // 底层lcd

// 双缓冲同步标志
static volatile uint8_t pending_swap = 0; // 有内容待交换

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim6) {
    lv_tick_inc(1); // 给lvgl提供时基
  }
}

extern DMA2D_HandleTypeDef hdma2d;
void DMA2D_IRQHandler(void) {
  /* USER CODE BEGIN DMA2D_IRQn 0 */

  if ((DMA2D->ISR & DMA2D_FLAG_TC) != 0) {
    DMA2D->IFCR = DMA2D_FLAG_TC; // 清除"传输完成"中断标志位
    /* 注意：lv_disp_flush_ready 现在由 disp_flush 同步调用 */
  } else if ((DMA2D->ISR & DMA2D_FLAG_TE) != 0) { // 处理可能发生的错误中断
    DMA2D->IFCR = DMA2D_FLAG_TE;                  // 清除"传输错误"中断标志位
  }
  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&hdma2d);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/*	ltdc行中断回调
 * 	需要配置为0行中断(垂直消隐周期)
 *  只有当有新内容准备好时才执行交换
 */
void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *ltdc) {
  if (pending_swap) {
    pending_swap = 0; // 清除标志
    lcd_hal_swap_buffer(lvgl_display);
  }
  // 重新使能行中断（HAL库会在处理中断后禁用行中断）
  HAL_LTDC_ProgramLineEvent(ltdc, 0);
}

void lvgl_handler_set_pending_swap(void) { pending_swap = 1; }

#endif
