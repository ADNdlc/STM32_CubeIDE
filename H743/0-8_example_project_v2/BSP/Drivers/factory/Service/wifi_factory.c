#include "wifi_factory.h"
#include "esp_8266/esp8266_wifi_driver.h"
#include "esp_8266/at_controller.h"
#include "usart_factory.h"
#include "gpio_factory.h"
#include <stddef.h>

// 静态实例管理
static esp8266_wifi_driver_t esp_wifi_drv;
static at_controller_t at_ctrl;
static bool initialized = false;

// 模拟 UART 队列缓冲区 (实际生产中可能由 dev_map 或专有逻辑提供)
static uint8_t at_tx_buf[256];
static uint8_t at_rx_buf[512];
static uart_queue_t at_uart_q;

wifi_driver_t *wifi_driver_get(wifi_id_t id) {
    if (id >= WIFI_MAX_DEVICES) return NULL;

    if (!initialized) {
        // 1. 获取底层硬件资源 (根据 dev_map.h)
        usart_driver_t *u_drv = usart_driver_get(USART_ID_ESP8266);
        gpio_driver_t *rst_pin = gpio_driver_get(GPIO_ID_ESP_RST); // 假设 ID
        
        if (u_drv) {
            // 2. 初始化 AT 控制器
            usart_hal_t *hal = usart_hal_create(u_drv);
            uart_queue_init(&at_uart_q, hal, at_tx_buf, sizeof(at_tx_buf), at_rx_buf, sizeof(at_rx_buf));
            uart_queue_start_receive(&at_uart_q);
            at_controller_init(&at_ctrl, &at_uart_q, rst_pin);
            
            // 3. 初始化 WiFi 驱动
            esp8266_wifi_driver_init(&esp_wifi_drv, &at_ctrl);
            initialized = true;
        }
    }

    return (wifi_driver_t *)&esp_wifi_drv;
}

// 供其他 Factory 使用 AT 控制器 (Internal)
at_controller_t* wifi_factory_get_at_controller(void) {
    if (!initialized) wifi_driver_get(WIFI_ID_MAIN);
    return &at_ctrl;
}
