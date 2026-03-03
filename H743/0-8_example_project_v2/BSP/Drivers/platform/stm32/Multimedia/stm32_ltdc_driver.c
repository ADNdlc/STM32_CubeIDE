/*
 * stm32_ltdc_driver.c
 *
 *  Created on: Mar 1, 2026
 *      Author: Antigravity
 */

#include "stm32_ltdc_driver.h"
#include "MemPool.h"
#include "Sys.h"
#include "gpio_factory.h" // 背光控制
#include <string.h>

#define LTDC_DRV_MEM_SOURCE SYS_MEM_INTERNAL

/* ================= 辅助函数 ================= */

#define DMA2D_WAIT_IDLE()                                                      \
  while ((DMA2D->CR & DMA2D_CR_START) != 0) {                                  \
  }

// 供 stm32xx_it.c 中的 DMA2D_IRQHandler 调用
void stm32_lcd_dma2d_irq_handler(lcd_driver_t *self) {
  // 检查是否是传输完成中断 (TCIF)
  if ((DMA2D->ISR & DMA2D_ISR_TCIF) != 0) {
    // 清除标志位
    DMA2D->IFCR = DMA2D_IFCR_CTCIF;

    // 触发应用层回调 (通知 LVGL 绘制完成)
    if (self->fill_done_cb != NULL) {
      self->fill_done_cb(self->fill_cb_data);
    }
  }
}

// 供 stm32xx_it.c 中的 LTDC_IRQHandler 调用
void stm32_lcd_ltdc_irq_handler(lcd_driver_t *self) {
  stm32_ltdc_driver_t *impl = (stm32_ltdc_driver_t *)self;

  // 检查是否是寄存器重载完成中断 (Register Reload / VSYNC 切换完成)
  if (__HAL_LTDC_GET_FLAG(impl->hltdc, LTDC_FLAG_RR) != RESET) {
    // 清除标志位并关闭该中断 (按需触发)
    __HAL_LTDC_CLEAR_FLAG(impl->hltdc, LTDC_FLAG_RR);
    __HAL_LTDC_DISABLE_IT(impl->hltdc, LTDC_IT_RR);

    // 触发应用层回调 (通知 LVGL 交换完毕，释放旧缓冲区)
    if (self->swap_done_cb != NULL) {
      self->swap_done_cb(self->swap_cb_data);
    }
  }
}

/* ================= 接口实现 ================= */

static int stm32_lcd_init(lcd_driver_t *self) {
  stm32_ltdc_driver_t *impl = (stm32_ltdc_driver_t *)self;

  // 配置 LTDC 层地址
  HAL_LTDC_SetAddress(impl->hltdc, (uint32_t)self->info.buffer_addr,
                      impl->layer_index);

  // 初始化背光 GPIO
  gpio_driver_t *bl = gpio_driver_get(impl->bl_gpio_id);
  if (bl) {
    GPIO_SET_MODE(bl, GPIO_PushPullOutput);
    GPIO_WRITE(bl, 1); // 默认打开
  }

  return 0;
}

static int stm32_lcd_set_buffer(lcd_driver_t *self, void *buf1, void *buf2) {
  stm32_ltdc_driver_t *impl = (stm32_ltdc_driver_t *)self;
  if (buf1 == NULL || buf2 == NULL) {
    buf1 ? (self->info.buffer_addr = buf1, self->info.back_buffer = buf1)
         : (self->info.buffer_addr = buf2, self->info.back_buffer = buf2);
  } else {
    self->info.buffer_addr = buf1;
    self->info.back_buffer = buf2;
  }

  // 立即更新硬件层地址
  HAL_LTDC_SetAddress(impl->hltdc, (uint32_t)self->info.buffer_addr,
                      impl->layer_index);
  return 0;
}

