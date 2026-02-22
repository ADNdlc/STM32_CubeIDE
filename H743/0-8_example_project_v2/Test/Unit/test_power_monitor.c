#include "test_config.h"
#if ENABLE_TEST_POWER_MONITOR
#include "PowerMonitor_driver.h"
#include "PowerMonitor_factory.h"
#include "Sys.h"
#include "dev_map.h"
#include "test_framework.h"


static PowerMonitor_driver_t *pm_main;

static void test_power_monitor_setup(void) {
  pm_main = PowerMonitor_factory_get(POWER_MONITOR_ID_MAIN);
  if (pm_main == NULL) {
    log_e("PowerMonitor ID_MAIN not found");
    return;
  }
  log_i("PowerMonitor Test Setup: Initialized.");
}

static void test_power_monitor_loop(void) {
  static uint32_t last_log_tick = 0;
  Power_Instant_Data_t instant;
  Power_Accumulated_Data_t accumulated;

  // 每 1s 打印一次数据
  if (sys_get_systick_ms() - last_log_tick >= 1000) {
    last_log_tick = sys_get_systick_ms();

    if (PM_READ_INSTANT(pm_main, &instant) == 0) {
      log_i("INA219: %.2f mV, %.2f mA, %.2f mW", instant.voltage_mV,
            instant.current_mA, instant.power_mW);
    }

    if (PM_READ_ACCUMULATED(pm_main, &accumulated) == 0) {
      log_i("Energy: %.4f mWh, Charge: %.4f mAh", accumulated.energy_mWh,
            accumulated.charge_mAh);
    }
  }
}

static void test_power_monitor_teardown(void) {
  log_i("PowerMonitor Test Teardown: Done.");
}

REGISTER_TEST(PowerMonitor, "Monitor V/I/P and Energy at 1Hz",
              test_power_monitor_setup, test_power_monitor_loop,
              test_power_monitor_teardown);

#endif
