/*
 * wifi_test.c
 *
 *  Created on: Dec 22, 2025
 */

#include "all_tests_config.h"
#if _wifi_test_

#include "../../Component/uart_queue/uart_queue.h"
#include "../../Component/wifi_service/wifi_service.h"
#include "../../Drivers/SYSTEM/sys.h"
#include "../../Drivers/device/esp_8266/at_controller.h"
#include "../../Drivers/device/esp_8266/esp8266_wifi_driver.h"
#include "../../HAL/usart_hal/usart_hal.h"
#include "device_mapping.h"
#include "gpio_factory.h"
#include "usart_factory.h"
#include "wifi_test.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "wifi_test"
#include "../../EasyLogger/easylogger/inc/elog.h"

#define wifiSSID "test1"
#define wifiPassword "yu12345678"

// Static resources for AT controller
static uint8_t at_tx_buf[512];
static uint8_t at_rx_buf[1024];

static at_controller_t g_at_ctrl;
static esp8266_wifi_driver_t g_esp_drv;
static wifi_service_t g_wifi_svc;
static uart_queue_t g_at_queue;

static void wifi_event_handler(wifi_service_t *svc, wifi_status_t status,
                               void *user_data) {
  log_i("WiFi Status Changed: %d", status);
}

static bool g_scan_done = false;

static void test_scan_cb(void *arg, wifi_ap_info_t *results, uint16_t count) {
  log_i("Scan complete. Found %d APs:", count);
  for (uint16_t i = 0; i < count; i++) {
    log_i(" [%d] SSID: %-16s, RSSI: %-3d, MAC: %s, CH: %d, ECN: %d, Proto: "
          "0x%02X",
          i, results[i].ssid, results[i].rssi, results[i].mac,
          results[i].channel, results[i].encryption, results[i].protocol);
  }
  g_scan_done = true;
}

void wifi_test_run(void) {
  log_i("Starting WiFi Test...");

  // 1. Get Hardware Drivers
  usart_driver_t *usart_drv = usart_driver_get(USART_ATCMD);
  gpio_driver_t *rst_gpio = gpio_driver_get(GPIO_ESP_RST);

  if (!usart_drv) {
    log_e("Failed to get USART_ATCMD driver");
    return;
  }

  // 2. Setup UART Queue
  usart_hal_t *uart_hal = usart_hal_create(usart_drv);
  if (!uart_hal) {
    log_e("Failed to create UART HAL");
    return;
  }
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

  log_i("WiFi Service initialized. Waiting for module ready...");

  // Give some time for the process loop to handle reset/ready
  uint32_t start_tick = sys_get_systick_ms();
  while (sys_get_systick_ms() - start_tick < 10000) {
    at_controller_process(&g_at_ctrl);

    if (wifi_svc_get_status(&g_wifi_svc) != WIFI_STATUS_DISCONNECTED) {
      // If already connected or something, keep going
    }

    sys_delay_ms(1);
  }

  log_i("State after 10s: %d", g_at_ctrl.state);

  // 6. Perform Scan
  log_i("Scanning for APs...");
  g_scan_done = false;
  wifi_svc_scan(&g_wifi_svc, test_scan_cb, NULL);

  start_tick = sys_get_systick_ms();
  while (sys_get_systick_ms() - start_tick < 15000) { // Wait up to 15s for scan
    at_controller_process(&g_at_ctrl);
    if (g_scan_done)
      break;
    sys_delay_ms(1);
  }

  if (!g_scan_done) {
    log_w("Scan timeout or failed.");
  }

  // 7. Try Connect (Optional, if user wants to test specific AP)
  log_i("Attempting to connect to: %s", wifiSSID);
  wifi_svc_connect(&g_wifi_svc, wifiSSID, wifiPassword);

  start_tick = sys_get_systick_ms();
  wifi_status_t last_status = WIFI_STATUS_DISCONNECTED;
  while (sys_get_systick_ms() - start_tick < 30000) { // Wait up to 30s
    at_controller_process(&g_at_ctrl);

    wifi_status_t current_status = wifi_svc_get_status(&g_wifi_svc);
    if (current_status != last_status) {
      log_i("Current WiFi Status: %d", current_status);
      last_status = current_status;
      if (current_status == WIFI_STATUS_GOT_IP) {
        log_i("WiFi Successfully connected and obtained IP!");
        break;
      }
    }
    sys_delay_ms(1);
  }

  log_i("WiFi Test cycle complete. Process loop continuing...");
}

#endif
