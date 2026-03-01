/*
 * stm32_ltdc_driver.c
 *
 *  Created on: Mar 1, 2026
 *      Author: Antigravity
 */

#include "stm32_ltdc_driver.h"
#include "MemPool.h"
#include "Sys.h"
#include <string.h>


#define LTDC_DRV_MEM_SOURCE SYS_MEM_INTERNAL

/* 像素字节跨度 (RGB565 为 2 字节) */
#define PIXEL_SIZE 2

/**
 * @brief 等待 DMA2D 完成
 */
static int stm32_ltdc_wait_until_ready(lcd_driver_t *self) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)self;
  if (!drv->hdma2d)
    return 0;

  while (drv->hdma2d->State == HAL_DMA2D_STATE_BUSY)
    ;
  return 0;
}

/**
 * @brief 初始化
 */
static int stm32_ltdc_init(lcd_driver_t *self) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)self;

  // 核心初始化已由 MX_LTDC_Init 完成
  // 这里主要确保显存地址正确挂载
  if (drv->hltdc && drv->frame_buffer) {
    HAL_LTDC_SetAddress(drv->hltdc, drv->frame_buffer, 0);
  }

  return 0;
}

/**
 * @brief 绘制/填充色块 (使用 DMA2D 硬件加速)
 */
static int stm32_ltdc_fill(lcd_driver_t *self, uint16_t sx, uint16_t sy,
                           uint16_t ex, uint16_t ey, uint32_t color) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)self;
  if (sx > ex || sy > ey)
    return -1;

  uint16_t width = ex - sx + 1;
  uint16_t height = ey - sy + 1;

  // 计算目标起始物理地址
  uint32_t dest_addr =
      drv->frame_buffer + (sy * self->info.width + sx) * PIXEL_SIZE;

  stm32_ltdc_wait_until_ready(self);

  // 使用 DMA2D R2M (Register to Memory) 模式填充纯色
  if (HAL_DMA2D_Start(drv->hdma2d, color, dest_addr, width, height) != HAL_OK) {
    return -2;
  }

  return 0;
}

/**
 * @brief 画面缓冲区填充 (对接 LVGL 刷新核心)
 */
static int stm32_ltdc_color_fill(lcd_driver_t *self, uint16_t sx, uint16_t sy,
                                 uint16_t ex, uint16_t ey,
                                 const void *color_p) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)self;
  if (sx > ex || sy > ey || !color_p)
    return -1;

  uint16_t width = ex - sx + 1;
  uint16_t height = ey - sy + 1;

  // 计算目标起始物理地址
  uint32_t dest_addr =
      drv->frame_buffer + (sy * self->info.width + sx) * PIXEL_SIZE;

  stm32_ltdc_wait_until_ready(self);

  // 配置 DMA2D M2M (Memory to Memory) 模式，并设置 Offset
  drv->hdma2d->Init.Mode = DMA2D_M2M;
  drv->hdma2d->Init.OutputOffset =
      self->info.width - width; // 每一行结束后跳过多少像素到下一行

  // 注意：如果输入源也是块，需要设置 Foreground Offset，但通常 LVGL 传入的
  // color_p 是紧凑数组
  drv->hdma2d->LayerCfg[1].InputOffset = 0;

  if (HAL_DMA2D_Init(drv->hdma2d) != HAL_OK)
    return -3;
  if (HAL_DMA2D_ConfigLayer(drv->hdma2d, 1) != HAL_OK)
    return -4;

  // 启动传输
  if (HAL_DMA2D_Start(drv->hdma2d, (uint32_t)color_p, dest_addr, width,
                      height) != HAL_OK) {
    return -2;
  }

  return 0;
}

/**
 * @brief 绘制像素
 */
static int stm32_ltdc_draw_point(lcd_driver_t *self, uint16_t x, uint16_t y,
                                 uint32_t color) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)self;
  if (x >= self->info.width || y >= self->info.height)
    return -1;

  uint16_t *buf = (uint16_t *)drv->frame_buffer;
  buf[y * self->info.width + x] = (uint16_t)color;
  return 0;
}

/**
 * @brief 读取像素
 */
static uint32_t stm32_ltdc_read_point(lcd_driver_t *self, uint16_t x,
                                      uint16_t y) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)self;
  if (x >= self->info.width || y >= self->info.height)
    return 0;

  uint16_t *buf = (uint16_t *)drv->frame_buffer;
  return buf[y * self->info.width + x];
}

static int stm32_ltdc_display_on(lcd_driver_t *self) {
  __HAL_LTDC_ENABLE(((stm32_ltdc_driver_t *)self)->hltdc);
  return 0;
}

static int stm32_ltdc_display_off(lcd_driver_t *self) {
  __HAL_LTDC_DISABLE(((stm32_ltdc_driver_t *)self)->hltdc);
  return 0;
}

static const lcd_driver_ops_t stm32_ltdc_ops = {
    .init = stm32_ltdc_init,
    .draw_point = stm32_ltdc_draw_point,
    .read_point = stm32_ltdc_read_point,
    .fill = stm32_ltdc_fill,
    .color_fill = stm32_ltdc_color_fill,
    .display_on = stm32_ltdc_display_on,
    .display_off = stm32_ltdc_display_off,
    .wait_until_ready = stm32_ltdc_wait_until_ready,
};

lcd_driver_t *stm32_ltdc_driver_create(LTDC_HandleTypeDef *hltdc,
                                       DMA2D_HandleTypeDef *hdma2d,
                                       uint32_t fb_addr) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)sys_malloc(
      LTDC_DRV_MEM_SOURCE, sizeof(stm32_ltdc_driver_t));
  if (!drv)
    return NULL;

  memset(drv, 0, sizeof(stm32_ltdc_driver_t));
  drv->base.ops = &stm32_ltdc_ops;
  drv->hltdc = hltdc;
  drv->hdma2d = hdma2d;
  drv->frame_buffer = fb_addr;

  // 默认 800x480 RGB565 (基于 MX_LTDC_Init 配置)
  drv->base.info.width = 800;
  drv->base.info.height = 480;
  drv->base.info.format = LCD_COLOR_RGB565;

  return (lcd_driver_t *)drv;
}
