#include "sys_config.h"
#include "flash_handler.h"
#include "util/cJSON.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "SYS_CFG"
#include "elog.h"

#define SYS_CFG_FILE_PATH "/sys/sys_cfg.json"
#define MAX_CFG_BUFFER_SIZE 1024
// 默认wifi配置
#define DEFAULT_WIFI_SSID "test1"
#define DEFAULT_WIFI_PASSWORD "yu12345678"
// 默认云平台配置
#define DEFAULT_CLOUD_PLATFORM CLOUD_PLATFORM_ONENET
#define DEFAULT_CLOUD_PRODUCT_ID "SQKg9n0Ii0"
#define DEFAULT_CLOUD_DEVICE_ID "test123"
#define DEFAULT_CLOUD_DEVICE_SECRET "test234"

// Internal storage header
typedef struct {
  uint32_t magic;
  uint32_t length;
  uint32_t checksum;
  uint8_t data[0];
} internal_cfg_header_t;

static sys_config_t
    g_sys_config; /// 当前系统配置(初始化时从外部flash加载或使用默认值)

// Forward declarations
static int sys_config_internal_save(void);
static int sys_config_internal_load(void);

/**
 * @brief 将g_sys_config配置为默认值
 *
 */
static void sys_config_set_defaults(void) {
  memset(&g_sys_config, 0, sizeof(sys_config_t));

  // Default Network Settings
  strcpy(g_sys_config.net.ssid, DEFAULT_WIFI_SSID);
  strcpy(g_sys_config.net.password, DEFAULT_WIFI_PASSWORD);
  g_sys_config.net.auto_connect = true;

  // Default Cloud Settings (OneNet)
  g_sys_config.cloud.platform = DEFAULT_CLOUD_PLATFORM;
  strcpy(g_sys_config.cloud.product_id, DEFAULT_CLOUD_PRODUCT_ID);
  strcpy(g_sys_config.cloud.device_id, DEFAULT_CLOUD_DEVICE_ID);
  strcpy(g_sys_config.cloud.device_secret, DEFAULT_CLOUD_DEVICE_SECRET);
}

int sys_config_init(void) {
  log_i("Loading system configuration...");

  // 0. Always reset to defaults first to prevent partial/mixed state
  sys_config_set_defaults();

  static char buffer[MAX_CFG_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  bool loaded = false;
  // 1.尝试从外部文件系统加载配置文件
  int res = flash_handler_read(SYS_CFG_FILE_PATH, 0, (uint8_t *)buffer,
                               sizeof(buffer) - 1);
  if (res == 0) {
    cJSON *root = cJSON_Parse(buffer);
    if (root) {
      // Parse Network Config - Robust parsing: only update if key exists
      cJSON *net = cJSON_GetObjectItem(root, "network");
      if (net) {
        cJSON *ssid = cJSON_GetObjectItem(net, "ssid");
        cJSON *pwd = cJSON_GetObjectItem(net, "password");
        cJSON *auto_conn = cJSON_GetObjectItem(net, "auto_connect");

        if (ssid && ssid->valuestring)
          strncpy(g_sys_config.net.ssid, ssid->valuestring,
                  sizeof(g_sys_config.net.ssid) - 1);
        if (pwd && pwd->valuestring)
          strncpy(g_sys_config.net.password, pwd->valuestring,
                  sizeof(g_sys_config.net.password) - 1);
        if (auto_conn)
          g_sys_config.net.auto_connect = cJSON_IsTrue(auto_conn);
      }

      // Parse Cloud Config
      cJSON *cloud = cJSON_GetObjectItem(root, "cloud");
      if (cloud) {
        cJSON *plt = cJSON_GetObjectItem(cloud, "platform");
        cJSON *dev_id = cJSON_GetObjectItem(cloud, "device_id");
        cJSON *prod_id = cJSON_GetObjectItem(cloud, "product_id");
        cJSON *secret = cJSON_GetObjectItem(cloud, "device_secret");

        if (plt)
          g_sys_config.cloud.platform = (cloud_platform_t)plt->valueint;
        if (dev_id && dev_id->valuestring)
          strncpy(g_sys_config.cloud.device_id, dev_id->valuestring,
                  sizeof(g_sys_config.cloud.device_id) - 1);
        if (prod_id && prod_id->valuestring)
          strncpy(g_sys_config.cloud.product_id, prod_id->valuestring,
                  sizeof(g_sys_config.cloud.product_id) - 1);
        if (secret && secret->valuestring)
          strncpy(g_sys_config.cloud.device_secret, secret->valuestring,
                  sizeof(g_sys_config.cloud.device_secret) - 1);
      }
      cJSON_Delete(root);
      log_i("System config loaded from file.");
      loaded = true;
    } else {
      log_e("Failed to parse config JSON.");
    }
  }

  if (!loaded) {
    log_w("Config file error, use defaults...");
    sys_config_set_defaults(); // Set default values
    sys_config_save();         // Repair/Create the JSON file
  }

  return 0;
}

const sys_config_t *sys_config_get(void) { return &g_sys_config; }

void sys_config_set_net(const net_config_t *net) {
  if (net) {
    memcpy(&g_sys_config.net, net, sizeof(net_config_t));
  }
}

void sys_config_set_cloud(const cloud_config_t *cloud) {
  if (cloud) {
    memcpy(&g_sys_config.cloud, cloud, sizeof(cloud_config_t));
  }
}

int sys_config_save(void) {
  cJSON *root = cJSON_CreateObject();

  // Build Network JSON
  cJSON *net = cJSON_CreateObject();
  cJSON_AddStringToObject(net, "ssid", g_sys_config.net.ssid);
  cJSON_AddStringToObject(net, "password", g_sys_config.net.password);
  cJSON_AddBoolToObject(net, "auto_connect", g_sys_config.net.auto_connect);
  cJSON_AddItemToObject(root, "network", net);

  // Build Cloud JSON
  cJSON *cloud = cJSON_CreateObject();
  cJSON_AddNumberToObject(cloud, "platform",
                          (double)g_sys_config.cloud.platform);
  cJSON_AddStringToObject(cloud, "device_id", g_sys_config.cloud.device_id);
  cJSON_AddStringToObject(cloud, "product_id", g_sys_config.cloud.product_id);
  cJSON_AddStringToObject(cloud, "device_secret",
                          g_sys_config.cloud.device_secret);
  cJSON_AddItemToObject(root, "cloud", cloud);

  char *out = cJSON_PrintUnformatted(root);
  if (!out) {
    cJSON_Delete(root);
    return -1;
  }

  int res = flash_handler_write(SYS_CFG_FILE_PATH, 0, (const uint8_t *)out,
                                strlen(out));

  cJSON_free(out);
  cJSON_Delete(root);

  if (res == 0) {
    log_i("System config saved to flash.");
  } else {
    log_e("Failed to save system config to flash.");
  }

  return res;
}
