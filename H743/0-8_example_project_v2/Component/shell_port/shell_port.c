/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief
 * @version 0.1
 * @date 2019-02-22
 *
 * @copyright (c) 2019 Letter
 *
 */

#ifdef FREERTOS_ENABLED
#include "FreeRTOS.h"
#include "elog.h"
#include "semphr.h"
#include "task.h"
#endif

#include "BSP_init.h"
#include "elog.h"
#include "shell.h"
#include "shell_fs.h"
#include "uart_queue/uart_queue.h"
#include "vfs_manager.h"
#include <string.h>

Shell shell;
char shellBuffer[512];
ShellFs shellFs;
char shellPathBuffer[VFS_MAX_PATH_LEN] = "/";

#ifdef FREERTOS_ENABLED
static SemaphoreHandle_t shellMutex;
#endif

/**
 * @brief 用户shell写
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 实际写入的数据长度
 */
short userShellWrite(char *data, unsigned short len) {
  if (g_debug_queue) {
    return uart_queue_send(g_debug_queue, (uint8_t *)data, len);
  }
  else {
    return 0;
  }
}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 实际读取到
 */
short userShellRead(char *data, unsigned short len) {
  if (g_debug_queue) {
    return uart_queue_getdata(g_debug_queue, (uint8_t *)data, 1);
  }
  else {
    return 0;
  }
}

#ifdef FREERTOS_ENABLED
/**
 * @brief 用户shell上锁
 *
 * @param shell shell
 *
 * @return int 0
 */
int userShellLock(Shell *shell) {
  xSemaphoreTakeRecursive(shellMutex, portMAX_DELAY);
  return 0;
}

/**
 * @brief 用户shell解锁
 *
 * @param shell shell
 *
 * @return int 0
 */
int userShellUnlock(Shell *shell) {
  xSemaphoreGiveRecursive(shellMutex);
  return 0;
}
#endif

// --- File System Adapter ---

static void normalize_path(char *path) {
  if (!path || path[0] == '\0')
    return;
  char *src = path, *dst = path;

  // 简化的归一化逻辑：仅处理重复的 '/'
  while (*src) {
    if (*src == '/' && *(src + 1) == '/') {
      src++;
      continue;
    }
    *dst++ = *src++;
  }
  *dst = '\0';

  // 移除末尾多余的 '/' (除非是根目录)
  if (dst > path + 1 && *(dst - 1) == '/') {
    *(dst - 1) = '\0';
  }
}

static size_t userShellGetCWD(char *path, size_t len) {
  strncpy(path, shellPathBuffer, len);
  return strlen(path);
}

static size_t userShellChDir(char *path) {
  char temp[VFS_MAX_PATH_LEN];
  if (path[0] == '/') {
    strncpy(temp, path, VFS_MAX_PATH_LEN);
  } else {
    snprintf(temp, VFS_MAX_PATH_LEN, "%s/%s", shellPathBuffer, path);
  }
  normalize_path(temp);

  // 处理根目录跳转
  if (strcmp(temp, "/") == 0) {
    strncpy(shellPathBuffer, temp, VFS_MAX_PATH_LEN);
    return 0;
  }

  vfs_dir_t dir;
  if (vfs_opendir(temp, &dir) == VFS_OK) {
    vfs_closedir(dir);
    strncpy(shellPathBuffer, temp, VFS_MAX_PATH_LEN);
    return 0;
  }
  return -1;
}

static size_t userShellListDir(char *path, char *buffer, size_t maxLen) {
  size_t offset = 0;
  buffer[0] = '\0';

  // 处理根目录虚拟列表（显示挂载点）
  if (strcmp(path, "/") == 0) {
    // 这里我们可以导出 vfs_manager 内部的挂载点，或者直接硬编码已知的
    // 简单起见，目前列出已挂载的 /sys
    offset += snprintf(buffer + offset, maxLen - offset, "%-20s %s\r\n", "sys",
                       "<DIR>");
    return offset;
  }

  vfs_dir_t dir;
  vfs_dirent_t dirent;
  if (vfs_opendir(path, &dir) != VFS_OK)
    return 0;

  while (vfs_readdir(dir, &dirent) > 0) {
    int len = snprintf(buffer + offset, maxLen - offset, "%-20s %s\r\n",
                       dirent.name, dirent.info.is_dir ? "<DIR>" : "");
    if (len < 0 || offset + len >= maxLen)
      break;
    offset += len;
  }
  vfs_closedir(dir);
  return offset;
}

/**
 * @brief 用户shell初始化
 *
 */
