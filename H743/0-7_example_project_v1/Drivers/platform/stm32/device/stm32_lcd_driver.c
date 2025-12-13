/*
 * stm32_lcd_driver.c
 *
 *  Created on: Dec 8, 2025
 *      Author: Antigravity
 */

#include "stm32_lcd_driver.h"
#include "dma2d.h"
#include "ltdc.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>


// --- Private Helper Functions ---

static inline void _dma2d_fill(void *pDst, uint32_t width, uint32_t height,
                               uint32_t lineOff, uint32_t pixelFormat,
                               uint32_t color) {
  /* DMA2D Configuration */
  DMA2D->CR = 0x00030000UL;
  DMA2D->OCOLR = color;
  DMA2D->OMAR = (uint32_t)pDst;
  DMA2D->OOR = lineOff;
  DMA2D->OPFCCR = pixelFormat;
  DMA2D->NLR = (uint32_t)(width << 16) | (uint16_t)height;

  /* Start */
  DMA2D->CR |= DMA2D_CR_START;

  /* Wait */
  while (DMA2D->CR & DMA2D_CR_START) {
  }
}

#define IT 1

static inline void _dma2d_copy(void *pDst, void *pSrc, uint32_t xSize,
                               uint32_t ySize, uint32_t OffLineSrc,
                               uint32_t OffLineDst, uint32_t PixelFormat) {
#if IT
  DMA2D->IFCR = 0x3FUL; // 清除中断标志

  /* DMA2D采用存储器到存储器模式 */
  DMA2D->CR = 0x00000000UL | (1 << 9); // 传输完全中断
  DMA2D->FGMAR = (uint32_t)pSrc;       // 前景地址(源)
  DMA2D->OMAR = (uint32_t)pDst;        // 目标地址
  DMA2D->FGOR = OffLineSrc;            // 前景偏移
  DMA2D->OOR = OffLineDst;             // 输出地址偏移

  /* 前景层和输出区域都采用的RGB565颜色格式 */
  DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
  DMA2D->OPFCCR = LTDC_PIXEL_FORMAT_RGB565;

  DMA2D->NLR = (uint32_t)(xSize << 16) | (uint16_t)ySize; // 行数

  DMA2D->CR |= DMA2D_CR_TCIE | DMA2D_CR_TEIE; // 中断:传输完成,传输错误
  /* 启动传输 */
  DMA2D->CR |= DMA2D_CR_START;
#else
  DMA2D->CR = 0x00000000UL | (1 << 9);
  DMA2D->FGMAR = (uint32_t)pSrc; // 前景地址(源)
  DMA2D->OMAR = (uint32_t)pDst;  // 目标地址
  DMA2D->FGOR = OffLineSrc;      // 前景偏移
  DMA2D->OOR = OffLineDst;       // 输出地址偏移

  /* 前景层和输出区域都采用的RGB565颜色格式 */
  DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
  DMA2D->OPFCCR = LTDC_PIXEL_FORMAT_RGB565;

  DMA2D->NLR = (uint32_t)(xSize << 16) | (uint16_t)ySize; // 行数

  DMA2D->CR |=
      DMA2D_IT_TC | DMA2D_IT_TE | DMA2D_IT_CE; // 中断:传输完成,两个错误
  /* 启动传输 */
  DMA2D->CR |= DMA2D_CR_START;

  /* 等待DMA2D传输完成 */
  while (DMA2D->CR & DMA2D_CR_START) {
  }
#endif
}

// --- Driver Implementation Functions ---

static void stm32_lcd_draw_point(lcd_driver_t *self, uint16_t x, uint16_t y,
                                 uint32_t color) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;

  if (self->orientation == LCD_ORIENTATION_PORTRAIT) {
    if (x >= 480 || y >= 800)
      return;
    uint32_t addr =
        (uint32_t)impl->base.draw_buffer + 2 * (800 * (479 - x) + y);
    *(uint16_t *)addr = (uint16_t)color;
  } else {
    if (x >= 800 || y >= 480)
      return;
    uint32_t addr = (uint32_t)impl->base.draw_buffer + 2 * (800 * y + x);
    *(uint16_t *)addr = (uint16_t)color;
  }
}

static uint32_t stm32_lcd_read_point(lcd_driver_t *self, uint16_t x,
                                     uint16_t y) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;
  uint16_t color = 0;

  if (self->orientation == LCD_ORIENTATION_PORTRAIT) {
    if (x >= 480 || y >= 800)
      return 0;
    uint32_t addr =
        (uint32_t)impl->base.draw_buffer + 2 * (800 * (479 - x) + y);
    color = *(uint16_t *)addr;
  } else {
    if (x >= 800 || y >= 480)
      return 0;
    uint32_t addr = (uint32_t)impl->base.draw_buffer + 2 * (800 * y + x);
    color = *(uint16_t *)addr;
  }
  return color;
}

