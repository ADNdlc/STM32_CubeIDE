#include "net_mgr.h"
#include "device/esp_8266/at_controller.h"
#include "device/esp_8266/esp8266_wifi_driver.h"
#include "device_mapping.h"
#include "gpio_factory.h"
#include "sys_config.h"
#include "sys_state.h"
#include "uart_queue/uart_queue.h"
#include "usart_factory.h"
#include "usart_hal/usart_hal.h"
#include "wifi_service/wifi_service.h"
#include <string.h>

#define LOG_TAG "NET_MGR"
#include "elog.h"

// --- Global Instances ---
static uint8_t at_tx_buf[512];
static uint8_t at_rx_buf[1024];

static uart_queue_t g_at_queue;
static at_controller_t g_at_ctrl;
static esp8266_wifi_driver_t g_esp_drv;
static wifi_service_t g_wifi_svc;

static bool g_wifi_target_state = false;

// --- Callbacks ---
static void wifi_event_handler(wifi_service_t *svc, wifi_status_t status,
                               void *user_data) {
  log_i("WiFi status changed: %d", status);

  // Sync with sys_state
  if (status == WIFI_STATUS_GOT_IP) {
    log_i("Network Ready: Obtained IP address");
    sys_state_set_wifi(true);
  } else if (status == WIFI_STATUS_DISCONNECTED) {
    log_w("Network Offline");
    sys_state_set_wifi(false);
  }
}

void net_mgr_init(void) {
  log_i("Initializing Network Manager...");

  // 1. Get Hardware Drivers
  usart_driver_t *usart_drv = usart_driver_get(USART_ATCMD);
  gpio_driver_t *rst_gpio = gpio_driver_get(GPIO_ESP_RST);
  if (!usart_drv) {
    log_e("Failed to get USART_ATCMD driver");
    return;
  }

  // 2. Setup UART Queue
  usart_hal_t *uart_hal = usart_hal_create(usart_drv);
  uart_queue_init(&g_at_queue, uart_hal, at_tx_buf, sizeof(at_tx_buf),
                  at_rx_buf, sizeof(at_rx_buf));
  uart_queue_start_receive(&g_at_queue);

  // 3. Initialize AT Controller
  at_controller_init(&g_at_ctrl, &g_at_queue, rst_gpio);

  // 4. Initialize ESP8266 WiFi Driver
  esp8266_wifi_driver_init(&g_esp_drv, &g_at_ctrl);

  // 5. Initialize WiFi Service
  wifi_service_init(&g_wifi_svc, (wifi_driver_t *)&g_esp_drv);
  wifi_service_register_callback(&g_wifi_svc, wifi_event_handler, NULL);

  log_i("Network Manager initialized successfully");
}

void net_mgr_process(void) {
  // 1. 底层驱动驱动 (Drive the AT state machine)
  at_controller_process(&g_at_ctrl);

  // 2. 服务层驱动 (Drive individual services)
  wifi_svc_process(&g_wifi_svc);

  // 未来此处可以驱动其他服务:
  // sntp_service_process(&g_sntp_svc);
  // mqtt_service_process(&g_mqtt_svc);
}

void net_mgr_wifi_enable(bool enable) {
  if (enable == g_wifi_target_state)
    return;
  g_wifi_target_state = enable;

  if (enable) {
    log_i("WiFi enabling requested...");
    const sys_config_t *cfg = sys_config_get();
    wifi_svc_set_mode(&g_wifi_svc, WIFI_MODE_STATION);
    wifi_svc_connect(&g_wifi_svc, cfg->net.ssid, cfg->net.password);
  } else {
    log_i("WiFi disabling requested...");
    wifi_svc_disconnect(&g_wifi_svc);
  }
}

bool net_mgr_wifi_is_enabled(void) { return g_wifi_target_state; }