static int stm32_lcd_set_dir(lcd_driver_t *self, disp_direction_t direction) {
  // 对于 LTDC，硬件旋转通常比较复杂，这里只修改软件逻辑
  // 如果需要硬件旋转，需要 DMA2D 的复杂配置或修改扫描方向
  self->info.dir = direction;
  // 交换宽高的逻辑应在应用层或 draw_point 中处理
  return 0;
}

static int stm32_lcd_display_on(lcd_driver_t *self) {
  stm32_ltdc_driver_t *impl = (stm32_ltdc_driver_t *)self;
  __HAL_LTDC_ENABLE(impl->hltdc);

  gpio_driver_t *bl = gpio_driver_get(impl->bl_gpio_id);
  if (bl) {
    GPIO_WRITE(bl, 1);
  }
  return 0;
}

static int stm32_lcd_display_off(lcd_driver_t *self) {
  stm32_ltdc_driver_t *impl = (stm32_ltdc_driver_t *)self;

  gpio_driver_t *bl = gpio_driver_get(impl->bl_gpio_id);
  if (bl) {
    GPIO_WRITE(bl, 0);
  }

  __HAL_LTDC_DISABLE(impl->hltdc);
  return 0;
}

static uint32_t stm32_lcd_read_point(lcd_driver_t *self, uint16_t x,
                                     uint16_t y) {
  // 简单实现：CPU 直接读取显存
  // 注意：如果是 H7，可能需要处理 Cache 一致性 (SCB_InvalidateDCache_by_Addr)
  uint8_t bytes = get_pixel_bytes(self->info.format);
  uint32_t offset = (y * self->info.width + x) * bytes;
  uint32_t addr = (uint32_t)self->info.back_buffer + offset;

  if (bytes == 2) {
    return *(uint16_t *)addr;
  } else {
    return *(uint32_t *)addr;
  }
}

static int stm32_lcd_draw_point(lcd_driver_t *self, uint16_t x, uint16_t y,
                                uint32_t color) {
  if (x >= self->info.width || y >= self->info.height)
    return -1;

  uint8_t bytes = get_pixel_bytes(self->info.format);
  uint32_t offset = (y * self->info.width + x) * bytes;
  uint32_t addr = (uint32_t)self->info.back_buffer + offset;

  // 直接写内存
  if (bytes == 2) {
    *(volatile uint16_t *)addr = (uint16_t)color;
  } else {
    *(volatile uint32_t *)addr = color;
  }
  return 0;
}

/* 使用 DMA2D 进行纯色填充 */
static int stm32_lcd_fill_rect(lcd_driver_t *self, uint16_t x, uint16_t y,
                               uint16_t w, uint16_t h, uint32_t color) {
  // 1. 计算目标地址
  uint8_t bytes = (self->info.format == LCD_PIXEL_RGB565) ? 2 : 4;
  uint32_t dest_addr =
      (uint32_t)self->info.back_buffer + (y * self->info.width + x) * bytes;

  // 2. 确保上一次传输已完成
  DMA2D_WAIT_IDLE();

  // 3. 配置模式：寄存器到内存 (R2M = 0x03)
  DMA2D->CR = 0x00030000UL;

  // 4. 配置输出颜色
  DMA2D->OCOLR = color;

  // 5. 配置输出地址
  DMA2D->OMAR = dest_addr;

  // 6. 配置输出偏移 (Output Offset)
  // 偏移量 = 总画布宽度 - 填充窗口宽度
  DMA2D->OOR = self->info.width - w;

  // 7. 配置输出格式 (假设 info.format 已经映射好，或者需要 switch 转换)
  // RGB565 = 2, ARGB8888 = 0, RGB888 = 1
  uint32_t pixel_fmt = (self->info.format == LCD_PIXEL_RGB565) ? 2 : 0;
  DMA2D->OPFCCR = pixel_fmt;

  // 8. 配置高(PL) 和 宽(NLR)
  // 高度在用于高16位，像素宽度在低16位
  DMA2D->NLR = (w << 16) | (h & 0xFFFF);

  // 9. 判断是否为异步模式
  if (self->fill_done_cb != NULL) {
    // 异步模式：开启 DMA2D 传输完成中断 (TCIE)
    DMA2D->CR |= DMA2D_CR_TCIE;
    DMA2D->CR |= DMA2D_CR_START; // 启动
    // 直接返回，绝不死等
    return 0;
  } else {
    // 阻塞模式：关闭中断，死等
    DMA2D->CR &= ~DMA2D_CR_TCIE;
    DMA2D->CR |= DMA2D_CR_START;
    DMA2D_WAIT_IDLE();
    return 0;
  }
}

