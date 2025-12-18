#include "Home.h"
#include "util/util.h"

#ifdef NDEBUG
#define app
#endif
#ifndef NDEBUG

/* 测试选项 */
//#define layout_fill   //布局测试，填充测试图标(不影响app)
#define app           //加载应用列表
//#define opa_COVER     //图标容器不透明
#endif

LV_IMG_DECLARE(icon_home);//图标声明
LV_IMG_DECLARE(wallpaper);//壁纸

/* ========================================== 界面定义与设置,私有变量 ============================================= */

/* --------------- 状态栏 ----------------- */
#define StatusBar_data_height 30          //状态栏高度
#define StatusBar_data_opa  LV_OPA_50    //状态栏透明度

typedef struct {
uint8_t Battery_level;  //电量0~100 
bool Wifi_connect;      //WiFi连接
uint16_t time;          //时间
} StatusBar_data_def; //状态栏数据定义
static StatusBar_data_def StatusBar_data; //储存状态栏数据
//状态栏图标对象
static lv_obj_t* Status_bar;            //状态栏对象
static lv_obj_t* time_label;            //时间标签对象
static lv_obj_t* wifi_icon;             //WiFi图标对象
static lv_obj_t* battery_icon;          //电池图标对象
static lv_obj_t* battery_label;         //电池百分比标签


/* --------------- 页面 ----------------- */
#define page_count      3   //页面数量
#define page_col        8   //列数,MAX=10,MIN=4
#define page_row        5   //行数,MAX=7 ,MIN=3
//(8,5)F=14/(4,3)=30
typedef struct {
    lv_obj_t* tile;//页面
    uint8_t app_count;
} page_def; //页面定义
page_def page[page_count];/* 创建页面列表 */

/**
 * @brief 将APP的名字写到对应行,图标就排列到对应的页
 */
const char* APP_in_page[page_count] = {
#if page_count >= 1//放在第一页的APP
    "null,Settings,Contol,ColorWheel"
#endif
#if page_count >= 2//放在第二页的APP
    ,"null,test1"
#endif
#if page_count >= 3
    ,"mull,File"
#endif
#if page_count >= 4
    ,"mull,"
#endif
#if page_count >= 5
    ,"mull,"
#endif
#if page_count >= 6
    ,"mull,"
#endif
};

//最大和最小值因屏幕大小而异，这里的值仅为800x480大小下比较协调的取值
#define LV_GRID_MAX_COL 11  //MAX=10
#define LV_GRID_MAX_ROW 8   //MAX=7
static lv_obj_t* home_wallpaper;
// Grid descriptors（网格描述符）, 定义为最大可能的大小，由函数动态填充
static lv_coord_t col_dsc[LV_GRID_MAX_COL];//列宽
static lv_coord_t row_dsc[LV_GRID_MAX_ROW];//行高

/* ========================================= 私有函数模块内部调用 =============================================== */
//回调声明
static void icon_click_event_cb(lv_event_t* e);


/**
 * @brief 检查一个app_name是否存在于一个逗号分隔的page_list字符串中
 * @param app_name      要查找的应用名称
 * @param page_list     包含应用名称列表的字符串
 * @return true (找到), false (未找到)
 */
static bool is_app_in_page_list(const char* app_name, const char* page_list) {
    if (!app_name || !page_list) {
        return false;
    }
    const char* p = page_list;
    size_t app_len = strlen(app_name);
    while ((p = strstr(p, app_name)) != NULL) {
        // 检查找到的子串是否是一个完整的单词
        // 条件1: 前一个字符是列表开头或者逗号
        bool pre_ok = (p == page_list || *(p - 1) == ',');
        // 条件2: 后一个字符是列表结尾或者逗号
        bool post_ok = (*(p + app_len) == '\0' || *(p + app_len) == ',');
        if (pre_ok && post_ok) {
            return true;
        }
        p++;// 如果不是完整匹配，则从下一个字符继续搜索
    }
    return false;
}

/**
 * @brief       根据设置计算网络布局
 * @param tile  页面列表
 */
