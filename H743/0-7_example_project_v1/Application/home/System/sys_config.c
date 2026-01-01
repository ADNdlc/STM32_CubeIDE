#include "sys_config.h"
#include "flash_handler.h"
#include "util/cJSON.h"
#include <stdio.h>
#include <string.h>


#define LOG_TAG "SYS_CFG"
#include "elog.h"

#define SYS_CFG_FILE_PATH "/sys/sys_cfg.json"
#define MAX_CFG_BUFFER_SIZE 1024

static sys_config_t g_sys_config;

static void sys_config_set_defaults(void) {
  memset(&g_sys_config, 0, sizeof(sys_config_t));

  // Default Network Settings
  strcpy(g_sys_config.net.ssid, "test1");
  strcpy(g_sys_config.net.password, "yu12345678");
  g_sys_config.net.auto_connect = true;

  // Default Cloud Settings (OneNet)
  g_sys_config.cloud.platform = CLOUD_PLATFORM_ONENET;
  strcpy(g_sys_config.cloud.product_id, "SQKg9n0Ii0");
  // device_id and device_secret should be empty or set via UI
}

int sys_config_init(void) {
  log_i("Loading system configuration...");

  static char buffer[MAX_CFG_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  int res = flash_handler_read(SYS_CFG_FILE_PATH, 0, (uint8_t *)buffer,
                               sizeof(buffer) - 1);
  if (res != 0) {
    log_w("Config file not found or read error, using defaults.");
    sys_config_set_defaults();
    return sys_config_save(); // Create default file
  }

  cJSON *root = cJSON_Parse(buffer);
  if (!root) {
    log_e("Failed to parse config JSON, using defaults.");
    sys_config_set_defaults();
    return -1;
  }

  // Parse Network Config
  cJSON *net = cJSON_GetObjectItem(root, "network");
  if (net) {
    cJSON *ssid = cJSON_GetObjectItem(net, "ssid");
    cJSON *pwd = cJSON_GetObjectItem(net, "password");
    cJSON *auto_conn = cJSON_GetObjectItem(net, "auto_connect");

    if (ssid)
      strncpy(g_sys_config.net.ssid, ssid->valuestring,
              sizeof(g_sys_config.net.ssid) - 1);
    if (pwd)
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
    if (dev_id)
      strncpy(g_sys_config.cloud.device_id, dev_id->valuestring,
              sizeof(g_sys_config.cloud.device_id) - 1);
    if (prod_id)
      strncpy(g_sys_config.cloud.product_id, prod_id->valuestring,
              sizeof(g_sys_config.cloud.product_id) - 1);
    if (secret)
      strncpy(g_sys_config.cloud.device_secret, secret->valuestring,
              sizeof(g_sys_config.cloud.device_secret) - 1);
  }

  cJSON_Delete(root);
  log_i("System config loaded successfully.");
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
