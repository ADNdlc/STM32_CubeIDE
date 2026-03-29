#include "net_mgr.h"
#include "Sys.h"
#include "cloud_bridge.h"
#include "mqtt_factory.h"
#include "mqtt_service/adapters/onenet_adapter.h"
#include "mqtt_service/mqtt_service.h"
#include "sntp_factory.h"
#include "sntp_service/sntp_service.h"
#include "sys_config.h"
#include "sys_state.h"
#include "wifi_factory.h"
#include "wifi_service/wifi_service.h"
#include <string.h>

#include "elog.h"
#define LOG_TAG "NET_MGR"

#define MQTT_RECONNECT_WAIT_TIME 5000

// WiFi 内部状态机定义
typedef enum {
  NET_MGR_WIFI_STATE_IDLE = 0,
  NET_MGR_WIFI_STATE_INIT_MODE,
  NET_MGR_WIFI_STATE_CONNECTING,
  NET_MGR_WIFI_STATE_WAIT_IP,
  NET_MGR_WIFI_STATE_WAIT_STABLE,
  NET_MGR_WIFI_STATE_CONNECTED,
  NET_MGR_WIFI_STATE_RECONNECT_WAIT
} net_mgr_wifi_state_t;

// MQTT 内部状态机定义
typedef enum {
  NET_MGR_MQTT_STATE_IDLE = 0,
  NET_MGR_MQTT_STATE_STARTING,
  NET_MGR_MQTT_STATE_RUNNING,
  NET_MGR_MQTT_STATE_RECONNECT_WAIT
} net_mgr_mqtt_state_t;

static wifi_service_t g_wifi_svc;             // WiFi服务实例
static mqtt_service_t g_mqtt_svc;             // MQTT服务实例
static sntp_service_t g_sntp_svc;             // SNTP服务
extern const mqtt_adapter_t g_onenet_adapter; // OneNet MQTT适配器

#define SNTP_SERVER_DOMAIN "ntp.aliyun.com"

static bool g_wifi_target_enabled = false; // 用户期望的使能状态
static net_mgr_wifi_state_t g_wifi_state = NET_MGR_WIFI_STATE_IDLE;
static net_mgr_mqtt_state_t g_mqtt_state = NET_MGR_MQTT_STATE_IDLE;
static uint32_t g_wifi_state_timer = 0;         // WiFi状态计时器
static uint32_t g_mqtt_state_timer = 0;         // MQTT状态计时器
static uint32_t g_retry_count = 0;         // 重试计数
static uint32_t g_mqtt_retry_count = 0;    // MQTT 特定重试计数库
static bool g_pending_config_save = false; // 是否需要持久化配置

/*****************
 * 内部辅助函数
 *****************/
static void net_mgr_set_wifi_state(net_mgr_wifi_state_t new_state) {
  if (g_wifi_state != new_state) {
    log_i("WiFi State: %d -> %d", g_wifi_state, new_state);
    g_wifi_state = new_state;
    g_wifi_state_timer = sys_get_systick_ms();
  }
}

static void net_mgr_set_mqtt_state(net_mgr_mqtt_state_t new_state) {
  if (g_mqtt_state != new_state) {
    log_i("MQTT State: %d -> %d", g_mqtt_state, new_state);
    g_mqtt_state = new_state;
    g_mqtt_state_timer = sys_get_systick_ms();
  }
}

/*****************
 * 内部状态与回调
 *****************/
#if NETWORK_SERVICE_ENABLE
static void wifi_event_handler(wifi_service_t *svc, wifi_status_t status,
                               void *user_data) {
  log_i("WiFi raw status: %d (cur_mgr_state: %d)", status, g_wifi_state);

  if (status == WIFI_STATUS_GOT_IP) {
    if (g_wifi_state == NET_MGR_WIFI_STATE_WAIT_IP ||
        g_wifi_state == NET_MGR_WIFI_STATE_RECONNECT_WAIT) {
      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_WAIT_STABLE);
    }
  } else if (status == WIFI_STATUS_DISCONNECTED) {
    if (g_wifi_state == NET_MGR_WIFI_STATE_WAIT_STABLE ||
        g_wifi_state == NET_MGR_WIFI_STATE_CONNECTED) {

      log_w("WiFi dropped unexpectedly!");
      sys_state_set_wifi(false);
      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_RECONNECT_WAIT);
    }
  }
}

