#include "oled_drv_ssd1306.h"

// 内部辅助函数：发送单条指令
static int ssd1306_send_cmd(oled_device_t *dev, uint8_t cmd) {
    return dev->bus->write(dev->bus, OLED_CMD, &cmd, 1);
}

// 1. 初始化指令序列
static int ssd1306_init(oled_device_t *dev) {
    // 根据高度动态计算参数 (兼容 128x64 和 128x32)
    uint8_t mux_ratio = dev->height - 1;
    uint8_t com_pin_cfg = (dev->height == 32) ? 0x02 : 0x12;

    uint8_t init_cmds[] = {
        0xAE,             // 1. 关闭显示
        0xD5, 0x80,       // 2. 设置时钟分频
        0xA8, mux_ratio,  // 3. 设置多路复用率 (动态)
        0xD3, 0x00,       // 4. 设置显示偏移
        0x40,             // 5. 设置起始行
        0x8D, 0x14,       // 6. 开启电荷泵
        0x20, 0x02,       // 7. 设置寻址模式 (页寻址模式 Page Addressing)
        0xA1,             // 8. 段重映射 (左右反转，正常显示0xA0)
        0xC8,             // 9. COM 扫描方向 (上下反转，正常显示0xC0)
        0xDA, com_pin_cfg,// 10. 设置 COM 硬件配置 (动态)
        0x81, 0x7F,       // 11. 设置对比度
        0xD9, 0xF1,       // 12. 设置预充电周期
        0xDB, 0x40,       // 13. 设置 VCOMH
        0xA4,             // 14. 全局显示开启
        0xA6,             // 15. 正常显示 (非反色)
        0xAF              // 16. 打开显示
    };

    // 批量发送初始化序列
    return dev->bus->write(dev->bus, OLED_CMD, init_cmds, sizeof(init_cmds));
}

// 2. 设置页寻址游标
static int ssd1306_set_cursor(oled_device_t *dev, uint8_t page, uint8_t col) {
    // 加上芯片列偏移 (SSD1306通常为0)
    col += dev->col_offset;

    uint8_t cmds[3] = {
        0xB0 + page,                // 页地址
        0x10 | ((col & 0xF0) >> 4), // 列高4位
        0x00 | (col & 0x0F)         // 列低4位
    };
    return dev->bus->write(dev->bus, OLED_CMD, cmds, sizeof(cmds));
}

static int ssd1306_set_contrast(oled_device_t *dev, uint8_t contrast) {
    uint8_t cmds[2] = {0x81, contrast};
    return dev->bus->write(dev->bus, OLED_CMD, cmds, sizeof(cmds));
}

static int ssd1306_display_on(oled_device_t *dev)
{
    return ssd1306_send_cmd(dev, 0xAF);
}

static int ssd1306_display_off(oled_device_t *dev)
{
    return ssd1306_send_cmd(dev, 0xAE);
}

// 导出操作接口表
const oled_chip_ops_t ssd1306_ops = {
    .init = ssd1306_init,
    .set_cursor = ssd1306_set_cursor,
    .set_contrast = ssd1306_set_contrast,
    .display_on = ssd1306_display_on,
    .display_off = ssd1306_display_off,
};
