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
#include <stdbool.h>

// 移植依赖
#include "device_mapping.h"
#include "elog.h"
#include "lcd_factory.h"
#include "lcd_hal/lcd_hal.h"
#include "sys.h"
#include "tim.h"
extern void lvgl_handler_set_pending_swap(void);

/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES 800 // 实际宽度
#define MY_DISP_VER_RES 480 // 实际高
#define SIZEOF_PIXEL sizeof(uint16_t)
#define LVGL_BUFFER_SIZE (MY_DISP_HOR_RES * MY_DISP_VER_RES * SIZEOF_PIXEL)

#ifndef MY_DISP_HOR_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
#define MY_DISP_HOR_RES 320
#endif

#ifndef MY_DISP_VER_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
#define MY_DISP_VER_RES 240
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                       lv_color_t *color_p);
// static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf,
// lv_coord_t dest_width,
//         const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_disp_drv_t *it_disp_drv; // 用于在中断中访问
// lcd_hal相关
static lcd_driver_t *lcd_driver = NULL;
lcd_hal_t *lvgl_display = NULL;
static uint16_t *lvgl_displaybuf = NULL;
static uint16_t *lvgl_drawbuf = NULL;

/**********************
 *      MACROS
 **********************/
#define BufferConfig 1        // 使用哪一种缓冲配置
#define Always_Whol_Redrawn 0 // 总是全屏重绘

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void) {
  /*-------------------------
   * Initialize your display
   * -----------------------*/
  HAL_TIM_Base_Start_IT(&htim6); // 开启lvgl时基

  log_i("lvgl display init start...");
  disp_init();
  log_i("lvgl display init success...");
  /*-----------------------------
   * Create a buffer for drawing
   *----------------------------*/

  /**
   * LVGL requires a buffer where it internally draws the widgets.
   * Later this buffer will passed to your display driver's `flush_cb` to copy
   * its content to your display. The buffer has to be greater than 1 display
   * row
   *
   * There are 3 buffering configurations:
   * 1. Create ONE buffer:
   *      LVGL will draw the display's content here and writes it to your
   * display
   *
   * 2. Create TWO buffer:
   *      LVGL will draw the display's content to a buffer and writes it your
   * display. You should use DMA to write the buffer's content to the display.
   *      It will enable LVGL to draw the next part of the screen to the other
   * buffer while the data is being sent form the first buffer. It makes
   * rendering and flushing parallel.
   *
   * 3. Double buffering
   *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
   *      This way LVGL will always provide the whole rendered screen in
   * `flush_cb` and you only need to change the frame buffer's address.
   */

#if BufferConfig == 1
  /* Example for 1) */
  static lv_disp_draw_buf_t draw_buf_dsc_1;
  static lv_color_t buf_1[MY_DISP_HOR_RES * 240]; // 内部缓冲区 = 384K
  lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL,
                        MY_DISP_HOR_RES *
                            240); /*Initialize the display buffer*/
#endif
#if BufferConfig == 2
  /* Example for 2) */
  static lv_disp_draw_buf_t draw_buf_dsc_2;
  static lv_color_t buf_2_1[MY_DISP_HOR_RES * 10]; /*A buffer for 10 rows*/
  static lv_color_t
      buf_2_2[MY_DISP_HOR_RES * 10]; /*An other buffer for 10 rows*/
  lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2,
                        MY_DISP_HOR_RES * 10); /*Initialize the display buffer*/
#endif
#if BufferConfig == 3
  /* Example for 3) also set disp_drv.full_refresh = 1 below*/
  static lv_disp_draw_buf_t draw_buf_dsc_3;
  static lv_color_t
      buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES]; /*A screen sized buffer*/
  static lv_color_t buf_3_2[MY_DISP_HOR_RES *
                            MY_DISP_VER_RES]; /*Another screen sized buffer*/
  lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2,
                        MY_DISP_VER_RES *
                            LV_VER_RES_MAX); /*Initialize the display buffer*/