void userShellInit(void) {
#ifdef FREERTOS_ENABLED
  shellMutex = xSemaphoreCreateMutex();
  shell.lock = userShellLock;
  shell.unlock = userShellUnlock;
#endif
  shell.write = userShellWrite;
  shell.read = userShellRead;

  shellInit(&shell, shellBuffer, 512);

  // 初始化文件系统支持
  shellFs.getcwd = userShellGetCWD;
  shellFs.chdir = userShellChDir;
  shellFs.listdir = userShellListDir;
  shellFsInit(&shellFs, shellPathBuffer, VFS_MAX_PATH_LEN);
  shellSetPath(&shell, shellPathBuffer);
  shellCompanionAdd(&shell, SHELL_COMPANION_ID_FS, &shellFs);

#ifdef FREERTOS_ENABLED
  if (xTaskCreate(shellTask, "shell", 512, &shell, 5, NULL) != pdPASS) {
    log_e("shell task creat failed");
  }
#endif
}

#ifndef FREERTOS_ENABLED
/**
 * @brief 裸机环境下处理shell输入
 * 在main函数的while(1)循环中调用此函数
 */
void userShellProcess(void) {
  char data;
  if (shell.read && shell.read(&data, 1) == 1) {
    shellHandler(&shell, data);
  }
}
#endif

// --- Extra FS Commands ---

int shellCat(int argc, char *argv[]) {
  if (argc < 2) {
    shellWriteString(&shell, "Usage: cat <filename>\r\n");
    return -1;
  }
  char *filename = argv[1];

  static char fullPath[VFS_MAX_PATH_LEN]; // 使用静态缓冲区减少栈压力
  if (filename[0] == '/') {
    strncpy(fullPath, filename, VFS_MAX_PATH_LEN - 1);
  } else {
    snprintf(fullPath, VFS_MAX_PATH_LEN, "%s/%s", shellPathBuffer, filename);
  }
  normalize_path(fullPath);
  // 调试日志：确认路径是否正确
  log_d("cat attempting to open: [%s]", fullPath);
  int fd = vfs_open(fullPath, VFS_O_RDONLY);
  if (fd < 0) {
    shellPrint(&shell, "Error: Cannot open %s (fd=%d)\r\n", fullPath, fd);
    return -1;
  }

  char buf[128];
  int len;
  while ((len = vfs_read(fd, buf, sizeof(buf) - 1)) > 0) {
    buf[len] = '\0';
    shellWriteString(&shell, buf);
  }
  vfs_close(fd);
  shellWriteString(&shell, "\r\n");
  return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 cat, shellCat, cat file);

int shellMkdir(int argc, char *argv[]) {
  if (argc < 2) {
    shellWriteString(&shell, "Usage: mkdir <path>\r\n");
    return -1;
  }
  char *path = argv[1];
  char fullPath[VFS_MAX_PATH_LEN];
  if (path[0] == '/')
    strncpy(fullPath, path, VFS_MAX_PATH_LEN);
  else
    snprintf(fullPath, VFS_MAX_PATH_LEN, "%s/%s", shellPathBuffer, path);
  normalize_path(fullPath);
  if (vfs_mkdir(fullPath) != VFS_OK) {
    shellPrint(&shell, "Error: mkdir %s failed\r\n", fullPath);
    return -1;
  }
  return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 mkdir, shellMkdir, make directory);

int shellRm(int argc, char *argv[]) {
  if (argc < 2) {
    shellWriteString(&shell, "Usage: rm <path>\r\n");
    return -1;
  }
  char *path = argv[1];
  char fullPath[VFS_MAX_PATH_LEN];
  if (path[0] == '/')
    strncpy(fullPath, path, VFS_MAX_PATH_LEN);
  else
    snprintf(fullPath, VFS_MAX_PATH_LEN, "%s/%s", shellPathBuffer, path);
  normalize_path(fullPath);
  if (vfs_unlink(fullPath) != VFS_OK) {
    shellPrint(&shell, "Error: rm %s failed\r\n", fullPath);
    return -1;
  }
  return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 rm, shellRm, remove file or dir);

int shellFormat(int argc, char *argv[]) {
  if (argc < 2) {
    shellWriteString(&shell, "Usage: format <path>\r\n");
    return -1;
  }
  char *path = argv[1];
  char fullPath[VFS_MAX_PATH_LEN];
  if (path[0] == '/')
    strncpy(fullPath, path, VFS_MAX_PATH_LEN);
  else
    snprintf(fullPath, VFS_MAX_PATH_LEN, "%s/%s", shellPathBuffer, path);
  normalize_path(fullPath);
  if (vfs_format(fullPath, NULL) != VFS_OK) {
    shellPrint(&shell, "Error: format %s failed\r\n", fullPath);
    return -1;
  }
  return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 format, shellFormat, format partition);
