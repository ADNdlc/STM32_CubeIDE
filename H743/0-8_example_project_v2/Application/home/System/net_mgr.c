#include "net_mgr.h"
#include "cloud_bridge.h"
#include "device_mapping.h"
#include "wifi_factory.h"
#include "mqtt_factory.h"
#include "sntp_factory.h"
#include "mqtt_service.h"
#include "wifi_service.h"
#include "sntp_service/sntp_service.h"
#include "sys_config.h"
#include "sys_state.h"
#include <string.h>
#include <stdbool.h>

#define LOG_TAG "NET_MGR"
#include "elog.h"

static wifi_service_t g_wifi_svc;             // WiFi服务实例
static mqtt_service_t g_mqtt_svc;             // MQTT服务实例
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
  if (event->type == MQTT_SVC_EVENT_CONNECTED) {
    log_i("MQTT Connected Event received");
    sys_state_set_mqtt(true);
  } else if (event->type == MQTT_SVC_EVENT_DISCONNECTED) {
    log_w("MQTT Disconnected Event received");
    sys_state_set_mqtt(false);
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

  // 1. 初始化服务 (工厂会自动处理驱动和 AT 控制器)
  wifi_svc_init(&g_wifi_svc, WIFI_ID_MAIN);
  wifi_service_register_callback(&g_wifi_svc, wifi_event_handler, NULL);

  mqtt_svc_init(&g_mqtt_svc, MQTT_ID_MAIN, &g_onenet_adapter);
  mqtt_svc_register_callback(&g_mqtt_svc, mqtt_event_handler, NULL);
  cloud_bridge_init(&g_mqtt_svc);

  sntp_svc_init(&g_sntp_svc, sntp_driver_get(SNTP_ID_MAIN));

  log_i("Network Manager initialized successfully");
}

/**
 * @brief 网络管理器循环处理函数
 *
 */
void net_mgr_process(void) {
  // 1. 处理底层工厂工作 (AT 控制器等)
  wifi_factory_process();

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
    log_d("sys_config_get: ssid=%s, pwd=%s", sys_config_get_wifi_ssid(),
          sys_config_get_wifi_password());
    wifi_svc_set_mode(&g_wifi_svc, WIFI_MODE_STATION);
    wifi_svc_connect(
        &g_wifi_svc, sys_config_get_wifi_ssid(),
        sys_config_get_wifi_password()); // 使用配置里的信息进行连接
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

int net_mgr_wifi_scan(wifi_scan_cb_t cb, void *arg) {
  log_i("WiFi Scan requested...");
  return wifi_svc_scan(&g_wifi_svc, cb, arg);
}

int net_mgr_wifi_connect_manual(const char *ssid, const char *pwd) {
  log_i("Manual WiFi connect: SSID=%s", ssid);

  // 1. 更新系统配置
  sys_config_set_wifi_ssid(ssid);
  sys_config_set_wifi_password(pwd);

  // 2. 启用 WiFi 并尝试连接
  g_wifi_target_state = true;
  //wifi_svc_set_mode(&g_wifi_svc, WIFI_MODE_STATION);
  return wifi_svc_connect(&g_wifi_svc, sys_config_get_wifi_ssid(), sys_config_get_wifi_password());
}
