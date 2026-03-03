/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable
 * content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "MemPool.h"
#include "Sys.h"
#include "dev_map.h"
#include "lcd_screen_driver.h"
#include "lcd_screen_factory.h"
#include <stdbool.h>

extern lcd_driver_t *lcd; // 来自 test_lcd.c 或 stm32h7xx_it.c 的全局指针

/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES 800
#define MY_DISP_VER_RES 480

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                       lv_color_t *color_p);
static void on_flush_done(void *user_data);

/**********************
 *  STATIC VARIABLES
 **********************/
static lcd_driver_t *lcd_drv = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void) {
  /*-------------------------
   * Initialize your display
   * -----------------------*/
  disp_init();

  /*-----------------------------
   * Create a buffer for drawing
   *----------------------------*/
  static lv_disp_draw_buf_t draw_buf_dsc_1;
  // LVGL 缓冲区放在 SDRAM 中，避免占用过多内部 RAM
  // 申请 800 * 100 像素的缓冲区 (约 160KB)
  uint32_t buf_size = MY_DISP_HOR_RES * 100;
  lv_color_t *buf_1 =
      (lv_color_t *)sys_malloc(SYS_MEM_EXTERNAL, buf_size * sizeof(lv_color_t));

  lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, buf_size);

  /*-----------------------------------
   * Register the display in LVGL
   *----------------------------------*/

  static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
  lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

  /*Set the resolution of the display*/
  disp_drv.hor_res = MY_DISP_HOR_RES;
  disp_drv.ver_res = MY_DISP_VER_RES;

  /*Used to copy the buffer's content to the display*/
  disp_drv.flush_cb = disp_flush;

  /*Set a display buffer*/
  disp_drv.draw_buf = &draw_buf_dsc_1;

  /*Finally register the driver*/
  lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void) {
  lcd_drv = lcd_screen_factory_create(LCD_ID_UI);
  if (lcd_drv) {
    LCD_INIT(lcd_drv);
    LCD_DISPLAY_ON(lcd_drv);
    // 注册填充完成回调，用于通知 LVGL 刷新完毕
    LCD_SET_FILL_CB(lcd_drv, on_flush_done, NULL);
    // 同步到全局指针，供中断 handler 使用
    lcd = lcd_drv;
  }
}

static void on_flush_done(void *user_data) {
  // 获取当前的 display driver 并通知就绪
  lv_disp_t *disp = lv_disp_get_default();
  if (disp) {
    lv_disp_flush_ready(disp->driver);
  }
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called
 * by LVGL
 */
void disp_enable_update(void) { disp_flush_enabled = true; }

/* Disable updating the screen (the flushing process) when disp_flush() is
 * called by LVGL
 */
void disp_disable_update(void) { disp_flush_enabled = false; }

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                       lv_color_t *color_p) {
  if (disp_flush_enabled && lcd_drv) {
    /* 使用 DMA2D 进行区域绘制 (异步模式) */
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    // 注意：LVGL 单缓冲模式刷新时，color_p 是当前绘制好的图像
    // 在 DMA 搬运前，必须将 CPU 写入 Cache 的数据刷入物理内存
//    uint32_t buf_size = w * h * 2; // RGB565 = 2 bytes
//    SCB_CleanDCache_by_Addr((uint32_t *)color_p, buf_size);

    // 我们利用之前实现的 LCD_DRAW_BITMAP 将其搬运到显存
    LCD_DRAW_BITMAP(lcd_drv, area->x1, area->y1, w, h, color_p);

    // 这里不需要调用 lv_disp_flush_ready，因为在 on_flush_done 中断中调
  } else {
    lv_disp_flush_ready(disp_drv);
  }
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