static void mqtt_event_handler(mqtt_service_t *svc,
                               const mqtt_drv_event_t *event, void *user_data) {
  if (g_wifi_state != NET_MGR_WIFI_STATE_CONNECTED) {
    return; // 忽略 WiFi 未好时的 MQTT 回调
  }

  if (event->type == MQTT_DRV_EVENT_CONNECTED) {
    log_i("MQTT Connected Event received");
    g_mqtt_retry_count = 0; // 重置计数器
    net_mgr_set_mqtt_state(NET_MGR_MQTT_STATE_RUNNING);
  } else if (event->type == MQTT_DRV_EVENT_DISCONNECTED) {
    log_w("MQTT Disconnected Event received");
    if (g_mqtt_state == NET_MGR_MQTT_STATE_RUNNING || g_mqtt_state == NET_MGR_MQTT_STATE_STARTING) {
      g_mqtt_retry_count++;
      if (g_mqtt_retry_count <= 3) {
        log_w("MQTT retrying %lu/3...", g_mqtt_retry_count);
        net_mgr_set_mqtt_state(NET_MGR_MQTT_STATE_RECONNECT_WAIT);
      } else {
        log_e("MQTT retry failed 3 times, resetting WiFi!");
        g_mqtt_retry_count = 0;
        sys_state_set_wifi(false);
        wifi_svc_disconnect(&g_wifi_svc);
        net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_RECONNECT_WAIT);
      }
    }
  }
}

#endif

/*****************
 * 外部服务调用
 *****************/
void net_mgr_init(void) {
  log_i("Initializing Network Manager...");

#if NETWORK_SERVICE_ENABLE
  wifi_svc_init(&g_wifi_svc, wifi_driver_get(WIFI_ID_MAIN));
  wifi_service_register_callback(&g_wifi_svc, wifi_event_handler, NULL);
#endif

#if SNTP_SERVICE_ENABLE
  sntp_svc_init(&g_sntp_svc, sntp_driver_get(SNTP_ID_MAIN));
  sntp_svc_set_config(&g_sntp_svc, 8, SNTP_SERVER_DOMAIN);
#endif

#if CLOUD_SERVICE_ENABLE
  const mqtt_adapter_t *adapter = &g_onenet_adapter;
  // ... 配置加载逻辑保持原样(略，见下文完整合入)
  onenet_config_t onenet_cfg;
  memset(&onenet_cfg, 0, sizeof(onenet_config_t));
  strncpy(onenet_cfg.product_id, sys_config_get_cloud_product_id(),
          sizeof(onenet_cfg.product_id) - 1);
  strncpy(onenet_cfg.device_id, sys_config_get_cloud_device_id(),
          sizeof(onenet_cfg.device_id) - 1);
  strncpy(onenet_cfg.device_secret, sys_config_get_cloud_device_secret(),
          sizeof(onenet_cfg.device_secret) - 1);
  onenet_adapter_init(&onenet_cfg);

  mqtt_svc_init(&g_mqtt_svc, mqtt_driver_get(MQTT_ID_MAIN), adapter);
  mqtt_svc_register_callback(&g_mqtt_svc, mqtt_event_handler, NULL);
  cloud_bridge_init(&g_mqtt_svc);
#endif

  g_wifi_state = NET_MGR_WIFI_STATE_IDLE;
  g_mqtt_state = NET_MGR_MQTT_STATE_IDLE;
  log_i("Network Manager initialized (IDLE)");
}

