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
 
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "shell.h"
#include "BSP_init.h"
#include "uart_queue/uart_queue.h"
#include "elog.h"


Shell shell;
char shellBuffer[512];

static SemaphoreHandle_t shellMutex;

/**
 * @brief 用户shell写
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际写入的数据长度
 */
short userShellWrite(char *data, unsigned short len)
{
    if (g_debug_queue) {
        return uart_queue_send(g_debug_queue, (uint8_t *)data, len);
    }
    return len;
}


/**
 * @brief 用户shell读
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际读取到
 */
short userShellRead(char *data, unsigned short len)
{
    if (g_debug_queue) {
        // 读取一个字节（带FreeRTOS超时阻塞）
        return uart_queue_getdata(g_debug_queue, (uint8_t *)data, 1);
    }
    return 0;
}

/**
 * @brief 用户shell上锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellLock(Shell *shell)
{
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
int userShellUnlock(Shell *shell)
{
    xSemaphoreGiveRecursive(shellMutex);
    return 0;
}

/**
 * @brief 用户shell初始化
 * 
 */
void userShellInit(void)
{
    shellMutex = xSemaphoreCreateMutex();

    shell.write = userShellWrite;
    shell.read = userShellRead;
    shell.lock = userShellLock;
    shell.unlock = userShellUnlock;
    shellInit(&shell, shellBuffer, 512);
    // 增加栈大小至 512 words (2KB) 确保安全
    if (xTaskCreate(shellTask, "shell", 512, &shell, 5, NULL) != pdPASS)
    {
        log_e("shell task creat failed");
    }
}