static void home_setup_grid_layout(lv_obj_t* tile) {
    //断言:
    assert(page_col<=LV_GRID_MAX_COL-1 && page_col>=4);//留出一位放结束符
    assert(page_row<=LV_GRID_MAX_ROW-1 && page_row>=3);//留出一位放结束符

    //中间变量
    lv_coord_t screen_h = scr_act_height();                 //屏幕高
    lv_coord_t remaining_h = screen_h - row_dsc[0];         //剩余行高
    lv_coord_t middle_rows_count = page_row - 1;            //应用行数(去掉状态栏)
    uint8_t col_width = scr_act_width() / page_col;         //列宽
    // ----------------------计算列描述符------------------------ //
    for (int i = 0; i < page_col; i++) {    //按照定义行数平均分
        col_dsc[i] = LV_GRID_FR(1);
    }
    col_dsc[page_col] = LV_GRID_TEMPLATE_LAST;
    //---------------------- 计算行描述符 ------------------------//
    if (page_row > 0) {row_dsc[0] = StatusBar_data_height;}// 第一行(状态栏)固定高度
    if (page_row > 1) {
        if (remaining_h >= middle_rows_count * col_width) {//够方块分行
            for (int i = 1; i < page_row-1; i++) {//分配到倒数第二行(page_row-2)
                assert(remaining_h >= col_width);
                row_dsc[i] = col_width;
                remaining_h -= col_width;
            }
            assert(remaining_h >= col_width);
            row_dsc[page_row - 1] = remaining_h;//最后一行分到剩下的
        }
        else//不够按方块分行,按行数平均分
        {
            lv_coord_t base_row_h = remaining_h / middle_rows_count;
            lv_coord_t leftover_h = remaining_h % middle_rows_count;
            for (int i = 1; i < page_row; i++) {
                row_dsc[i] = base_row_h;
            }
            // 把剩余的像素加到最后一行
            if (page_row > 1) {
                row_dsc[page_row-1] += leftover_h;
            }
        }
    }
    row_dsc[page_row] = LV_GRID_TEMPLATE_LAST;//结束
    // 应用布局描述符
    lv_obj_set_grid_dsc_array(tile, col_dsc, row_dsc);
    // 移除容器默认样式
    lv_obj_set_style_pad_all(tile, 0, 0);//容器内边距
    lv_obj_set_style_border_width(tile, 0, LV_PART_MAIN);//边框
    lv_obj_set_style_pad_gap(tile, 0, 0);//单元格子间距
    lv_obj_set_style_bg_opa(tile, LV_OPA_TRANSP, 0);//LV_OPA_COVER，LV_OPA_TRANSP
}

/**
 * @brief 根据行高设置字体
 * @param lv_font_t* 返回一个字体地址
 */
static lv_font_t* setup_app_name_font(lv_coord_t icon_height) {
    if (icon_height < 85) {
        return &lv_font_montserrat_12;
    }
    else if (icon_height < 115) {
        return &lv_font_montserrat_14;
    }
    else if (icon_height < 140) {
        return &lv_font_montserrat_18;
    }
    else if (icon_height < 170) {
        return &lv_font_montserrat_22;
    }
    else if (icon_height < 200) {
        return &lv_font_montserrat_24;
    }
    else if (icon_height < 250) {
        return &lv_font_montserrat_26;
    }
    else {
        return &lv_font_montserrat_30;
    }
}

/**
 * @brief 为app创建一个容器并加载它的图标和名称，然后将这个容器添加到传入页面的布局当中.
 * @param app_def       app
 * @param parent_tile   添加到的页面
 * @param row_idx       行号
 * @param col_idx       列号
 */
static void create_icon_widget(const app_def_t* app_def, lv_obj_t* parent_tile, uint8_t row_idx, uint8_t col_idx) {
    if (row_idx < page_row) { // 确保不超过最大行数
        //创建容器
        lv_obj_t* icon_widget = lv_obj_create(parent_tile);        //以此页为父创建图标容器
        lv_obj_clear_flag(icon_widget, LV_OBJ_FLAG_SCROLLABLE);     //禁用滚动
#ifndef opa_COVER
        lv_obj_set_style_bg_opa(icon_widget, LV_OPA_TRANSP, 0);   //图标容器透明
#endif
        lv_obj_set_style_border_width(icon_widget, 0, 0);           //去除边框
        lv_obj_set_style_pad_all(icon_widget, 0, 0);                //去除边距

        /* --- 动态计算图标和字体大小 --- */
        lv_coord_t icon_height = row_dsc[row_idx];                  // 原图标大小对应行高100
#ifndef NDEBUG
        printf("%s height:%d\r\n", app_def->name, row_dsc[row_idx]);
#endif
        //app icon
        lv_obj_t* img = lv_img_create(icon_widget);                 //创建图标
        lv_img_set_src(img, app_def->icon);                         //传入图片源
        lv_obj_align(img, LV_ALIGN_CENTER, 0, -icon_height / 15);     //对齐并留出label空间
        lv_img_set_zoom(img, (256 * icon_height) / 100);                // 计算缩放倍率
        //app name
        lv_obj_t* label = lv_label_create(icon_widget);
        lv_label_set_text(label, app_def->name);
        lv_obj_set_style_text_font(label, setup_app_name_font(icon_height), LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -icon_height / 14);
        /* --- 添加点击属性和回调函数 --- */
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, icon_click_event_cb, LV_EVENT_CLICKED, (void*)app_def);//把app传入回调
        //添加此icon_widget到此页面布局
        lv_obj_set_grid_cell(icon_widget, LV_GRID_ALIGN_STRETCH, col_idx, 1, LV_GRID_ALIGN_STRETCH, row_idx, 1);
    }
}