void net_mgr_process(void) {
  uint32_t now = sys_get_systick_ms();

  // 1. 底层服务处理
#if NETWORK_SERVICE_ENABLE
  wifi_svc_process(&g_wifi_svc);
#endif
#if SNTP_SERVICE_ENABLE
  sntp_svc_process(&g_sntp_svc);
#endif
#if CLOUD_SERVICE_ENABLE
  mqtt_svc_process(&g_mqtt_svc);
  cloud_bridge_process();
#endif

  // 2. WiFi 状态机处理
  if (!g_wifi_target_enabled) {
    if (g_wifi_state != NET_MGR_WIFI_STATE_IDLE) {
      wifi_svc_disconnect(&g_wifi_svc);
      sys_state_set_wifi(false);
      mqtt_svc_disconnect(&g_mqtt_svc);
      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_IDLE);
      net_mgr_set_mqtt_state(NET_MGR_MQTT_STATE_IDLE);
    }
    return;
  }

  switch (g_wifi_state) {
  case NET_MGR_WIFI_STATE_IDLE:
    net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_INIT_MODE);
    break;

  case NET_MGR_WIFI_STATE_INIT_MODE:
    if (now - g_wifi_state_timer > 500) {
      log_i("Setting WiFi Mode: Station");
      wifi_svc_set_mode(&g_wifi_svc, WIFI_MODE_STATION);
      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_CONNECTING);
    }
    break;

  case NET_MGR_WIFI_STATE_CONNECTING:
    if (now - g_wifi_state_timer > 500) {
      const char *ssid = sys_config_get_wifi_ssid();
      const char *pwd = sys_config_get_wifi_password();

      if (ssid == NULL || strlen(ssid) == 0) {
        log_w("No WiFi SSID configured. Waiting...");
        net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_RECONNECT_WAIT);
        break;
      }

      log_i("Connecting to AP: %s", ssid);
      wifi_svc_connect(&g_wifi_svc, ssid, pwd);
      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_WAIT_IP);
    }
    break;

  case NET_MGR_WIFI_STATE_WAIT_IP:
    if (now - g_wifi_state_timer > 30000) {
      log_e("Timeout waiting for IP. Retrying...");
      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_RECONNECT_WAIT);
    }
    break;

  case NET_MGR_WIFI_STATE_WAIT_STABLE:
    if (now - g_wifi_state_timer > 2000) {
      log_i("Network stable. Ready to start services.");
      sys_state_set_wifi(true);

      if (g_pending_config_save) {
        log_i("WiFi connection successful, persisting credentials...");
        sys_config_save();
        g_pending_config_save = false;
      }

      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_CONNECTED);
    }
    break;

  case NET_MGR_WIFI_STATE_CONNECTED:
    // WiFi正常运行，挂起由 MQTT 接管
    break;

  case NET_MGR_WIFI_STATE_RECONNECT_WAIT:
    if (now - g_wifi_state_timer > 5000) {
      log_i("Retry cooldown finished. Attempting reconnect...");
      net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_INIT_MODE);
      g_retry_count++;
    }
    break;
  }

  // 3. MQTT 状态机处理 (依赖 WiFi 状态)
  if (g_wifi_state != NET_MGR_WIFI_STATE_CONNECTED) {
    if (g_mqtt_state != NET_MGR_MQTT_STATE_IDLE) {
      mqtt_svc_disconnect(&g_mqtt_svc);
      net_mgr_set_mqtt_state(NET_MGR_MQTT_STATE_IDLE);
    }
  } else {
    switch (g_mqtt_state) {
    case NET_MGR_MQTT_STATE_IDLE:
      log_i("WiFi is connected, starting MQTT...");
      mqtt_svc_connect(&g_mqtt_svc);
      net_mgr_set_mqtt_state(NET_MGR_MQTT_STATE_STARTING);
      break;

    case NET_MGR_MQTT_STATE_STARTING:
      // 等待 MQTT EVENT CONNECTED 触发转变为 RUNNING
      break;

    case NET_MGR_MQTT_STATE_RUNNING:
      // MQTT 正常运行中，由服务侧自动维持心跳
      break;

    case NET_MGR_MQTT_STATE_RECONNECT_WAIT:
      if (now - g_mqtt_state_timer > MQTT_RECONNECT_WAIT_TIME) {
        log_i("Attempting MQTT reconnect (%lu/3)...", g_mqtt_retry_count);
        mqtt_svc_connect(&g_mqtt_svc);
        net_mgr_set_mqtt_state(NET_MGR_MQTT_STATE_STARTING);
      }
      break;
    }
  }
}

void net_mgr_wifi_enable(bool enable) {
  if (enable == g_wifi_target_enabled)
    return;
  g_wifi_target_enabled = enable;
  log_i("WiFi target state changed to: %s", enable ? "ENABLED" : "DISABLED");
}

int net_mgr_wifi_connect_manual(const char *ssid, const char *pwd) {
  log_i("Manual WiFi config updating: SSID=%s", ssid);
  sys_config_set_wifi_ssid(ssid);
  sys_config_set_wifi_password(pwd);
  // 【优化】：此处不再直接 save，标记等待连接成功后再 save
  g_pending_config_save = true;

  // 强制重置状态机以应用新配置
  g_wifi_target_enabled = true;
  net_mgr_set_wifi_state(NET_MGR_WIFI_STATE_INIT_MODE);
  return 0;
}

bool net_mgr_wifi_is_enabled(void) { return g_wifi_target_enabled; }

int net_mgr_wifi_scan(wifi_scan_cb_t cb, void *arg) {
  return wifi_svc_scan(&g_wifi_svc, cb, arg);
}
