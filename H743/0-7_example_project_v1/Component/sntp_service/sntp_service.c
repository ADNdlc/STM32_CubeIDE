/*
 * sntp_service.c
 *
 *  Implementation of the SNTP service.
 */

#include "sntp_service.h"
#include "rtc_hal/rtc_hal.h"
#include "sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "sntp_svc"
#include "elog.h"

// 默认同步周期：1天 (24 * 3600 * 1000 ms)
#define DEFAULT_SYNC_INTERVAL_MS (24 * 3600 * 1000)

// 内部处理函数
static time_t parse_asctime(const char *buf);

/**
 * @brief 异步获取时间的结果回调
 */
static void sntp_on_time_received(sntp_driver_t *drv, int result,
                                  const char *time_str, void *user_data)
{
  sntp_service_t *self = (sntp_service_t *)user_data;
  if (!self)
    return;

  if (result == 0 && time_str && strlen(time_str) > 0){
    time_t sync_ts = parse_asctime(time_str); // 解析时间字符串(asctime style time)为时间戳
    if (sync_ts > 0)
    {
      log_i("SNTP synced successfully: %s", time_str);
      self->last_sync_tick = sys_get_systick_ms();
      self->status = SNTP_SVC_STATUS_SYNCED;  // 时间同步成功

      // --- RTC 校准逻辑 ---
      struct tm *tm_info = localtime(&sync_ts);
      rtc_time_t rtc_time = {.hour = tm_info->tm_hour,
                             .minute = tm_info->tm_min,
                             .second = tm_info->tm_sec};
      rtc_date_t rtc_date = {
          .year = (uint8_t)(tm_info->tm_year + 1900 - 2000), // 假设 2000 年基准
          .month = (uint8_t)(tm_info->tm_mon + 1),
          .day = (uint8_t)tm_info->tm_mday,
          .week = (uint8_t)tm_info->tm_wday};

      // RTC 校准(RTC为全局单例,在此之前应该初始化完成)
      rtc_hal_set_time(&rtc_time);
      rtc_hal_set_date(&rtc_date);

      // 调用事件回调
      if (self->event_cb){
        self->event_cb(self, self->status, self->user_data);
      }
      return;
    }
  }

  log_e("SNTP sync failed or returned invalid data");
  self->status = SNTP_SVC_STATUS_ERROR; // 同步失败
  if (self->event_cb){
    self->event_cb(self, self->status, self->user_data);
  }
}

/* ---- 外部API ---- */
void sntp_svc_init(sntp_service_t *self, sntp_driver_t *driver)
{
  if (!self || !driver)
    return;
  memset(self, 0, sizeof(sntp_service_t));
  self->driver = driver;
  self->status = SNTP_SVC_STATUS_IDLE;
  self->sync_interval_ms = DEFAULT_SYNC_INTERVAL_MS;
}

/**
 * @brief 注册sntp事件回调
 *
 * @param self      服务实例
 * @param cb        回调函数
 * @param user_data 用户上下文
 */
void sntp_svc_register_callback(sntp_service_t *self, sntp_svc_event_cb_t cb,
                                void *user_data)
{
  if (!self)
    return;
  self->event_cb = cb;
  self->user_data = user_data;
}

void sntp_svc_set_network_ready(sntp_service_t *self, bool ready)
{
  if (!self)
    return;
  if (self->wifi_ready != ready)
  {
    self->wifi_ready = ready;
    log_d("SNTP Network Ready: %s", ready ? "YES" : "NO");

    // 网络恢复后重置同步定时，以便立即同步一次
    if (ready)
    {
      self->last_sync_tick = 0;
    }
  }
}

/**
 * @brief 设置并启动同步
 *
 * @param self        服务实例
 * @param timezone    时区
 * @param interval_ms 同步周期
 * @return int        0: 成功，-1: 失败
 */
int sntp_svc_start_sync(sntp_service_t *self, int timezone,
                        uint32_t interval_ms){
  if (!self || !self->driver)
    return -1;

  self->timezone = timezone;
  if (interval_ms > 0){
    self->sync_interval_ms = interval_ms;
  }

  log_i("SNTP Config: TZ=%d, Interval=%u ms", timezone, self->sync_interval_ms);
  self->status = SNTP_SVC_STATUS_IDLE;

  // 配置驱动层 SNTP 参数 ,当前模块(esp8266)会自动开始获取sntp时间，成功后驱动层状态变为SNTP_DRV_STATUS_SYNCED
  return self->driver->ops->config(self->driver, 1, timezone, "cn.ntp.org.cn");
}

/**
 * @brief 处理SNTP服务(何时同步)
 * 
 * @param self 
 */
void sntp_svc_process(sntp_service_t *self)
{
  if (!self || !self->driver || !self->wifi_ready)
    return;

  uint32_t now = sys_get_systick_ms();
  sntp_drv_status_t drv_status = self->driver->ops->get_status(self->driver); // 获取驱动状态

  // 状态机处理
  if (drv_status == SNTP_DRV_STATUS_SYNCED){ // 查询驱动层是否已同步
    if (self->last_sync_tick != 0 &&(now - self->last_sync_tick <= self->sync_interval_ms)){
      return; // 还没到同步时间,直接返回
    }
    if (self->status == SNTP_SVC_STATUS_SYNCING){
      return; // 正在同步中,直接返回
    }

    log_d("Sync interval reached, requesting time update...");
    self->status = SNTP_SVC_STATUS_SYNCING; //开始获取同步时间
    self->driver->ops->get_time(self->driver, sntp_on_time_received, self);// 注册结果回调
  }
}

time_t sntp_svc_get_time(sntp_service_t *self)
{
  rtc_time_t rt;
  rtc_date_t rd;
  if (rtc_hal_get_time(&rt) != 0 || rtc_hal_get_date(&rd) != 0)
    return 0;

  struct tm t;
  memset(&t, 0, sizeof(struct tm));
  t.tm_year = rd.year + 2000 - 1900;
  t.tm_mon = rd.month - 1;
  t.tm_mday = rd.day;
  t.tm_hour = rt.hour;
  t.tm_min = rt.minute;
  t.tm_sec = rt.second;

  return mktime(&t);
}

// 内部函数：解析 "Fri May 30 18:06:18 2025" 格式
static time_t parse_asctime(const char *buf)
{
  struct tm t;
  char month[4], week[4];
  memset(&t, 0, sizeof(struct tm));

  if (sscanf(buf, "%s %s %d %d:%d:%d %d", week, month, &t.tm_mday, &t.tm_hour,
             &t.tm_min, &t.tm_sec, &t.tm_year) != 7)
  {
    return 0;
  }

  static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  for (int i = 0; i < 12; i++)
  {
    if (strcmp(month, months[i]) == 0)
    {
      t.tm_mon = i;
      break;
    }
  }
  t.tm_year -= 1900;
  return mktime(&t);
}
