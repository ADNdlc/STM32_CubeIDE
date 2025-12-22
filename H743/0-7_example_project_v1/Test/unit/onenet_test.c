#include "onenet_test.h"
#include "all_tests_config.h"
#include "elog.h"
#include "esp8266.h"
#include "onenet_port.h"
#include "rtc_hal/rtc_hal.h"
#include "uart_queue/uart_queue.h"
#include "usart_factory.h"


// TODO: 包含 OneNET SDK 头文件 (需用户拷贝 SDK 到 Component/onenet)
// #include "aiot_tm_api.h"

// WiFi 配置
#define WIFI_SSID "your_ssid"
#define WIFI_PWD "your_password"

// OneNET 产品与设备信息
#define ONENET_PRODUCT_ID "your_product_id"
#define ONENET_DEV_NAME "your_device_name"
#define ONENET_ACCESS_KEY "your_access_key"

static esp8266_t *s_esp = NULL;

void onenet_test_run(void) {
  log_i("OneNET Integration Test Start...");

  // 1. 获取串口与创建排队驱动
  usart_hal_t *huart = usart_hal_get_instance(USART_ATCMD);
  if (!huart) {
    log_e("Failed to get USART_ATCMD!");
    return;
  }

  // 这里假设已经有一个 uart_queue 实例或者是新创建
  // 实际项目中通常由工厂或初始化脚本创建
  // 为了演示，我们直接使用已有的 huart

  // 2. 创建 ESP8266 实例
  // 注意：需要确保 huart 已经绑定了 uart_queue
  // 这里简化处理，实际应该从工厂获取绑定的 queue
  // s_esp = esp8266_create(target_queue);

  log_w("Please ensure target_queue is passed to esp8266_create!");

  if (!s_esp) {
    log_e("ESP8266 instance not created!");
    return;
  }

  // 3. 绑定到 OneNET 适配层
  plat_tcp_set_device(s_esp);

  // 4. 初始化模块与连接 WiFi
  if (esp8266_init(s_esp) != 0) {
    log_e("ESP8266 init failed!");
    return;
  }

  log_i("Connecting to WiFi: %s...", WIFI_SSID);
  if (esp8266_wifi_connect(s_esp, WIFI_SSID, WIFI_PWD, 10000) != 0) {
    log_e("WiFi connect failed!");
    return;
  }
  log_i("WiFi connected.");

  // 5. OneNET 登录
  log_i("Logging into OneNET...");
  // int32_t ret = tm_init();
  // if (ret != 0) { ... }

  // ret = tm_login(ONENET_PRODUCT_ID, ONENET_DEV_NAME, ONENET_ACCESS_KEY, 0,
  // 5000); if (ret == 0) {
  //     log_i("OneNET Login Success.");
  // } else {
  //     log_e("OneNET Login Failed: %d", ret);
  // }

  // 6. 主循环
  log_i("Entering main loop...");
  while (1) {
    esp8266_process(s_esp);
    // tm_step(10);

    // 建议在这里喂狗或处理其他系统任务
    sys_delay_ms(1);
  }
}
