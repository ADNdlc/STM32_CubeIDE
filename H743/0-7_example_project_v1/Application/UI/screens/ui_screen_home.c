#include "ui_screen_home.h"
#include "Act_manager.h"
#include <stdio.h>

lv_obj_t *ui_screen_home;
lv_obj_t *ui_home_tileview;

// --- 配置区域 ---
#define PAGE_COUNT 3
#define PAGE_COL 8
#define PAGE_ROW 5
#define STATUS_BAR_H 30

// 定义 APP 在哪一页 (替代 messy CSV)
typedef struct {
  const char *name;
  uint8_t page_index;
} app_placement_t;

static const app_placement_t APP_PLACEMENTS[] = {
    {"Settings", 0}, {"Control", 0}, {"ColorWheel", 0},
    {"test1", 1},    {"File", 2},
    // 添加更多...
};
#define APP_PLACEMENT_COUNT (sizeof(APP_PLACEMENTS) / sizeof(APP_PLACEMENTS[0]))

// 网格描述符 (Buffer)
static lv_coord_t col_dsc[PAGE_COL + 1];
static lv_coord_t row_dsc[PAGE_ROW + 1];

// 页面追踪
typedef struct {
  lv_obj_t *tile;
  uint8_t current_app_count;
} home_page_t;
static home_page_t pages[PAGE_COUNT];

// --- 辅助函数：布局计算 (Refactored from old ui) ---
static void setup_grid_layout(lv_obj_t *tile) {
  lv_coord_t screen_h = 480; // 假设高度，实际应该 scr_act_height()
  lv_coord_t screen_w = 800; // 假设宽度

  // 简单化的网格计算
  uint8_t middle_rows = PAGE_ROW - 1;
  lv_coord_t col_w = screen_w / PAGE_COL;

  // 列
  for (int i = 0; i < PAGE_COL; i++)
    col_dsc[i] = LV_GRID_FR(1); // 均分
  col_dsc[PAGE_COL] = LV_GRID_TEMPLATE_LAST;

  // 行
  row_dsc[0] = STATUS_BAR_H; // 第一行留给状态栏
  lv_coord_t remain_h = screen_h - STATUS_BAR_H;
  lv_coord_t row_h = remain_h / middle_rows;

  for (int i = 1; i < PAGE_ROW; i++)
    row_dsc[i] = row_h;
  // 补齐像素余数到最后一行
  row_dsc[PAGE_ROW - 1] += (remain_h % middle_rows);
  row_dsc[PAGE_ROW] = LV_GRID_TEMPLATE_LAST;

  lv_obj_set_grid_dsc_array(tile, col_dsc, row_dsc);

  // 清除默认边距
  lv_obj_set_style_pad_all(tile, 0, 0);
  lv_obj_set_style_border_width(tile, 0, 0);
  lv_obj_set_style_bg_opa(tile, LV_OPA_TRANSP, 0);
}

// --- 辅助函数：查找 APP 应该在哪一页 ---
static int get_preferred_page(const char *app_name) {
  for (int i = 0; i < APP_PLACEMENT_COUNT; i++) {
    if (strcmp(APP_PLACEMENTS[i].name, app_name) == 0) {
      return APP_PLACEMENTS[i].page_index;
    }
  }
  return -1; // 自动分配
}

// --- 核心初始化 ---
void ui_screen_home_init(void) {
  // 1. 创建屏幕
  ui_screen_home = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_screen_home, LV_OBJ_FLAG_SCROLLABLE);

  // 2. 创建背景
  lv_obj_t *bg = lv_img_create(ui_screen_home);
  lv_img_set_src(bg, &wallpaper);

  // 3. 创建 TileView
  ui_home_tileview = lv_tileview_create(ui_screen_home);
  lv_obj_set_style_bg_opa(ui_home_tileview, LV_OPA_TRANSP, 0);
  lv_obj_remove_style(ui_home_tileview, NULL, LV_PART_SCROLLBAR);

  // 4. 初始化页面
  for (int i = 0; i < PAGE_COUNT; i++) {
    pages[i].tile = lv_tileview_add_tile(ui_home_tileview, i, 0,
                                         LV_DIR_LEFT | LV_DIR_RIGHT);
    pages[i].current_app_count = 0;
    setup_grid_layout(pages[i].tile);
  }

  // 5. 遍历并放置 APP
  uint8_t total_apps = activity_manager_get_registered_count();

  for (int i = 0; i < total_apps; i++) {
    const app_def_t *app = activity_manager_get_def_by_index(i);
    if (strcmp(app->name, "HOME") == 0)
      continue;

    // 确定页面
    int page_idx = get_preferred_page(app->name);
    if (page_idx < 0 || page_idx >= PAGE_COUNT) {
      // 自动寻找空位
      for (int p = 0; p < PAGE_COUNT; p++) {
        // 简单检查: 假设满页是 (ROW-1)*COL
        if (pages[p].current_app_count < (PAGE_ROW - 1) * PAGE_COL) {
          page_idx = p;
          break;
        }
      }
    }
    if (page_idx < 0)
      page_idx = PAGE_COUNT - 1; // 实在没地儿了放最后一页

    // 计算行列
    uint8_t idx_in_page = pages[page_idx].current_app_count;
    uint8_t col = idx_in_page % PAGE_COL;
    uint8_t row = 1 + (idx_in_page / PAGE_COL); // +1 因为第0行是状态栏

    // 创建组件
    lv_obj_t *icon_comp =
        ui_comp_app_icon_create(pages[page_idx].tile, app->icon, app->name);

    // 布局组件
    lv_obj_set_grid_cell(icon_comp, LV_GRID_ALIGN_STRETCH, col, 1,
                         LV_GRID_ALIGN_STRETCH, row, 1);

    // 更新组件样式以适应行高
    ui_comp_app_icon_update_layout(icon_comp, row_dsc[row]);

    // 添加事件
    // 注意: ui_comp_app_icon_create
    // 返回的是容器，我们需要让内部图片响应点击，或者容器本身响应 在 comp_create
    // 里我们给 img 加了 flag，但最好在这里统一加 event 获取 img 子对象
    // (假设child 0)
    lv_obj_t *img = lv_obj_get_child(icon_comp, 0);
    lv_obj_add_event_cb(img, ui_event_app_icon_click, LV_EVENT_CLICKED,
                        (void *)app);

    pages[page_idx].current_app_count++;
  }

  // 6. 初始化状态栏 (简略)
  // StatusBar_init(ui_screen_home);
}
