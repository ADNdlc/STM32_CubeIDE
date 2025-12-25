#include "flash_handler.h"
#include "sys.h"    // mutex还未实现
#include <string.h>


#define MAX_MOUNT_POINTS 4  // 最大挂载点

typedef struct {
  char prefix[16];            // 前缀
  block_device_t *dev;        // 设备
  flash_strategy_t *strategy; // 策略
} mount_point_t;

static mount_point_t mount_table[MAX_MOUNT_POINTS]; // 挂载表
static int mount_count = 0;

// Global Mutex (Coarse-grained)
// static sys_mutex_t handler_mutex;
// Placeholder for mutex APIs

int flash_handler_init(void) {
  memset(mount_table, 0, sizeof(mount_table));
  mount_count = 0;
  // handler_mutex = sys_mutex_create();  // 创建互斥锁(未实现)
  return 0;
}

/**
 * @brief 注册挂载点
 * 
 * @param prefix   前缀
 * @param dev      设备
 * @param strategy 策略
 * @return int     0 成功，-1 失败
 */
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy) {
  // Lock Mutex
  if (mount_count >= MAX_MOUNT_POINTS)
    return -1;

  // 加入注册表
  strncpy(mount_table[mount_count].prefix, prefix,
          sizeof(mount_table[mount_count].prefix) - 1);
  mount_table[mount_count].dev = dev;
  mount_table[mount_count].strategy = strategy;

  // 挂载设备(调用策略的 mount函数)
  if (strategy && strategy->ops->mount) {
    strategy->ops->mount(strategy, dev);
  }

  mount_count++;
  // Unlock Mutex
  return 0;
}

/**
 * @brief 查找挂载点
 * 
 * @param path 路径
 * @return mount_point_t* 挂载点
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
 * 
 * @param path     路径
 * @param offset   偏移
 * @param buf      缓冲区
 * @param size     大小
 * @return int     0 成功，-1 失败
 */
int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size) {
  // Lock Coarse-grained Mutex
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || !mp->strategy) {
    // Unlock
    return -1;
  }

  int ret = mp->strategy->ops->read(mp->strategy, path, offset, buf, size);
  // Unlock
  return ret;
}

/**
 * @brief 写入数据
 * 
 * @param path     路径
 * @param offset   偏移
 * @param buf      缓冲区
 * @param size     大小
 * @return int     0 成功，-1 失败
 */
int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size) {
  // Lock Coarse-grained Mutex
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || !mp->strategy) {
    // Unlock
    return -1;
  }

  int ret = mp->strategy->ops->write(mp->strategy, path, offset, buf, size);
  // Unlock
  return ret;
}