#ifdef layout_fill //布局测试填充所有网格单元格
/**
 * @brief 在所有页面的所有网格单元格中创建占位符,以测试布局,会跳过第 0 行。
 */
static void home_test_fill_all_slots(void) {
    // 遍历所有已创建的页面
    for (uint8_t i = 0; i < page_count; i++) {
        // 遍历每一行 (从第 1 行开始，跳过状态栏区域)
        for (uint8_t row = 1; row < page_row; row++) {
            // 遍历每一列
            for (uint8_t col = 0; col < page_col; col++) {
                // 1. 在当前页面(tile)上创建一个简单的占位符对象
                lv_obj_t* placeholder = lv_obj_create(page[i].tile);
                lv_obj_clear_flag(placeholder, LV_OBJ_FLAG_CLICKABLE);//移除点击属性
                // 2. 为其设置一个独特的背景色，以便区分不同页面的对象
                //    这里使用LVGL的调色板函数，让每个页面的颜色都不同
                lv_obj_set_style_bg_color(placeholder, lv_palette_main(LV_PALETTE_RED + i), 0);
                lv_obj_set_style_border_width(placeholder, 1, 0); // 加个边框看得更清楚
                lv_obj_set_style_border_color(placeholder, lv_color_white(), 0);
                lv_obj_set_style_bg_opa(placeholder, LV_OPA_50, 0);
                // 3. 在占位符中央添加一个标签，显示其坐标，方便调试
                lv_obj_t* label = lv_label_create(placeholder);
                lv_label_set_text_fmt(label, "P%d\n%d, %d", i, row, col);
                lv_obj_set_style_text_color(label, lv_color_white(), 0);
                lv_obj_center(label);
                // 4. 将占位符放置在正确的网格单元格中，并让它填满整个单元格
                lv_obj_set_grid_cell(placeholder,
                LV_GRID_ALIGN_STRETCH, // 水平方向拉伸
                col,                   // 列索引
                1,                     // 占 1 列
                LV_GRID_ALIGN_STRETCH, // 垂直方向拉伸
                row,                   // 行索引
                1);                    // 占 1 行
            }
        }
    }
}
#endif

/**
 * @brief 初始化时创建顶部状态栏,创建一次就不销毁,隐藏时移出屏幕
 * @param null
 */
