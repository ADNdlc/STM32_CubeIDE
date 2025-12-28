#include "ui_screen_home.h"
// #include "elog.h"
#include "app_manager.h" // Added missing include
#include "components/util.h"
#include "core_app.h"
#include "ui_helpers.h"
#include <assert.h>
#include <stdio.h>

#define NODEBUG

lv_obj_t *ui_screen_home;
lv_obj_t *ui_home_tileview;

// --- 配置区域 ---
#define PAGE_COUNT 3    // 页面数量
#define PAGE_COL 8      // 每页app列数
#define PAGE_ROW 4      // 每页app行数
#define STATUS_BAR_H 30 // 状态栏高度(网络布局第一行高度)

// 网格描述符 (Buffer)
static lv_coord_t col_dsc[PAGE_COL + 2]; // 需要加上空行和结束符
static lv_coord_t row_dsc[PAGE_ROW + 2];

// 页面追踪
typedef struct {
  lv_obj_t *tile;            // 页面对象(TileView的tile)
  uint8_t current_app_count; // 当前页面上的app数量
} home_page_t;
static home_page_t pages[PAGE_COUNT]; // 缓存页面对象

// --- 辅助函数：布局计算 (Refactored from old ui) ---
static void setup_grid_layout(lv_obj_t *tile) {
  // 清除默认边距
  lv_obj_set_style_pad_all(tile, 0, 0);            // 容器内边距
  lv_obj_set_style_border_width(tile, 0, 0);       // 边框
  lv_obj_set_style_pad_gap(tile, 0, 0);            // 单元格子间距(方便计算布局)
  lv_obj_set_style_bg_opa(tile, LV_OPA_TRANSP, 0); // 背景透明

  lv_coord_t screen_h = scr_act_height();
  lv_coord_t screen_w = scr_act_width();
  lv_coord_t remaining_h = screen_h - row_dsc[0]; // 可用行高(第一行空给状态栏)
  uint8_t col_width = screen_w / PAGE_COL;        // 每列宽度

  col_dsc[PAGE_COL + 1] = LV_GRID_TEMPLATE_LAST; // 结束符
  row_dsc[PAGE_ROW + 1] = LV_GRID_TEMPLATE_LAST;

  // 计算列(列宽)描述符
  for (int i = 0; i < PAGE_COL; i++) { // 按照定义行数平均分
    col_dsc[i] = LV_GRID_FR(1);        // 均分列
  }

  // 计算行(行高)描述符
  if (PAGE_COL > 0) {
    row_dsc[0] = STATUS_BAR_H;
  } // 第一行给状态栏
  if (PAGE_COL > 1) {
    if (remaining_h >= PAGE_ROW * col_width) {
      for (int i = 1; i < PAGE_ROW; i++) {
        assert(remaining_h >= col_width);
        row_dsc[i] = col_width; // 正方形图标
        remaining_h -= col_width;
      }
      row_dsc[PAGE_ROW] = remaining_h; // 最后一行占满剩余
    } else {
      lv_coord_t base_row_h = remaining_h / PAGE_ROW;
      lv_coord_t leftover_h = remaining_h % PAGE_ROW;
      for (int i = 1; i < PAGE_ROW; i++) {
        assert(base_row_h > 0);
        row_dsc[i] = base_row_h; // 均分行高
      }
      row_dsc[PAGE_ROW] = base_row_h + leftover_h; // 最后一行占满剩余
    }
  }

  // 应用布局描述符
  lv_obj_set_grid_dsc_array(tile, col_dsc, row_dsc);
}

