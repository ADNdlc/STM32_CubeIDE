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

// 内部状态机定义
typedef enum {
  NET_MGR_STATE_IDLE = 0,            // 0: 空闲/WiFi禁用
  NET_MGR_STATE_INIT_MODE,           // 1: 正在初始化模式
  NET_MGR_STATE_CONNECTING,          // 2: 准备连接 AP
  NET_MGR_STATE_WAIT_IP,             // 3: 挂起等待获取 IP
  NET_MGR_STATE_WAIT_STABLE,         // 4: 已获取 IP，等待网络稳定期
  NET_MGR_STATE_READY,               // 5: 网络就绪，启动服务
  NET_MGR_STATE_RUNNING,             // 6: 正常运行中挂起态
  NET_MGR_STATE_MQTT_RECONNECT_WAIT, // 7: 等待重试连接MQTT
  NET_MGR_STATE_RECONNECT_WAIT       // 8: 断开连接，等待重试冷却
} net_mgr_internal_state_t;

static wifi_service_t g_wifi_svc;             // WiFi服务实例
static mqtt_service_t g_mqtt_svc;             // MQTT服务实例
static sntp_service_t g_sntp_svc;             // SNTP服务
extern const mqtt_adapter_t g_onenet_adapter; // OneNet MQTT适配器

#define SNTP_SERVER_DOMAIN "ntp.aliyun.com"

static bool g_wifi_target_enabled = false; // 用户期望的使能状态
static net_mgr_internal_state_t g_state = NET_MGR_STATE_IDLE;
static uint32_t g_state_timer = 0;         // 状态计时器
static uint32_t g_retry_count = 0;         // 重试计数
static uint32_t g_mqtt_retry_count = 0;    // MQTT 特定重试计数库
static bool g_pending_config_save = false; // 是否需要持久化配置

/*****************
 * 内部辅助函数
 *****************/
static void net_mgr_set_state(net_mgr_internal_state_t new_state) {
  if (g_state != new_state) {
    log_i("State: %d -> %d", g_state, new_state);
    g_state = new_state;
    g_state_timer = sys_get_systick_ms();
  }
}

/*****************
 * 内部状态与回调
 *****************/
#if NETWORK_SERVICE_ENABLE
static void wifi_event_handler(wifi_service_t *svc, wifi_status_t status,
                               void *user_data) {
  log_i("WiFi raw status: %d (cur_mgr_state: %d)", status, g_state);

  if (status == WIFI_STATUS_GOT_IP) {
    if (g_state == NET_MGR_STATE_WAIT_IP ||
        g_state == NET_MGR_STATE_RECONNECT_WAIT) {
      net_mgr_set_state(NET_MGR_STATE_WAIT_STABLE);
    }
  } else if (status == WIFI_STATUS_DISCONNECTED) {
    // 【精准判定】：只在稳定、就绪、运行这三个状态下遭遇断开，才算意外掉线
    if (g_state == NET_MGR_STATE_WAIT_STABLE ||
        g_state == NET_MGR_STATE_READY || g_state == NET_MGR_STATE_RUNNING) {

      log_w("WiFi dropped unexpectedly!");
      sys_state_set_wifi(false);
      mqtt_svc_disconnect(&g_mqtt_svc);
      net_mgr_set_state(NET_MGR_STATE_RECONNECT_WAIT);
    }
  }
}

