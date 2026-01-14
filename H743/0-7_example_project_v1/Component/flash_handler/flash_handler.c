#include "flash_handler.h"
#include "elog.h"
#include "main.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "FlashHdlr"

#define MAX_MOUNT_POINTS 4 // 最大挂载点

typedef struct {
  char prefix[16];            // 前缀
  block_device_t *dev;        // 设备
  flash_strategy_t *strategy; // 策略
  flash_status_t status;      // 当前状态
  uint32_t next_retry_tick;   // 下次尝试挂载的时间戳
} mount_point_t;

static mount_point_t mount_table[MAX_MOUNT_POINTS]; // 挂载表
static int mount_count = 0;
static flash_event_cb_t g_event_cb = NULL;

int flash_handler_init(void) {
  memset(mount_table, 0, sizeof(mount_table));
  mount_count = 0;
  g_event_cb = NULL;
  return 0;
}

void flash_handler_set_callback(flash_event_cb_t cb) { g_event_cb = cb; }

/**
 * @brief 注册带有路径前缀和策略的设备
 */
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy) {
  if (mount_count >= MAX_MOUNT_POINTS) {
    log_e("Too many mount points");
    return -1;
  }

  // 添加到挂载表 (初始状态设为 DISCONNECTED)
  strncpy(mount_table[mount_count].prefix, prefix,
          sizeof(mount_table[mount_count].prefix) - 1);
  mount_table[mount_count].dev = dev;
  mount_table[mount_count].strategy = strategy;
  mount_table[mount_count].status = FLASH_STATUS_DISCONNECTED;
  mount_table[mount_count].next_retry_tick = 0;

  // 尝试初始挂载
  if (strategy && strategy->ops->mount) {
    if (strategy->ops->mount(strategy, dev) == 0) {
      mount_table[mount_count].status = FLASH_STATUS_READY;
      log_i("Mount point %s initialized as READY", prefix);
    } else {
      log_w("Mount point %s registered as DISCONNECTED (init failed)", prefix);
    }
  }

  mount_count++;
  return 0;
}

/**
 * @brief 内部事件通知
 */
static void _notify_event(const char *prefix, flash_event_t event) {
  if (g_event_cb) {
    g_event_cb(prefix, event);
  }
}

/**
 * @brief 状态轮询
 */
void flash_handler_poll(void) {
  uint32_t now = HAL_GetTick();

  for (int i = 0; i < mount_count; i++) {
    mount_point_t *mp = &mount_table[i];
    if (!mp->dev || !mp->dev->ops->sync)
      continue;

    // 1. 如果已断开且未到重试时间，则跳过（避免频繁初始化导致卡顿）
    if (mp->status == FLASH_STATUS_DISCONNECTED) {
      if (now < mp->next_retry_tick)
        continue;
    }

    // 2. 探测设备
    int ready = (BLOCK_DEV_SYNC(mp->dev) == 0);

    // 3. 状态切换逻辑
    if (mp->status == FLASH_STATUS_DISCONNECTED && ready) {
      log_i("Device %s detected, attempting mount...", mp->prefix);
      if (mp->strategy && mp->strategy->ops->mount) {
        if (mp->strategy->ops->mount(mp->strategy, mp->dev) == 0) {
          mp->status = FLASH_STATUS_READY;
          _notify_event(mp->prefix, FLASH_EVENT_INSERTED);
        } else {
          // 挂载失败，设置 2 秒后再重试
          mp->next_retry_tick = now + 2000;
        }
      }
    } else if (mp->status == FLASH_STATUS_READY && !ready) {
      log_w("Device %s removed", mp->prefix);
      if (mp->strategy && mp->strategy->ops->unmount) {
        mp->strategy->ops->unmount(mp->strategy);
      }
      mp->status = FLASH_STATUS_DISCONNECTED;
      mp->next_retry_tick = now + 1000; // 移除后延时探测
      _notify_event(mp->prefix, FLASH_EVENT_REMOVED);
    }
  }
}

/**
 * @brief 根据路径查找挂载点
 */
static mount_point_t *_find_mount_point(const char *path) {
  for (int i = 0; i < mount_count; i++) {
    if (strncmp(path, mount_table[i].prefix, strlen(mount_table[i].prefix)) ==
        0) {
      return &mount_table[i];
    }
  }
  return NULL;
}

/**
 * @brief 读取数据
 */
int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || !mp->strategy || mp->status != FLASH_STATUS_READY)
    return -1;
  return mp->strategy->ops->read(mp->strategy, path, offset, buf, size);
}

/**
 * @brief 写入数据
 */
int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || !mp->strategy || mp->status != FLASH_STATUS_READY)
    return -1;
  return mp->strategy->ops->write(mp->strategy, path, offset, buf, size);
}

// LVGL FS 端口获取文件系统实例
#include "lfs.h"
#include "strategy/lfs_strategy.h"
lfs_t *flash_handler_get_lfs(const char *prefix) {
  mount_point_t *mp = _find_mount_point(prefix);
  if (!mp || !mp->strategy || mp->status != FLASH_STATUS_READY)
    return NULL;
  return lfs_strategy_get_lfs(mp->strategy);
}
