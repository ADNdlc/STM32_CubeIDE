#include "vfs_manager.h"
#include "elog.h"
#include <string.h>
#include <stdbool.h>

#define LOG_TAG "VFS"

static mount_point_t s_mount_points[VFS_MAX_MOUNT_POINTS];
static vfs_event_cb_t s_event_cb = NULL;

#define VFS_MAX_OPEN_FILES 8

typedef struct {
    mount_point_t *mp;
    vfs_file_t fs_file;
    bool in_use;
} vfs_fd_node_t;

static vfs_fd_node_t s_fds[VFS_MAX_OPEN_FILES];

/* ------------------ 内部辅助函数 ------------------ */

// 路径匹配路由逻辑
static mount_point_t* find_mount_point(const char *path, const char **rel_path) {
    const char *p = path;
    if (p[0] == '/') p++; // 转换 "/lfs/aa.txt" 为 "lfs/aa.txt"

    for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
        mount_point_t *mp = &s_mount_points[i];
        if (mp->name[0] == '\0' || !mp->is_mounted) continue;

        size_t len = strlen(mp->name);
        // 完全匹配前缀且后面跟着 / 或结束符
        if (strncmp(p, mp->name, len) == 0 && (p[len] == '/' || p[len] == '\0')) {
            if (p[len] == '/') *rel_path = p + len + 1;
            else *rel_path = ".";
            return mp;
        }
    }
    return NULL;
}

static int alloc_fd(mount_point_t *mp, vfs_file_t fs_file) {
    for (int i = 0; i < VFS_MAX_OPEN_FILES; i++) {
        if (!s_fds[i].in_use) {
            s_fds[i].mp = mp;
            s_fds[i].fs_file = fs_file;
            s_fds[i].in_use = true;
            return i;
        }
    }
    return -1;
}

/**
 * @brief 内部底层事件回调 (由底层驱动触发)
 */
static void vfs_internal_dev_cb(storage_device_t *self, dev_event_t event, void *user_data) {
    mount_point_t *mp = (mount_point_t *)user_data;
    if (!mp) return;

    log_i("VFS Dev Event: [%s] -> %d", mp->name, event);

    if (event == DEVICE_EVENT_REMOVED) {
        mp->dev_state = STORAGE_STATUS_OFFLINE;
        if (mp->is_mounted) {
            log_w("VFS: Sudden removal detected for [%s], force unmounting.", mp->name);
            vfs_unmount(mp->name);
        }
    } else if (event == DEVICE_EVENT_INSERTED) {
        mp->dev_state = STORAGE_STATUS_OK;
        log_i("VFS: Device [%s] ready for re-mount.", mp->name);
    }

    if (s_event_cb) {
        s_event_cb(mp->name, event);
    }
}

/* ------------------ VFS 管理核心接口 ------------------ */

int vfs_init(void) {
    memset(s_mount_points, 0, sizeof(s_mount_points));
    memset(s_fds, 0, sizeof(s_fds));
    log_i("VFS Manager initialized.");
    return 0;
}

int vfs_mount(const char *path_prefix, storage_device_t *dev, fs_strategy_t *strategy) {
    if (!path_prefix || !dev || !strategy) return -1;
    
    // 规范化前缀逻辑 (去掉前导 /)
    const char *prefix = path_prefix;
    if (prefix[0] == '/') prefix++;

    int free_idx = -1;
    for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
        if (s_mount_points[i].name[0] == '\0') {
            if (free_idx == -1) free_idx = i;
        } else if (strcmp(s_mount_points[i].name, prefix) == 0) {
            log_w("VFS Mount point [%s] busy.", prefix);
            return -2;
        }
    }
    if (free_idx == -1) return -3;

    mount_point_t *mp = &s_mount_points[free_idx];
    strncpy((char*)mp->name, prefix, sizeof(mp->name) - 1);
    mp->device = dev;
    mp->fs_strategy = strategy;
    mp->is_mounted = false;
    
    // 初始化时探测状态
    mp->dev_state = STORAGE_CHECK_ALIVE(dev);
    
    // 绑定事件以便触发异步卸载
    STORAGE_SET_CB(dev, vfs_internal_dev_cb, mp);

    if (mp->dev_state == STORAGE_STATUS_OK) {
        if (VFS_MOUNT(strategy, mp) == 0) {
            mp->is_mounted = true;
            log_i("VFS: Mounted [%s] successfully.", mp->name);
        }
    } else {
        log_w("VFS: Device [%s] offline, waiting for insertion...", mp->name);
    }

    return 0;
}