static void mqtt_event_handler(mqtt_service_t *svc,
                               const mqtt_drv_event_t *event, void *user_data) {
  if (event->type == MQTT_DRV_EVENT_CONNECTED) {
    log_i("MQTT Connected Event received");
    g_mqtt_retry_count = 0; // 重置计数器
  } else if (event->type == MQTT_DRV_EVENT_DISCONNECTED) {
    log_w("MQTT Disconnected Event received");
    if (g_state == NET_MGR_STATE_RUNNING) {
      g_mqtt_retry_count++;
      if (g_mqtt_retry_count <= 3) {
        log_w("MQTT retrying %lu/3...", g_mqtt_retry_count);
        net_mgr_set_state(NET_MGR_STATE_MQTT_RECONNECT_WAIT);
      } else {
        log_e("MQTT retry failed 3 times, resetting WiFi!");
        g_mqtt_retry_count = 0;
        sys_state_set_wifi(false);
        wifi_svc_disconnect(&g_wifi_svc);
        net_mgr_set_state(NET_MGR_STATE_RECONNECT_WAIT);
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

  g_state = NET_MGR_STATE_IDLE;
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

  // 2. 状态机逻辑处理
  if (!g_wifi_target_enabled) {
    if (g_state != NET_MGR_STATE_IDLE) {
      wifi_svc_disconnect(&g_wifi_svc);
      sys_state_set_wifi(false);
      mqtt_svc_disconnect(&g_mqtt_svc);
      net_mgr_set_state(NET_MGR_STATE_IDLE);
    }
    return;
  }

  switch (g_state) {
  case NET_MGR_STATE_IDLE:
    net_mgr_set_state(NET_MGR_STATE_INIT_MODE);
    break;

  case NET_MGR_STATE_INIT_MODE:
    if (now - g_state_timer > 500) {
      log_i("Setting WiFi Mode: Station");
      wifi_svc_set_mode(&g_wifi_svc, WIFI_MODE_STATION);
      // 发送完立即进入下一状态，底层 AT 队列会自动排队，不会冲突
      net_mgr_set_state(NET_MGR_STATE_CONNECTING);
    }
    break;

  case NET_MGR_STATE_CONNECTING:
    if (now - g_state_timer > 500) {
      const char *ssid = sys_config_get_wifi_ssid();
      const char *pwd = sys_config_get_wifi_password();

      // 【保护】：如果没有配置SSID，阻止其发送非法空指令
      if (ssid == NULL || strlen(ssid) == 0) {
        log_w("No WiFi SSID configured. Waiting...");
        net_mgr_set_state(NET_MGR_STATE_RECONNECT_WAIT); // 进入等待重试
        break;
      }

      log_i("Connecting to AP: %s", ssid);
      wifi_svc_connect(&g_wifi_svc, ssid, pwd);

      // 【修复】：指令已下发给 AT 队列，立即进入挂起态等待回调，禁止再发！
      net_mgr_set_state(NET_MGR_STATE_WAIT_IP);
    }
    break;

  case NET_MGR_STATE_WAIT_IP:
    // 在此状态什么都不做，完全由 wifi_event_handler (回调) 驱动状态机
    // 如果 30 秒都拿不到 IP，强制重试防死锁
    if (now - g_state_timer > 30000) {
      log_e("Timeout waiting for IP. Retrying...");
      net_mgr_set_state(NET_MGR_STATE_RECONNECT_WAIT);
    }
    break;

  case NET_MGR_STATE_WAIT_STABLE:
    if (now - g_state_timer > 2000) {
      log_i("Network stable. Ready to start services.");
      net_mgr_set_state(NET_MGR_STATE_READY);
    }
    break;

  case NET_MGR_STATE_READY:
    log_i("Starting MQTT & SNTP...");
    sys_state_set_wifi(true);
    mqtt_svc_connect(&g_mqtt_svc);

    // 【新增】：如果是手动连接成功，此时持久化配置到 VFS
    if (g_pending_config_save) {
      log_i("WiFi connection successful, persisting credentials...");
      sys_config_save();
      g_pending_config_save = false;
    }

    // 【修复】：显式进入 RUNNING 状态
    net_mgr_set_state(NET_MGR_STATE_RUNNING);
    break;

  case NET_MGR_STATE_RUNNING:
    // 网络正常运行中，这里完全挂起，不需要做任何事情。
    // 顶部的 mqtt_svc_process() 会自动维持心跳和数据收发。
    break;

  case NET_MGR_STATE_MQTT_RECONNECT_WAIT:
    if (now - g_state_timer > MQTT_RECONNECT_WAIT_TIME) {
      log_i("Attempting MQTT reconnect (%lu/3)...", g_mqtt_retry_count);
      mqtt_svc_connect(&g_mqtt_svc);
      net_mgr_set_state(NET_MGR_STATE_RUNNING); // 触发连接后挂起等待回调
    }
    break;

  case NET_MGR_STATE_RECONNECT_WAIT:
    // 断开后等待 5 秒重试
    if (now - g_state_timer > 5000) {
      log_i("Retry cooldown finished. Attempting reconnect...");
      net_mgr_set_state(NET_MGR_STATE_INIT_MODE);
      g_retry_count++;
    }
    break;

  default:
    // 已就绪，监控网络状态
    break;
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
  net_mgr_set_state(NET_MGR_STATE_INIT_MODE);
  return 0;
}

bool net_mgr_wifi_is_enabled(void) { return g_wifi_target_enabled; }

int net_mgr_wifi_scan(wifi_scan_cb_t cb, void *arg) {
  return wifi_svc_scan(&g_wifi_svc, cb, arg);
}
