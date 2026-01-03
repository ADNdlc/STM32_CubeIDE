 #include "net_mgr.h"
#include "sys.h"
#include "sys_state.h"
#include <stdbool.h>
#include <stdint.h>


#define LOG_TAG "CONN_TEST"
#include "elog.h"

/**
 * @brief WiFi and MQTT Connectivity Test
 *
 * This test assumes sys_services_init() has already been called.
 * It enables WiFi and monitors the connection state until MQTT is connected.
 */
void connectivity_test_run(void) {
  log_i("Starting Connectivity Test (WiFi + MQTT)...");

  // 1. Enable WiFi (Uses existing configuration from sys_config)
  log_i("Requesting WiFi Connection...");
  net_mgr_wifi_enable(true);

  uint32_t start_time = sys_get_systick_ms();
  const uint32_t timeout_ms = 60000; // 60 seconds timeout
  bool wifi_ok = false;
  bool mqtt_ok = false;

  log_i("Waiting for connection (Timeout: %d s)...", timeout_ms / 1000);

  while (sys_get_systick_ms() - start_time < timeout_ms) {
    // Process background tasks
    net_mgr_process();

    // Monitor status
    const sys_state_t *state = sys_state_get();

    if (!wifi_ok && state->wifi_connected) {
      wifi_ok = true;
      log_i(">>> WiFi Connected! (took %d ms)",
            sys_get_systick_ms() - start_time);
    }

    if (!mqtt_ok && state->mqtt_connected) {
      mqtt_ok = true;
      log_i(">>> MQTT Connected! (took %d ms)",
            sys_get_systick_ms() - start_time);
      break; // Both connected
    }

    sys_delay_ms(100);
  }

  // 3. Report Results
  log_i("--- Connectivity Test Result ---");
  log_i("WiFi Status: %s", wifi_ok ? "PASSED" : "FAILED (TIMEOUT)");
  log_i("MQTT Status: %s", mqtt_ok ? "PASSED" : "FAILED (TIMEOUT)");

  if (wifi_ok && mqtt_ok) {
    log_i("OVERALL RESULT: SUCCESS");
  } else {
    log_e("OVERALL RESULT: FAILURE");
  }
  log_i("--------------------------------");
}