int vfs_unmount(const char *path_prefix) {
    const char *prefix = path_prefix;
    if (prefix[0] == '/') prefix++;

    for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
        if (strcmp(s_mount_points[i].name, prefix) == 0) {
            if (s_mount_points[i].is_mounted) {
                VFS_UNMOUNT(s_mount_points[i].fs_strategy, &s_mount_points[i]);
                s_mount_points[i].is_mounted = false;
            }
            memset(&s_mount_points[i], 0, sizeof(mount_point_t));
            return 0;
        }
    }
    return -1;
}

void vfs_storage_monitor_task(void) {
    for (int i = 0; i < VFS_MAX_MOUNT_POINTS; i++) {
        mount_point_t *mp = &s_mount_points[i];
        if (mp->name[0] == '\0' || !mp->device) continue;

        // 对处在挂载列表中但未标识 IO 占用的设备进行存活轮询
        storage_status_t current = STORAGE_CHECK_ALIVE(mp->device);
        
        if (current != mp->dev_state) {
            log_d("VFS Monitor: [%s] state transition %d -> %d", mp->name, mp->dev_state, current);
            
            if (current == STORAGE_STATUS_OK && !mp->is_mounted) {
                // 尝试自动重新挂载
                if (VFS_MOUNT(mp->fs_strategy, mp) == 0) {
                    mp->is_mounted = true;
                }
            } else if (current == STORAGE_STATUS_NOT_INIT) {
                // 探测到卡已插入或需要初始化
                log_i("VFS Monitor: [%s] needs initialization, calling STORAGE_INIT...", mp->name);
                if (STORAGE_INIT(mp->device) == 0) {
                    // 初始化成功后，下一次轮询会进入 STORAGE_STATUS_OK 分支进行挂载
                    log_i("VFS Monitor: [%s] initialized successfully.", mp->name);
                } else {
                    log_e("VFS Monitor: [%s] initialization failed.", mp->name);
                }
            } else if (current == STORAGE_STATUS_OFFLINE && mp->is_mounted) {
                // 主动探测到了移除
                VFS_UNMOUNT(mp->fs_strategy, mp);
                mp->is_mounted = false;
            }
            mp->dev_state = current;
        }
    }
}

void vfs_set_event_callback(vfs_event_cb_t cb) {
    s_event_cb = cb;
}

/* ------------------ 标准 POSIX 路由实现 ------------------ */

int vfs_open(const char *path, int flags) {
    const char *rel_path = NULL;
    mount_point_t *mp = find_mount_point(path, &rel_path);
    if (!mp) return -1;

    vfs_file_t fs_file = NULL;
    int res = mp->fs_strategy->ops->open(mp, &fs_file, rel_path, flags);
    if (res == 0) {
        int fd = alloc_fd(mp, fs_file);
        if (fd < 0) {
            mp->fs_strategy->ops->close(fs_file);
            return -2;
        }
        return fd;
    }
    return -3;
}

int vfs_read(int fd, void *buf, size_t len) {
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !s_fds[fd].in_use) return -1;
    return s_fds[fd].mp->fs_strategy->ops->read(s_fds[fd].fs_file, buf, len);
}

int vfs_write(int fd, const void *buf, size_t len) {
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !s_fds[fd].in_use) return -1;
    return s_fds[fd].mp->fs_strategy->ops->write(s_fds[fd].fs_file, buf, len);
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || !s_fds[fd].in_use) return -1;
    int res = s_fds[fd].mp->fs_strategy->ops->close(s_fds[fd].fs_file);
    s_fds[fd].in_use = false;
    return res;
}

int vfs_opendir(const char *path, vfs_dir_t *dir) {
    const char *rel_path = NULL;
    mount_point_t *mp = find_mount_point(path, &rel_path);
    if (!mp) return -1;
    return mp->fs_strategy->ops->opendir(mp, dir, rel_path);
}