static void stm32_lcd_fill_rect(lcd_driver_t *self, uint16_t x, uint16_t y,
                                uint16_t w, uint16_t h, uint32_t color) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;

  if (self->orientation == LCD_ORIENTATION_PORTRAIT) {
    if (x + w > 480)
      w = 480 - x;
    if (y + h > 800)
      h = 800 - y;

    if (479 < (x + w))
      return;

    uint32_t pDist_offset = (479 - (x + w)) * 800 + y;
    void *pDist = (void *)((uint32_t)impl->base.draw_buffer + pDist_offset * 2);

    _dma2d_fill(pDist, h, w, 800 - h, LTDC_PIXEL_FORMAT_RGB565, color);
  } else {
    if (x + w > 800)
      w = 800 - x;
    if (y + h > 480)
      h = 480 - y;

    uint32_t pDist_offset = y * 800 + x;
    void *pDist = (void *)((uint32_t)impl->base.draw_buffer + pDist_offset * 2);

    _dma2d_fill(pDist, w, h, 800 - w, LTDC_PIXEL_FORMAT_RGB565, color);
  }
}

static void stm32_lcd_draw_bitmap(lcd_driver_t *self, uint16_t x, uint16_t y,
                                  uint16_t w, uint16_t h, const void *pSrc) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;

  if (self->orientation == LCD_ORIENTATION_LANDSCAPE) {
    if (x + w > 800)
      w = 800 - x;
    if (y + h > 480)
      h = 480 - y;

    uint32_t pDist_offset = y * 800 + x;
    void *pDist = (void *)((uint32_t)impl->base.draw_buffer + pDist_offset * 2);

    _dma2d_copy(pDist, (void *)pSrc, w, h, 0, 800 - w,
                LTDC_PIXEL_FORMAT_RGB565);
  }
}

static void stm32_lcd_copy_buffer(void *pDst, void *pSrc, uint32_t xSize,
                                  uint32_t ySize, uint32_t OffLineSrc,
                                  uint32_t OffLineDst, uint32_t PixelFormat) {
  // 目标buffer, 源buffer
  _dma2d_copy(pDst, pSrc, xSize, ySize, OffLineSrc, OffLineDst, PixelFormat);
}

static void stm32_lcd_set_orientation(lcd_driver_t *self,
                                      lcd_orientation_t orientation) {
  self->orientation = orientation;
  if (orientation == LCD_ORIENTATION_PORTRAIT) {
    self->width = 480;
    self->height = 800;
  } else {
    self->width = 800;
    self->height = 480;
  }
}

static void stm32_lcd_swap_buffer(lcd_driver_t *self) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;

  // Swap pointers
  uint16_t *temp = impl->base.display_buffer;
  impl->base.display_buffer = impl->base.draw_buffer;
  impl->base.draw_buffer = temp;

  // 更新LTDC硬件使用新的显示缓冲区地址
  if (impl->hltdc) {
    HAL_LTDC_SetAddress(impl->hltdc, (uint32_t)impl->base.display_buffer, 0);
  }
}

static uint16_t *stm32_lcd_get_drawbuf(lcd_driver_t *self) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;
  return impl->base.draw_buffer;
}

static uint16_t *stm32_lcd_get_displaybuf(lcd_driver_t *self) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;
  return impl->base.display_buffer;
}

static void stm32_lcd_set_drawbuf(lcd_driver_t *self, uint16_t *buffer) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;
  impl->base.draw_buffer = buffer;
}

static void stm32_lcd_set_displaybuf(lcd_driver_t *self, uint16_t *buffer) {
  stm32_lcd_driver_t *impl = (stm32_lcd_driver_t *)self;
  impl->base.display_buffer = buffer;
  if (impl->hltdc) {
    HAL_LTDC_SetAddress(impl->hltdc, (uint32_t)impl->base.display_buffer, 0);
  }
}

// Driver operations table
static const lcd_driver_ops_t stm32_lcd_ops = {
    .draw_point = stm32_lcd_draw_point,
    .read_point = stm32_lcd_read_point,
    .fill_rect = stm32_lcd_fill_rect,
    .draw_bitmap = stm32_lcd_draw_bitmap,
    .copy_buffer = stm32_lcd_copy_buffer,
    .set_orientation = stm32_lcd_set_orientation,
    .swap_buffer = stm32_lcd_swap_buffer,
    .get_drawbuf = stm32_lcd_get_drawbuf,
    .get_displaybuf = stm32_lcd_get_displaybuf,
    .set_drawbuf = stm32_lcd_set_drawbuf,
    .set_displaybuf = stm32_lcd_set_displaybuf};

lcd_driver_t *stm32_lcd_driver_create(LTDC_HandleTypeDef *hltdc, uint16_t width,
                                      uint16_t height) {
  stm32_lcd_driver_t *driver = (stm32_lcd_driver_t *)sys_malloc(
      SYS_MEM_INTERNAL, sizeof(stm32_lcd_driver_t));

  if (driver) {
    memset(driver, 0, sizeof(stm32_lcd_driver_t));
    driver->base.ops = &stm32_lcd_ops;
    driver->hltdc = hltdc;
    // Set default orientation
    driver->base.orientation = LCD_ORIENTATION_LANDSCAPE;
    driver->base.width = width;
    driver->base.height = height;
  }
  HAL_LTDC_SetWindowSize(hltdc, width, height, LTDC_LAYER_1);

  __HAL_LTDC_ENABLE_IT(hltdc, LTDC_IT_LI); // 确保行中断(LI)已使能
  HAL_LTDC_ProgramLineEvent(hltdc, 0);     // 在第0行触发中断(垂直消隐期开始)

  return (lcd_driver_t *)driver;
}