#endif
  /*-----------------------------------
   * Register the display in LVGL
   *----------------------------------*/

  static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
  lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

  /*Set up the functions to access to your display*/

  /*Set the resolution of the display*/
  disp_drv.hor_res = MY_DISP_HOR_RES;
  disp_drv.ver_res = MY_DISP_VER_RES;

  /*Used to copy the buffer's content to the display*/
  disp_drv.flush_cb = disp_flush;

  /*Set a display buffer*/
#if BufferConfig == 1
  disp_drv.draw_buf = &draw_buf_dsc_1;
#endif
#if BufferConfig == 2
  disp_drv.draw_buf = &draw_buf_dsc_2;
#endif
#if BufferConfig == 3
  disp_drv.draw_buf = &draw_buf_dsc_3;
#endif

  /*Required for Example 3)*/
#if Always_Whol_Redrawn
  disp_drv.full_refresh = 1; // 总是重绘整个屏幕
#endif

  /* Fill a memory array with a color if you have GPU.
   * Note that, in lv_conf.h you can enable GPUs that has built-in support in
   * LVGL. But if you have a different GPU you can use with this callback.*/
  // disp_drv.gpu_fill_cb = gpu_fill;

  /*Finally register the driver*/
  it_disp_drv = &disp_drv; // 保存驱动描述符的地址
  lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void) {
  /*You code here*/
  // 初始化lcd相关
  lcd_driver = lcd_driver_get(LCD_MAIN, 800, 480); // 获取平台驱动
  if (!lcd_driver) {
    log_e("Failed to get LCD driver instance");
  }
  lvgl_drawbuf = (uint16_t *)sys_malloc(SYS_MEM_EXTERNAL, LVGL_BUFFER_SIZE);
  lvgl_displaybuf = (uint16_t *)sys_malloc(SYS_MEM_EXTERNAL, LVGL_BUFFER_SIZE);
  if (!lvgl_drawbuf || !lvgl_displaybuf) {
    log_e("Failed to allocate memory for LCD buffer");
  }
  lvgl_display = lcd_hal_create(lcd_driver, lvgl_drawbuf, lvgl_displaybuf);
  if (!lvgl_display) {
    log_e("Failed to create LCD HAL instance");
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

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the
 * background but 'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                       lv_color_t *color_p) {
  if (disp_flush_enabled) {
    uint32_t x = area->x2 - area->x1 + 1;
    uint32_t y = area->y2 - area->y1 + 1;
    uint32_t offset = (area->y1 * disp_drv->hor_res + area->x1) * 2;

    /* 获取当前双缓冲区的物理地址 */
    uint16_t *draw_buf = lcd_hal_get_drawbuf(lvgl_display);
    uint16_t *disp_buf = lcd_hal_get_displaybuf(lvgl_display);

    /*
     * 策略：同步双路拷贝
     * 1. 拷贝到当前的绘图缓冲区 (draw_buffer)
     * 2. 拷贝到当前的显示缓冲区 (display_buffer)
     * 确保两边内容在更新区域上保持同步。
     */

    // 拷贝到 draw_buffer
    lcd_hal_copy_buffer(lvgl_display, (void *)((size_t)draw_buf + offset),
                        (void *)color_p, x, y, 0, (disp_drv->hor_res - x),
                        LTDC_PIXEL_FORMAT_RGB565);

    // 拷贝到 display_buffer
    lcd_hal_copy_buffer(lvgl_display, (void *)((size_t)disp_buf + offset),
                        (void *)color_p, x, y, 0, (disp_drv->hor_res - x),
                        LTDC_PIXEL_FORMAT_RGB565);

    /* 标记内容已就绪，在下一个 Vsync 触发时交换显示地址 */
    lvgl_handler_set_pending_swap();
  }

  /* 通知 LVGL 刷新完成 (同步模式) */
  lv_disp_flush_ready(disp_drv);
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a
 * memory with a color*/
// static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf,
// lv_coord_t dest_width,
//                     const lv_area_t * fill_area, lv_color_t color)
//{
//     /*It's an example code which should be done by your GPU*/
//     int32_t x, y;
//     dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//     for(y = fill_area->y1; y <= fill_area->y2; y++) {
//         for(x = fill_area->x1; x <= fill_area->x2; x++) {
//             dest_buf[x] = color;
//         }
//         dest_buf+=dest_width;    /*Go to the next line*/
//     }
// }

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
