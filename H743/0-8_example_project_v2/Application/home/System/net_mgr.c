#include "net_mgr.h"
#include "cloud_bridge.h"
#include "wifi_factory.h"
#include "mqtt_factory.h"
#include "sntp_factory.h"
#include "wifi_service/wifi_service.h"
#include "sntp_service/sntp_service.h"
#include "mqtt_service/mqtt_service.h"
#include "mqtt_service/adapters/onenet_adapter.h"
#include "sys_config.h"
#include <string.h>



#include "elog.h"
#define LOG_TAG "NET_MGR"

static wifi_service_t g_wifi_svc;             // WiFi服务实例
static mqtt_service_t g_mqtt_svc;             // MQTT服务实例
static sntp_service_t g_sntp_svc;             // SNTP服务
extern const mqtt_adapter_t g_onenet_adapter; // OneNet MQTT适配器

#define SNTP_SERVER_DOMAIN "ntp.aliyun.com"

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
#if NETWORK_SERVICE_ENABLE
static void wifi_event_handler(wifi_service_t *svc, wifi_status_t status,
                               void *user_data) {
  log_i("WiFi status changed: %d", status);

  // 同步给系统状态
  if (status == WIFI_STATUS_GOT_IP) { // 获得IP地址,网络准备就绪
    log_i("Network Ready: Obtained IP address");

    // 连接上wifi, 自动启动MQTT服务连接
    log_i("Starting MQTT Service...");
    mqtt_svc_connect(&g_mqtt_svc);

    // SNTP 会在 sntp_svc_process 中无阻塞地等待底层 +TIME_UPDATED 后自动同步
    log_i("SNTP Sync is armed automatically...");
    // 移除提前调用的 sntp_svc_start_sync(&g_sntp_svc); 防止报错

  } else if (status == WIFI_STATUS_DISCONNECTED) { // 断开连接
    log_w("Network Offline");
    mqtt_svc_disconnect(&g_mqtt_svc);
  }
}
#endif
#if CLOUD_SERVICE_ENABLE
/**
 * @brief MQTT状态变化回调
 */
static void mqtt_event_handler(mqtt_service_t *svc,
                               const mqtt_drv_event_t *event, void *user_data) {
  if (event->type == MQTT_DRV_EVENT_CONNECTED) {
    log_i("MQTT Connected Event received");

  } else if (event->type == MQTT_DRV_EVENT_DISCONNECTED) {
    log_w("MQTT Disconnected Event received");

  }
}
#endif

/*****************
 * 外部服务调用
 *****************/
/**
 * @brief 初始化网络管理器和其所有底层依赖
 *
 */
void net_mgr_init(void) {
  log_i("Initializing Network Manager...");

#if NETWORK_SERVICE_ENABLE // 初始化wifi服务
  wifi_svc_init(&g_wifi_svc, wifi_driver_get(WIFI_ID_MAIN));
  wifi_svc_set_mode(&g_wifi_svc, WIFI_MODE_STATION);
  wifi_service_register_callback(&g_wifi_svc, wifi_event_handler, NULL);
#endif

#if SNTP_SERVICE_ENABLE // 初始化sntp服务
  sntp_svc_init(&g_sntp_svc, sntp_driver_get(SNTP_ID_MAIN));
  sntp_svc_set_config(&g_sntp_svc, 8, SNTP_SERVER_DOMAIN);
#endif

#if CLOUD_SERVICE_ENABLE // 初始化mqtt service
  const mqtt_adapter_t *adapter = NULL;
  int platform = sys_config_get_cloud_platform();

  if (platform == CLOUD_PLATFORM_ONENET) {
    adapter = &g_onenet_adapter;
    onenet_config_t onenet_cfg;
    memset(&onenet_cfg, 0, sizeof(onenet_config_t));
    strncpy(onenet_cfg.product_id, sys_config_get_cloud_product_id(), sizeof(onenet_cfg.product_id) - 1);
    strncpy(onenet_cfg.device_id, sys_config_get_cloud_device_id(), sizeof(onenet_cfg.device_id) - 1);
    strncpy(onenet_cfg.device_secret, sys_config_get_cloud_device_secret(), sizeof(onenet_cfg.device_secret) - 1);
    onenet_adapter_init(&onenet_cfg);
  } else {
    log_w("Unsupported cloud platform: %d, defaulting to OneNet", platform);
    adapter = &g_onenet_adapter;
    onenet_config_t onenet_cfg;
    memset(&onenet_cfg, 0, sizeof(onenet_config_t));
    strncpy(onenet_cfg.product_id, sys_config_get_cloud_product_id(), sizeof(onenet_cfg.product_id) - 1);
    strncpy(onenet_cfg.device_id, sys_config_get_cloud_device_id(), sizeof(onenet_cfg.device_id) - 1);
    strncpy(onenet_cfg.device_secret, sys_config_get_cloud_device_secret(), sizeof(onenet_cfg.device_secret) - 1);
    onenet_adapter_init(&onenet_cfg);
  }

  mqtt_svc_init(&g_mqtt_svc, mqtt_driver_get(MQTT_ID_MAIN), adapter);
  mqtt_svc_register_callback(&g_mqtt_svc, mqtt_event_handler, NULL);
  cloud_bridge_init(&g_mqtt_svc);
#endif

  log_i("Network Manager initialized successfully");
}

/**
 * @brief 网络管理器循环处理函数
 *
 */
void net_mgr_process(void) {
  // 服务层处理
#if NETWORK_SERVICE_ENABLE // 初始化wifi服务
  wifi_svc_process(&g_wifi_svc);
#endif
#if SNTP_SERVICE_ENABLE 
  sntp_svc_process(&g_sntp_svc);
#endif
#if CLOUD_SERVICE_ENABLE // 初始化mqtt服务
  mqtt_svc_process(&g_mqtt_svc);
  cloud_bridge_process();
#endif
}

/****************
 * WiFi服务控制
 ****************/
#if NETWORK_SERVICE_ENABLE // 初始化wifi服务
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
  sys_config_save(); // 手动连接后保存配置到文件系统
  return wifi_svc_connect(&g_wifi_svc, sys_config_get_wifi_ssid(), sys_config_get_wifi_password());
}
#endif