/* 使用 DMA2D 进行图片搬运 (高性能) */
static int stm32_lcd_draw_bitmap(lcd_driver_t *self, uint16_t x, uint16_t y,
                                 uint16_t w, uint16_t h, const void *bitmap) {
  uint8_t bytes = (self->info.format == LCD_PIXEL_RGB565) ? 2 : 4;
  uint32_t dest_addr =
      (uint32_t)self->info.back_buffer + (y * self->info.width + x) * bytes;

  DMA2D_WAIT_IDLE();

  /* --- 配置前景层 (源数据) --- */
  // 设定源地址
  DMA2D->FGMAR = (uint32_t)bitmap;

  // 设定源偏移 (Source Offset)
  // 如果是拷贝整张小图，源偏移通常为 0 (连续读取)
  // 如果是从大图里抠图，这里需要计算
  DMA2D->FGOR = 0;

  // 设定前景格式 (假设源图片格式和屏幕格式一致，否则需要 PFC)
  uint32_t pixel_fmt = (self->info.format == LCD_PIXEL_RGB565) ? 2 : 0;
  DMA2D->FGPFCCR = pixel_fmt; // 设置前景层格式

  /* --- 配置输出层 (目标显存) --- */
  DMA2D->OMAR = dest_addr;
  DMA2D->OOR = self->info.width - w; // 关键：跳过屏幕右侧剩余部分换行
  DMA2D->OPFCCR = pixel_fmt;

  /* --- 配置通用控制 --- */
  DMA2D->NLR = (w << 16) | (h & 0xFFFF);

  // 模式：内存到内存 (M2M = 0x00)
  DMA2D->CR = 0x00000000UL;

  // 9. 判断是否为异步模式
  if (self->fill_done_cb != NULL) {
    // 异步模式：开启 DMA2D 传输完成中断 (TCIE)
    DMA2D->CR |= DMA2D_CR_TCIE;
    DMA2D->CR |= DMA2D_CR_START; // 启动
    // 直接返回
    return 0;
  } else {
    // 阻塞模式：关闭中断，死等
    DMA2D->CR &= ~DMA2D_CR_TCIE;
    DMA2D->CR |= DMA2D_CR_START;
    DMA2D_WAIT_IDLE();
    return 0;
  }
}

static void *stm32_lcd_get_act_buffer(lcd_driver_t *self) {
  return self->info.buffer_addr;
}

static void *stm32_lcd_get_back_buffer(lcd_driver_t *self) {
  return self->info.back_buffer;
}

static disp_direction_t stm32_lcd_get_act_dir(lcd_driver_t *self) {
  return self->info.dir;
}

