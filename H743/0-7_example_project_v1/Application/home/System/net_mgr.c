#include "net_mgr.h"
#include "cloud_bridge.h"
#include "device_mapping.h"
#include "esp_8266/at_controller.h"
#include "esp_8266/esp8266_mqtt_driver.h"
#include "esp_8266/esp8266_sntp_driver.h"
#include "esp_8266/esp8266_wifi_driver.h"
#include "gpio_factory.h"
#include "mqtt_adapter.h"
#include "mqtt_driver.h"
#include "mqtt_service.h"
#include "sntp_service/sntp_service.h"
#include "sys_config.h"
#include "sys_state.h"
#include "uart_queue/uart_queue.h"
#include "usart_factory.h"
#include "usart_hal/usart_hal.h"
#include "wifi_service/wifi_service.h"
#include <string.h>
#include <time.h>

#define LOG_TAG "NET_MGR"
#include "elog.h"

static uint8_t at_tx_buf[512];  // AT控制器发送缓冲
static uint8_t at_rx_buf[1024]; // AT控制器接收缓冲

static uart_queue_t g_at_queue;               // AT命令UART队列
static at_controller_t g_at_ctrl;             // AT控制器实例
static esp8266_wifi_driver_t g_esp_drv;       // ESP8266 WiFi驱动实例
static wifi_service_t g_wifi_svc;             // WiFi服务实例
static esp8266_mqtt_driver_t g_mqtt_drv;      // ESP8266 MQTT驱动实例
static mqtt_service_t g_mqtt_svc;             // MQTT服务实例
static esp8266_sntp_driver_t g_sntp_drv;      // ESP8266 SNTP驱动
static sntp_service_t g_sntp_svc;             // SNTP服务
extern const mqtt_adapter_t g_onenet_adapter; // OneNet MQTT适配器

/*****************
 * 内部状态与回调
 *****************/
static bool g_wifi_target_state = false; // 管理器WiFi目标状态(非实际系统状态)

/**
 * @brief WiFi状态变化回调
 *
 * @param svc       WiFi服务实例
 * @param status    WiFi当前状态
 * @param user_data 用户数据指针
 */
static void wifi_event_handler(wifi_service_t *svc, wifi_status_t status,
                               void *user_data) {
  log_i("WiFi status changed: %d", status);

  // 同步给系统状态
  if (status == WIFI_STATUS_GOT_IP) { // 获得IP地址,网络准备就绪
    log_i("Network Ready: Obtained IP address");
    sys_state_set_wifi(true);

    // 连接上wifi, 自动启动MQTT服务连接
    log_i("Starting MQTT Service...");
    mqtt_svc_connect(&g_mqtt_svc);

    // 自动启动 SNTP 时间同步 (UTC+8)
    log_i("Starting SNTP Sync...");
    sntp_svc_set_network_ready(&g_sntp_svc, true);
    sntp_svc_start_sync(&g_sntp_svc, 8, 24 * 3600 * 1000);

  } else if (status == WIFI_STATUS_DISCONNECTED) { // 断开连接
    log_w("Network Offline");
    sys_state_set_wifi(false);
    sntp_svc_set_network_ready(&g_sntp_svc, false);
  }
}

/**
 * @brief MQTT状态变化回调
 */
static void mqtt_event_handler(mqtt_service_t *svc,
                               const mqtt_svc_event_t *event, void *user_data) {
  if (event->type == MQTT_SVC_EVENT_STATE_CHANGED) {
    log_i("MQTT status changed: %d", event->state);
    if (event->state == MQTT_SVC_STATE_CONNECTED) {
      sys_state_set_mqtt(true);
    } else {
      sys_state_set_mqtt(false);
    }
  }
}

/*****************
 * 外部服务调用
 *****************/
/**
 * @brief 初始化网络管理器和其所有底层依赖
 *
 */
void net_mgr_init(void) {
  log_i("Initializing Network Manager...");

  // 1. 获取底层驱动
  usart_driver_t *usart_drv = usart_driver_get(USART_ATCMD); // AT控制器串口驱动
  gpio_driver_t *rst_gpio =
      gpio_driver_get(GPIO_ESP_RST); // AT控制器复位引脚驱动
  if (!usart_drv) {
    log_e("Failed to get Hardware Driver");
    return;
  }

  // 2. 初始化UART队列
  usart_hal_t *uart_hal = usart_hal_create(usart_drv); // 创建一个串口
  uart_queue_init(&g_at_queue, uart_hal, at_tx_buf, sizeof(at_tx_buf),
                  at_rx_buf, sizeof(at_rx_buf)); // 初始化UART队列(全局对象)
  uart_queue_start_receive(&g_at_queue);

  // 3. 初始化AT控制器
  at_controller_init(&g_at_ctrl, &g_at_queue, rst_gpio);

  // 4. 初始化ESP8266 WiFi驱动
  esp8266_wifi_driver_init(&g_esp_drv, &g_at_ctrl);

  // 5. 初始化WiFi服务
  wifi_service_init(&g_wifi_svc, (wifi_driver_t *)&g_esp_drv);
  wifi_service_register_callback(&g_wifi_svc, wifi_event_handler, NULL);

  // 6. 初始化MQTT驱动与服务
  esp8266_mqtt_driver_init(&g_mqtt_drv, &g_at_ctrl);
  mqtt_svc_init(&g_mqtt_svc, (mqtt_driver_t *)&g_mqtt_drv, &g_onenet_adapter);
  mqtt_svc_register_callback(&g_mqtt_svc, mqtt_event_handler, NULL);
  cloud_bridge_init(&g_mqtt_svc);

  // 7. 初始化 SNTP 驱动与服务
  esp8266_sntp_driver_init(&g_sntp_drv, &g_at_ctrl);
  sntp_svc_init(&g_sntp_svc, (sntp_driver_t *)&g_sntp_drv);

  log_i("Network Manager initialized successfully");
}

/**
 * @brief 网络管理器循环处理函数
 *
 */
void net_mgr_process(void) {
  // 1. 处理AT控制器层
  at_controller_process(&g_at_ctrl);

  // 2. 处理服务层
  wifi_svc_process(&g_wifi_svc);
  mqtt_svc_process(&g_mqtt_svc);
  sntp_svc_process(&g_sntp_svc);
  cloud_bridge_process();
}

/****************
 * WiFi服务控制
 ****************/
void net_mgr_wifi_enable(bool enable) {
  if (enable == g_wifi_target_state)
    return;
  g_wifi_target_state = enable;

  if (enable) {
    log_i("WiFi enabling requested...");
    const sys_config_t *cfg = sys_config_get(); // 获取系统配置
    log_d("sys_config_get: ssid=%s, pwd=%s", cfg->net.ssid, cfg->net.password);
    wifi_svc_set_mode(&g_wifi_svc, WIFI_MODE_STATION);
    wifi_svc_connect(&g_wifi_svc, cfg->net.ssid,
                     cfg->net.password); // 使用配置里的信息进行连接
  } else {
    log_i("WiFi disabling requested...");
    wifi_svc_disconnect(&g_wifi_svc); // 断开连接
  }
}

/**
 * @brief wifi使能状态查询
 *
 * @return g_wifi_target_state true:启用,false:禁用
 */
bool net_mgr_wifi_is_enabled(void) { return g_wifi_target_state; }
