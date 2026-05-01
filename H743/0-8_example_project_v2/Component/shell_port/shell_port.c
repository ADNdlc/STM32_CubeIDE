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
#include "semphr.h"
#include "task.h"
#include "elog.h"
#endif

#include "shell.h"
#include "BSP_init.h"
#include "uart_queue/uart_queue.h"


Shell shell;
char shellBuffer[512];

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
        return uart_queue_getdata(g_debug_queue, (uint8_t *)data, 1);
    }
    return 0;
}

#ifdef FREERTOS_ENABLED
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
#endif

/**
 * @brief 用户shell初始化
 * 
 */
void userShellInit(void)
{
#ifdef FREERTOS_ENABLED
    shellMutex = xSemaphoreCreateMutex();
    shell.lock = userShellLock;
    shell.unlock = userShellUnlock;
#endif
    shell.write = userShellWrite;
    shell.read = userShellRead;

    shellInit(&shell, shellBuffer, 512);

#ifdef FREERTOS_ENABLED
    if (xTaskCreate(shellTask, "shell", 512, &shell, 5, NULL) != pdPASS)
    {
        log_e("shell task creat failed");
    }
#endif
}

#ifndef FREERTOS_ENABLED
/**
 * @brief 裸机环境下处理shell输入
 * 在main函数的while(1)循环中调用此函数
 */
void userShellProcess(void)
{
    char data;
    if (shell.read && shell.read(&data, 1) == 1)
    {
        shellHandler(&shell, data);
    }
}
#endif