void StatusBar_data_Init(void) {
    //状态栏主体
    Status_bar = lv_obj_create(lv_layer_top());//创建在顶层
    lv_obj_set_size(Status_bar, 800, StatusBar_data_height);
    lv_obj_set_grid_cell(Status_bar, LV_GRID_ALIGN_CENTER, 0, page_col, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_style_pad_all(Status_bar, 0, LV_PART_MAIN);       //内边距
    lv_obj_set_style_border_width(Status_bar, 0, LV_PART_MAIN);  //边框
    lv_obj_set_style_radius(Status_bar, 0, LV_PART_MAIN);        //圆角
    lv_obj_set_style_bg_opa(Status_bar, StatusBar_data_opa, LV_PART_MAIN);//测试

    //根据底层接口获取实际系统状态初始化状态栏数据
    StatusBar_data.Battery_level = 50;  
    StatusBar_data.Wifi_connect = false;
    StatusBar_data.time = 102;

    //时间标签
    time_label = lv_label_create(Status_bar);
    lv_label_set_text_fmt(time_label, "%02d:%02d", StatusBar_data.time / 100, StatusBar_data.time % 100);//格式化时间
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_18, LV_PART_MAIN);//设置字体
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);    //居中

    //电池图标(使用库内置图标)
    battery_icon = lv_label_create(Status_bar);
    lv_obj_set_style_text_font(battery_icon, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(battery_icon, LV_ALIGN_RIGHT_MID, -10, 0); // 左对齐
    //电量
    battery_label = lv_label_create(Status_bar);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(battery_label, battery_icon, LV_ALIGN_OUT_LEFT_MID, 10, 0); // 在电池图标左侧对齐

    //WiFi标志
    wifi_icon = lv_label_create(Status_bar);
    lv_label_set_text(wifi_icon, StatusBar_data.Wifi_connect ? LV_SYMBOL_WIFI : LV_SYMBOL_CLOSE); // 使用 LVGL 的 WiFi 符号
    lv_obj_set_style_text_font(wifi_icon, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_align_to(wifi_icon, battery_label, LV_ALIGN_OUT_LEFT_MID, -10, 0); // 左对齐

    //设置电池图标和百分比
    set_battery_icon(StatusBar_data.Battery_level);
    //设置WiFi图标状态
    set_Wifi_icon(StatusBar_data.Wifi_connect);
    //设置事件标签
    set_time_label(1,3);
}


/* ============================================== 回调函数 ============================================== */
/**
 * @brief 图标点击回调函数,打开对应app
 * @param e 
 */
static void icon_click_event_cb(lv_event_t* e) {
    app_def_t* activity_app = (app_def_t*)lv_event_get_user_data(e);//获取触发的app
    if (activity_app) {
#ifdef NDEBUG
    printf("click_icon:%s\r\n", activity_app->name);
#endif
        act_manager_switch_to(activity_app,NULL);//切换到触发的app
    }
}

/* ============================================== 外部调用接口 ============================================== */

/**
 * @brief 获取状态栏高
 * @return 状态栏高
 */
uint8_t get_StatusBar_data_height() {
    return StatusBar_data_height;
}
/**
 * @brief 获取状态栏透明度
 * @return 状态栏透明度
 */
uint8_t get_StatusBar_data_opa() {
    return StatusBar_data_opa;
}

/**
 * @brief 传入0~100，设置电池百分比和图标
 * @param level  电量水平
 */
void set_battery_icon(uint8_t level) {
    //电池图标
    if (level <= 20) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY); // 使用 LVGL 的电池符号
    }
    else if (level <= 40) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_1);
    }
    else if (level <= 60) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_2);
    }
    else if (level <= 80) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_3);
    }
    else {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL); // 使用 LVGL 的电池符号
    }
    // 设置电量百分比标签
    lv_label_set_text_fmt(battery_label, "%d%%", level);
    lv_obj_align_to(battery_label, battery_icon, LV_ALIGN_OUT_LEFT_MID, -2, 0); // 字符总长可能变化需要重新设置对齐

    lv_obj_align_to(wifi_icon, battery_label, LV_ALIGN_OUT_LEFT_MID, -10, 0); // 左对齐
}

/**
 * @brief 设置WiFi图标状态
 * @param connect  true:连接, false:未连接
 */
void set_Wifi_icon(bool connect) {
    StatusBar_data.Wifi_connect = connect; // 更新状态栏数据
    lv_label_set_text(wifi_icon, connect ? LV_SYMBOL_WIFI : LV_SYMBOL_CLOSE); // 更新图标对象
    lv_obj_align_to(wifi_icon, battery_label, LV_ALIGN_OUT_LEFT_MID, -10, 0); // 左对齐
}

/**
 * @brief 设置时间标签
 * @param hour    小时
 * @param minute  分钟
 */
void set_time_label(uint8_t hour, uint8_t minute) {
    StatusBar_data.time = hour * 100 + minute; // 更新状态栏数据
    lv_label_set_text_fmt(time_label, "%02d:%02d", hour, minute); // 格式化时间
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0); // 居中对齐
}

/* ============================================== 通用app接口实现 ============================================== */
static app_def_t home = {
    .create = home_create_cb,
    .destroy = home_destroy_cb,
    .pause = home_pause_cb,
    .resume = home_resume_cb,
    .name = "HOME",
    .icon = &icon_home
};

//接口获取
app_def_t* home_def_get() {
    return &home;
}