// --- 核心初始化 ---
void ui_screen_home_init(void) {
  // 创建屏幕
  ui_screen_home = lv_obj_create(NULL);
  if (!ui_screen_home) {
    LV_LOG_ERROR("Failed to create ui_screen_home");
    return;
  }
  lv_obj_clear_flag(ui_screen_home, LV_OBJ_FLAG_SCROLLABLE);

  // 设置背景
  lv_obj_t *bg = lv_img_create(ui_screen_home);
  // lv_img_set_src(bg, &wallpaper);  // 设置图片源

  // 使用TileView作为主体
  ui_home_tileview = lv_tileview_create(ui_screen_home);
  lv_obj_set_style_bg_opa(ui_home_tileview, LV_OPA_TRANSP, 0);
  // lv_obj_remove_style(ui_home_tileview, NULL, LV_PART_SCROLLBAR); //
  // 移除滚动条

  // 初始化TileView,添加页面并设置布局
  for (int i = 0; i < PAGE_COUNT; i++) {
    pages[i].tile = lv_tileview_add_tile(ui_home_tileview, i, 0,
                                         LV_DIR_LEFT | LV_DIR_RIGHT);
    pages[i].current_app_count = 0;
    setup_grid_layout(pages[i].tile); // 配置网格布局(根据设置计算)
  }

  // 遍历并放置 APP (从 App Manager 获取)
  uint8_t total_apps = app_manager_get_app_count();

  // 创建图标并布局
  for (int i = 0; i < total_apps; i++) {
    const app_def_t *app = app_manager_get_app_by_index(i);

    // Safety Check 1: App Pointer
    if (app == NULL) {
      continue;
    }

    // Safety Check 2: App Name (Check pointer validity first if possible, but
    // start with NULL check)
    if (app->name == NULL) {
      LV_LOG_ERROR("App index %d has NULL name! Skipping.", i);
      continue;
    }

    if (strcmp(app->name, "HOME") == 0)
      continue;

    // 确定页面 (从 App Manager 获取偏好页码)
    int page_idx = app_manager_get_page_index(app);

    // 如果首选页面无效或已满，寻找空位
    int max_apps_per_page = PAGE_ROW * PAGE_COL;

    if (page_idx < 0 || page_idx >= PAGE_COUNT ||
        pages[page_idx].current_app_count >= max_apps_per_page) {

      int found_page = -1;
      // 自动寻找有空位的页面
      for (int p = 0; p < PAGE_COUNT; p++) {
        if (pages[p].current_app_count < max_apps_per_page) {
          found_page = p;
          break;
        }
      }
      page_idx = found_page;
    }

    // 如果所有页面都满了
    if (page_idx < 0) {
      LV_LOG_WARN("Home screen full! Cannot add app: %s", app->name);
      continue;
    }

    // 计算行列
    uint8_t idx_in_page = pages[page_idx].current_app_count;
    uint8_t col = idx_in_page % PAGE_COL;
    uint8_t row = 1 + (idx_in_page / PAGE_COL); // +1 因为第0行是状态栏

    // 双重检查 row 是否越界
    if (row > PAGE_ROW) {
      LV_LOG_WARN("App row out of bounds! %s (Row: %d, Max: %d)", app->name,
                  row, PAGE_ROW);
      continue;
    }

    // 创建组件
    lv_obj_t *icon_comp =
        ui_comp_app_icon_create(pages[page_idx].tile, app->icon, app->name);

    if (icon_comp == NULL) {
      LV_LOG_ERROR("Failed to create icon for %s (OOM?)", app->name);
      continue;
    }

    // 布局组件
    lv_obj_set_grid_cell(icon_comp, LV_GRID_ALIGN_STRETCH, col, 1,
                         LV_GRID_ALIGN_STRETCH, row, 1);

    // 更新组件样式以适应行高
    // 确保 row_dsc 访问安全 (row 在 1..PAGE_ROW 之间)
    if (row <= PAGE_ROW) {
      ui_comp_app_icon_update_layout(icon_comp, row_dsc[row]);
    }

    // 添加事件
    lv_obj_t *img = lv_obj_get_child(icon_comp, 0);
    if (img) {
      lv_obj_add_event_cb(img, ui_event_app_icon_click, LV_EVENT_CLICKED,
                          (void *)app);
    }

    pages[page_idx].current_app_count++;
  }

#ifndef NODEBUG
  // 布局测试
  for (int i = 0; i < PAGE_COUNT; i++)
    test_layout_grid(pages[i].tile, PAGE_ROW + 1, PAGE_COL);
#endif

  // 6. 初始化状态栏 (简略)
  // StatusBar_init(ui_screen_home);
}