static int stm32_lcd_swap_buffer(lcd_driver_t *self) {
  if (self->info.buffer_addr == NULL || self->info.back_buffer == NULL) {
    return -1; // 缺少缓冲区
  }
  stm32_ltdc_driver_t *impl = (stm32_ltdc_driver_t *)self;

  // 1. 确保 DMA2D 绘图已完成
  DMA2D_WAIT_IDLE();

  // 2. 维护 Cache 一致性 (H7 必须将 Cache 数据刷入物理显存)
  // 如果 SDRAM 区配置为 Non-cacheable，则不需要此步。
  // uint32_t fb_size =
  //     self->info.width * self->info.height *
  //     get_pixel_bytes(self->info.format);
  // SCB_CleanDCache_by_Addr((uint32_t *)self->info.back_buffer, fb_size);

  // 3. 交换内部指针
  void *tmp = self->info.buffer_addr;
  self->info.buffer_addr = self->info.back_buffer;
  self->info.back_buffer = tmp;

  // 4. 申请硬件地址重载 (使用 NoReload 配合垂直消隐重载)
  HAL_LTDC_SetAddress_NoReload(impl->hltdc, (uint32_t)self->info.buffer_addr,
                               impl->layer_index);

  // 5. 配置为垂直消隐期生效 (VSYNC reload)
  HAL_LTDC_Reload(impl->hltdc, LTDC_RELOAD_VERTICAL_BLANKING);

  return 0;
}

static int stm32_lcd_wait_swap(lcd_driver_t *self) {
  // 如果已经配置了异步回调，说明无需 CPU 阻塞死等，直接返回
  if (self->swap_done_cb != NULL) {
    return 0;
  }

  // 只有未配置回调时，才走传统的 CPU 死等轮询
  stm32_ltdc_driver_t *impl = (stm32_ltdc_driver_t *)self;
  while ((impl->hltdc->Instance->SRCR & LTDC_SRCR_VBR) != 0) { }
  return 0;
}

static int stm32_lcd_set_swap_cb(lcd_driver_t *self, lcd_async_cb_t cb, void *user_data) {
  self->swap_done_cb = cb;
  self->swap_cb_data = user_data;
  return 0;
}

static int stm32_lcd_set_fill_cb(lcd_driver_t *self, lcd_async_cb_t cb, void *user_data) {
  self->fill_done_cb = cb;
  self->fill_cb_data = user_data;
  return 0;
}

/* ================= 构造函数 ================= */

static const lcd_driver_ops_t stm32_ltdc_ops = {
    .init = stm32_lcd_init,
    .set_buffer = stm32_lcd_set_buffer,
    .set_dir = stm32_lcd_set_dir,
    .display_on = stm32_lcd_display_on,
    .display_off = stm32_lcd_display_off,
    .read_point = stm32_lcd_read_point,
    .draw_point = stm32_lcd_draw_point,
    .fill_rect = stm32_lcd_fill_rect,
    .draw_bitmap = stm32_lcd_draw_bitmap,
    .get_act_buffer = stm32_lcd_get_act_buffer,
    .get_back_buffer = stm32_lcd_get_back_buffer,
    .get_act_dir = stm32_lcd_get_act_dir,
    .swap_buffer = stm32_lcd_swap_buffer,
    .wait_swap = stm32_lcd_wait_swap,
	.set_swap_cb = stm32_lcd_set_swap_cb,
	.set_fill_cb = stm32_lcd_set_fill_cb,
};

lcd_driver_t *stm32_ltdc_driver_create(stm32_ltdc_config_t *congfig,
                                       lcd_screen_info_t info) {
  stm32_ltdc_driver_t *drv = (stm32_ltdc_driver_t *)sys_malloc(
      LTDC_DRV_MEM_SOURCE, sizeof(stm32_ltdc_driver_t));
  if (!drv)
    return NULL;

  memset(drv, 0, sizeof(stm32_ltdc_driver_t));

  // 初始化虚函数表
  drv->base.ops = &stm32_ltdc_ops;

  // 初始化基本信息 (因为 info 是结构体了，直接赋值拷贝)
  drv->base.info = info;
  stm32_lcd_set_buffer((lcd_driver_t *)drv, drv->base.info.buffer_addr,
                       drv->base.info.back_buffer);

  // 初始化派生类私有数据
  drv->hltdc = congfig->hltdc;
  drv->hdma2d = congfig->hdma2d;
  drv->layer_index = congfig->layer;
  drv->bl_gpio_id = congfig->bl_gpio_id;

  return (lcd_driver_t *)drv;
}
