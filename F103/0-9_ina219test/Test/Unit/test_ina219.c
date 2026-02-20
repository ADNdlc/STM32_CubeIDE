#include "test_config.h"
#if 1 // 临时开启测试
#include "dev_map.h"
#include "i2c/stm32_i2c_driver.h"
#include "ina219/ina219_driver.h"
#include "test_framework.h"
#include "timer_factory.h"


static PowerMonitor_Dev_t *ina219_dev;
static timer_driver_t *timer;

// 定时器回调：仅负责触发采样请求 (上下文: 中断)
static void sampling_timer_callback(void *context) {
  if (ina219_dev) {
    ina219_request_sample(ina219_dev);
  }
}

static void test_ina219_setup(void) {
  // 1. 初始化 I2C (假设使用 I2C1)
  // 实际项目中应通过 Factory 或 Mapping 获取
  extern I2C_HandleTypeDef hi2c1;
  static stm32_i2c_driver_t i2c_drv_instance;
  // 手动构建个临时的 driver (或者复用 factory 里的)
  // 这里为了测试简便，手动 link
  // 实际应该用: i2c_driver_t *i2c = i2c_factory_get(I2C_BUS_SENSOR);

  // 假设 factory 还没完全就绪，我们先 mock 一个
  stm32_i2c_config_t i2c_cfg = {.is_soft = 0, .resource.hi2c = &hi2c1};
  i2c_driver_t *i2c_drv = stm32_i2c_driver_create(&i2c_cfg);

  // 2. 创建 INA219
  ina219_config_t ina_cfg = {.dev_addr = 0x80, // Default 0x40 << 1
                             .max_current_A = 2.0f,
                             .shunt_resistor_ohm = 0.1f};
  ina219_dev = ina219_create(i2c_drv, &ina_cfg);

  if (ina219_dev && PM_INIT(ina219_dev) == 0) {
    log_i("INA219 Initialized.");
  } else {
    log_e("INA219 Init Failed.");
  }

  // 3. 启动定时器 (100ms 采样一次)
  timer = timer_driver_get(TIMER_ID_1);
  if (timer) {
    TIMER_SET_CALLBACK(timer, sampling_timer_callback, NULL);
    TIMER_SET_PERIOD(timer, 100);
    TIMER_START(timer);
  }
}

static void test_ina219_loop(void) {
  static uint32_t last_log = 0;

  if (ina219_dev) {
    // 1. 处理累积数据 (从 Buffer 读出并计算)
    ina219_process_data(ina219_dev);

    // 2. 每秒打印一次状态
    if (HAL_GetTick() - last_log > 1000) {
      last_log = HAL_GetTick();

      Power_Instant_Data_t instant;
      Power_Accumulated_Data_t accum;

      PM_READ_INSTANT(ina219_dev, &instant);
      PM_READ_ACCUMULATED(ina219_dev, &accum);

      log_i("V: %.2fmV, I: %.2fmA, Charge: %.4fmAh", instant.voltage_mV,
            instant.current_mA, accum.charge_mAh);
    }
  }
}

static void test_ina219_teardown(void) {
  if (timer)
    TIMER_STOP(timer);
}

REGISTER_TEST(INA219, "INA219 Power Monitor Test", test_ina219_setup,
              test_ina219_loop, test_ina219_teardown);

#endif
