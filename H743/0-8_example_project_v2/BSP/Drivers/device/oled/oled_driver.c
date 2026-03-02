/*
 * oled_driver.c
 *
 *  Created on: Mar 2, 2026
 *      Author: 12114
 */

#include "oled_driver.h"
#include <string.h>

void OLED_Init(oled_device_t *dev) {
    if (dev->ops->init) {
        dev->ops->init(dev);
    }
    OLED_Clear(dev);
    OLED_Flush(dev);
}

void OLED_Clear(oled_device_t *dev) {
    uint32_t buf_size = (dev->width * dev->height) / 8;
    memset(dev->frame_buffer, 0x00, buf_size);
}

void OLED_DrawPoint(oled_device_t *dev, int x, int y, uint8_t color) {
    if (x >= dev->width || y >= dev->height || x < 0 || y < 0) return;

    // 页寻址：y/8 算出在第几页。乘 width 定位到行，加 x 定位到列。
    uint32_t index = (y / 8) * dev->width + x;
    uint8_t bit_pos = y % 8; // 计算在字节中的第几位

    if (color) {
        dev->frame_buffer[index] |= (1 << bit_pos);  // 置1
    } else {
        dev->frame_buffer[index] &= ~(1 << bit_pos); // 清0
    }
}

// 核心：把 Frame Buffer 刷到硬件
void OLED_Flush(oled_device_t *dev) {
    uint8_t pages = dev->height / 8;

    for (uint8_t page = 0; page < pages; page++) {
        // 1. 设置芯片硬件光标到当前页行首
        dev->ops->set_cursor(dev, page, 0);

        // 2. 计算当前页在 RAM 中的起始指针
        uint8_t *page_data = &dev->frame_buffer[page * dev->width];

        // 3. 将这一整行像素数据（如 128 字节）一并发送给总线
        dev->bus->write(dev->bus, OLED_DATA, page_data, dev->width);
    }
}