/**
 * @brief 主页面创建(主页面应该在home_init初始化时就创建
 * @return 返回主页面屏幕
 */
lv_obj_t* home_create_cb(void){
    //当每个 Display(lv_display) 对象被创建时,会与之一起创建一个默认屏幕,并设置为其“Active Screen”
    lv_obj_t* home_Screens = lv_scr_act();//获取这个屏幕并把它作为home的页面
    lv_obj_clean(home_Screens);//清屏

    //======================创建壁纸层=====================//
    home_wallpaper = lv_obj_create(home_Screens);
    lv_obj_set_size(home_wallpaper, 800, 480); // 设置为屏幕大小
    lv_obj_set_style_pad_all(home_wallpaper, 0, LV_PART_MAIN);//内边距
    lv_obj_set_style_border_width(home_wallpaper, 0, LV_PART_MAIN);//边框
    //这里先使用图片作为壁纸,省内存也可用API绘制
    lv_obj_t* bg_img = lv_img_create(home_wallpaper);           /*样式修改标记*/
    lv_img_set_src(bg_img, &wallpaper);

    //======================tileview(滑动页面)=======================//
    //创建图标列表(平铺视图)
    lv_obj_t* tileview = lv_tileview_create(home_Screens);
    //设置透明度为0
    lv_obj_set_style_bg_opa(tileview, LV_OPA_0, LV_PART_MAIN);
#ifdef NDEBUG
    lv_obj_remove_style(tileview, NULL, LV_PART_SCROLLBAR);     //移除滚动条
#endif
    // 初始化页面计数器
    for (int i = 0; i < page_count; i++) {
        page[i].app_count = 0;
    }
    // 添加页面并设置布局
    for (uint8_t i = 0; i < page_count; i++) {
        page[i].tile = lv_tileview_add_tile(tileview, i, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
        home_setup_grid_layout(page[i].tile);
    }

    //添加状态栏
    StatusBar_data_Init();

    //======================添加子项(图标)=====================//
#ifdef app
    // 添加应用图标0
    uint8_t app_total = activity_manager_get_registered_count();//获取应用数量
    for (uint8_t i = 0; i < app_total; i++) {
        const app_def_t* app_def = activity_manager_get_def_by_index(i);//获取应用列表
        if (strcmp(app_def->name, "HOME") == 0) {
            continue;
        }
        bool placed = false;//已放置
        // 1. 检查是否在指定页面
        for (uint8_t j = 0; j < page_count; j++) {
            if (is_app_in_page_list(app_def->name, APP_in_page[j])) {//判断名称是否存在这一页
                // 计算图标要放置的网格位置
                uint8_t current_app_idx = page[j].app_count;    //已存在数量作为位置
                uint8_t col_idx = current_app_idx % page_col;   //列数,计算换行
                uint8_t row_idx = 1 + (current_app_idx / page_col); // 计算行数,从第1行开始放图标,第0行是状态栏
                //添加到父页面布局
                create_icon_widget(app_def, page[j].tile, row_idx, col_idx);
                page[j].app_count++;
                placed = true;//已放置
                break; //放置后跳出页面循环
            }
        }
        if (!placed) {//未放置放在默认位置(从第一页起找个空位)
            for (uint8_t j = 0; j<page_count; j++) {
                if (page[j].app_count < page_col * (page_row - 1)) { // 如果此页面未满
                    uint8_t current_app_idx = page[j].app_count;    //已存在数量作为位置
                    uint8_t col_idx = current_app_idx % page_col;   //列数,计算换行
                    uint8_t row_idx = 1 + (current_app_idx / page_col); // 计算行数,从第1行开始放图标,第0行是状态栏
                    //添加到父页面布局
                    create_icon_widget(app_def, page[j].tile, row_idx, col_idx);
                    page[j].app_count++;
                    placed = true;
                    break;
                }
            }
        }
    }
#endif

#ifdef layout_fill    //布局测试
    home_test_fill_all_slots();
#endif


    return home_Screens;
}

/**
 * @brief 主页面不会被释放(快速返回桌面)
 */
void home_destroy_cb(struct activity_t* activity) {}
/**
 * @brief 主页面不会被释放(快速返回桌面)
 */
void home_pause_cb(struct activity_t* activity) {}
/**
 * @brief 主页面不会被释放(快速返回桌面)
 */
void home_resume_cb(struct activity_t* activity) {}


