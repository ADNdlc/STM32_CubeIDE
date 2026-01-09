#include "flash_handler.h"
#include "elog.h"
#include "sys.h"
#include <string.h>

#define LOG_TAG "FlashHdlr"

#define MAX_MOUNT_POINTS 4 // 最大挂载点

typedef struct {
  char prefix[16];            // 前缀
  block_device_t *dev;        // 设备
  flash_strategy_t *strategy; // 策略
} mount_point_t;

static mount_point_t mount_table[MAX_MOUNT_POINTS]; // 挂载表
static int mount_count = 0;

int flash_handler_init(void) {
  memset(mount_table, 0, sizeof(mount_table));
  mount_count = 0;
  return 0;
}

/**
 * @brief 注册带有路径前缀和策略的设备
 * 
 * @param prefix   前缀(字符串)
 * @param dev      block_device_t 设备对象
 * @param strategy flash_strategy_t 策略对象
 * @return int     0: 成功, -1: 失败
 */
int flash_handler_register(const char *prefix, block_device_t *dev,
                           flash_strategy_t *strategy) {
  if (mount_count >= MAX_MOUNT_POINTS) {
    log_e("Too many mount points");
    return -1;
  }

  // 调用策略的挂载函数
  if (strategy && strategy->ops->mount) {
    if (strategy->ops->mount(strategy, dev) != 0) {
      log_e("Failed to mount %s", prefix);
      return -1;
    }
  }

  // 添加到挂载表
  strncpy(mount_table[mount_count].prefix, prefix,
          sizeof(mount_table[mount_count].prefix) - 1);
  mount_table[mount_count].dev = dev;
  mount_table[mount_count].strategy = strategy;

  mount_count++;
  log_i("Mount point %s registered", prefix);
  return 0;
}

/**
 * @brief 根据路径查找挂载点
 * 
 * @param path 
 * @return mount_point_t* 
 */
static mount_point_t *_find_mount_point(const char *path) {
  // 遍历挂载表查找匹配的前缀
  for (int i = 0; i < mount_count; i++) {
    if (strncmp(path, mount_table[i].prefix, strlen(mount_table[i].prefix)) == 0) {
      return &mount_table[i];
    }
  }
  return NULL;
}

/**
 * @brief 读取数据
 * 
 * @param path   路径
 * @param offset 偏移
 * @param buf    数据缓冲区
 * @param size   大小
 * @return int   0: 成功, -1: 失败
 */
int flash_handler_read(const char *path, uint32_t offset, uint8_t *buf,
                       size_t size) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || !mp->strategy)
    return -1;
  return mp->strategy->ops->read(mp->strategy, path, offset, buf, size);
}

/**
 * @brief 写入数据
 * 
 * @param path   路径
 * @param offset 偏移
 * @param buf    数据缓冲区
 * @param size   大小
 * @return int   0: 成功, -1: 失败
 */
int flash_handler_write(const char *path, uint32_t offset, const uint8_t *buf,
                        size_t size) {
  mount_point_t *mp = _find_mount_point(path);
  if (!mp || !mp->strategy)
    return -1;
  return mp->strategy->ops->write(mp->strategy, path, offset, buf, size);
}

// LVGL FS 端口获取文件系统实例
#include "lfs.h"
lfs_t *flash_handler_get_lfs(const char *prefix) {
  mount_point_t *mp = _find_mount_point(prefix);
  if (!mp || !mp->strategy)
    return NULL;
  // We assume the caller knows this prefix belongs to an LFS strategy
  return lfs_strategy_get_lfs(mp->strategy);
}
