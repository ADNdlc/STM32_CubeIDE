#include "data_logger.h"
#include "Sys.h"
#include "rtc_factory.h"
#include "sys_config.h"
#include "thing_model/thing_model.h"
#include "vfs_manager.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "DATA_LOG"
#include "elog.h"

#define LOG_BUFFER_SIZE 1024
// 动态生成文件名

// 定时刷写参数
#define FLUSH_INTERVAL_MS 10000 // 10秒刷写一次

#define FLUSH_THRESHOLD_BYTES 512 // 满 512 字节强制刷写

static char s_log_buffer[LOG_BUFFER_SIZE];
static size_t s_log_len = 0;
static uint32_t s_last_flush_time = 0;
static rtc_driver_t *s_rtc_drv = NULL;

static void get_current_log_path(char *path, size_t size) {
  if (s_rtc_drv) {
    rtc_date_t date;
    if (RTC_GET_DATE(s_rtc_drv, &date) == 0) {
      snprintf(path, size, "/sys/log/data_20%02d%02d%02d.log", date.year,
               date.month, date.day);
      return;
    }
  }
  // 如果 RTC 不可用，回退到默认文件名
  snprintf(path, size, "/sys/log/data.log");
}

static int data_logger_flush(void) {
  if (s_log_len == 0) {
    return 0;
  }

  // 保护：检查 VFS 是否挂载成功
  if (!vfs_point_is_mounted("/sys")) {
    log_w("VFS point /sys is offline or detached. Skip writing logs.");
    return -1;
  }

  char current_path[64];
  get_current_log_path(current_path, sizeof(current_path));

  // 写入 VFS 文件系统
  vfs_mkdir("/sys/log");

  int fd = vfs_open(current_path, VFS_O_WRONLY | VFS_O_CREAT | VFS_O_APPEND);
  if (fd >= 0) {
    int res = vfs_write(fd, s_log_buffer, s_log_len);
    if (res > 0) {
      log_d("Flushed %d bytes to %s", res, current_path);
    } else {
      log_e("Failed to write to %s", current_path);
      vfs_close(fd);
      return -1;
    }
    vfs_close(fd);
  } else {
    log_e("Failed to open %s for write", current_path);
    return -1;
  }

  // 清空缓冲
  s_log_len = 0;
  s_last_flush_time = sys_get_systick_ms();
  return 0;
}

static void on_thing_model_event(const thing_model_event_t *event,
                                 void *user_data) {
  if (!sys_config_get_local_data_save()) {
    return;
  }

  if (event->type == THING_EVENT_PROPERTY_CHANGED) {
    char time_str[32] = {0};

    if (s_rtc_drv) {
      rtc_time_t time;
      rtc_date_t date;
      if (RTC_GET_TIME(s_rtc_drv, &time) == 0 &&
          RTC_GET_DATE(s_rtc_drv, &date) == 0) {
        snprintf(time_str, sizeof(time_str), "20%02d-%02d-%02d %02d:%02d:%02d",
                 date.year, date.month, date.day, time.hour, time.minute,
                 time.second);
      } else {
        snprintf(time_str, sizeof(time_str), "%lu", sys_get_systick_ms());
      }
    } else {
      snprintf(time_str, sizeof(time_str), "%lu", sys_get_systick_ms());
    }

    char record[128];
    int len = 0;

    // 使用事件中的设备指针直接查找属性，解决同 ID 冲突问题
    thing_device_t *dev = event->device;
    if (dev) {
      thing_property_t *prop = find_property_by_id(dev, event->prop_id);
      if (prop && prop->local_log) { // 仅记录 local_log 为真的属性
        switch (prop->type) {
        case THING_PROP_TYPE_SWITCH:
          len = snprintf(record, sizeof(record), "[%s] %s.%s = %s\n", time_str,
                         event->device_id, event->prop_id,
                         event->value.b ? "true" : "false");
          break;
        case THING_PROP_TYPE_INT:
          len = snprintf(record, sizeof(record), "[%s] %s.%s = %ld\n", time_str,
                         event->device_id, event->prop_id, event->value.i);
          break;
        case THING_PROP_TYPE_FLOAT:
          len =
              snprintf(record, sizeof(record), "[%s] %s.%s = %.2f\n", time_str,
                       event->device_id, event->prop_id, event->value.f);
          break;
        case THING_PROP_TYPE_STRING:
          len = snprintf(record, sizeof(record), "[%s] %s.%s = \"%s\"\n",
                         time_str, event->device_id, event->prop_id,
                         event->value.s ? event->value.s : "");
          break;
        default:
          len =
              snprintf(record, sizeof(record), "[%s] %s.%s = (unknown type)\n",
                       time_str, event->device_id, event->prop_id);
          break;
        }
      }
    }
    if (s_log_len + len < LOG_BUFFER_SIZE) {
      memcpy(s_log_buffer + s_log_len, record, len);
      s_log_len += len;
      s_log_buffer[s_log_len] = '\0';
    } else {
      // 容量不足，先强制刷写
      data_logger_flush();
      // 再存入
      if (len < LOG_BUFFER_SIZE) {
        memcpy(s_log_buffer, record, len);
        s_log_len = len;
        s_log_buffer[s_log_len] = '\0';
      }
    }
  }
}

void data_logger_init(void) {
  s_log_len = 0;
  memset(s_log_buffer, 0, sizeof(s_log_buffer));
  s_last_flush_time = sys_get_systick_ms();

  // 获取 RTC 句柄
  s_rtc_drv = rtc_driver_get(RTC_ID_INTERNAL);
  if (!s_rtc_drv) {
    log_w("RTC driver not found, falling back to systick for data logger.");
  } else {
    if (!RTC_GET_INITFLAG(s_rtc_drv)) {
      log_w("RTC not initialized, data logger timestamps will be invalid until "
            "set.");
    }
  }

  // 注册物模型大事件观察者
  thing_model_add_observer(on_thing_model_event, NULL);

  log_i("Data logger initialized.");
}

void data_logger_process(void) {
  if (!sys_config_get_local_data_save()) {
    // 如果开关被关闭，可以根据需要清空缓冲
    s_log_len = 0;
    return;
  }

  if (s_log_len == 0) {
    return;
  }

  uint32_t now = sys_get_systick_ms();
  bool time_to_flush = (now - s_last_flush_time >= FLUSH_INTERVAL_MS);
  bool size_to_flush = (s_log_len >= FLUSH_THRESHOLD_BYTES);

  if (time_to_flush || size_to_flush) {
    if (data_logger_flush() != 0) {
      s_last_flush_time = now;
    }
  }
}
