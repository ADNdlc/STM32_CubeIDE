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
#define DEFAULT_CLOUD_DEVICE_ID ""
#define DEFAULT_CLOUD_DEVICE_SECRET ""

// Internal storage header
typedef struct {
  uint32_t magic;
  uint32_t length;
  uint32_t checksum;
  uint8_t data[0];
} internal_cfg_header_t;

static sys_config_t g_sys_config;

// Forward declarations
static int sys_config_internal_save(void);
static int sys_config_internal_load(void);

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

#include "stm32h7xx_hal.h"

static uint32_t calculate_checksum(const void *data, size_t size) {
  uint32_t checksum = 0;
  const uint8_t *p = (const uint8_t *)data;
  for (size_t i = 0; i < size; i++) {
    checksum += p[i];
  }
  return checksum;
}

static int sys_config_internal_save(void) {
  HAL_StatusTypeDef status;
  FLASH_EraseInitTypeDef erase_init;
  uint32_t sector_error;

  HAL_FLASH_Unlock();

  // Erase Sector 7 of Bank 2 (0x081E0000)
  erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase_init.Banks = FLASH_BANK_2;
  erase_init.Sector = FLASH_SECTOR_7;
  erase_init.NbSectors = 1;
  erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
  if (status != HAL_OK) {
    log_e("Internal Flash Erase Failed: %d", status);
    HAL_FLASH_Lock();
    return -1;
  }

  // Prepare header and data
  internal_cfg_header_t header;
  header.magic = SYS_CFG_MAGIC;
  header.length = sizeof(sys_config_t);
  header.checksum = calculate_checksum(&g_sys_config, sizeof(sys_config_t));

  // Program header (must be 32-byte aligned for H7)
  uint8_t buffer[256 + 32] = {0}; // Enough for header + config + padding
  memcpy(buffer, &header, sizeof(header));
  memcpy(buffer + sizeof(header), &g_sys_config, sizeof(sys_config_t));

  // Program in 32-byte chunks (Flash Word)
  uint32_t addr = SYS_CFG_INTERNAL_ADDR;
  for (uint32_t i = 0; i < sizeof(buffer); i += 32) {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr + i,
                               (uint32_t)((uintptr_t)buffer + i));
    if (status != HAL_OK) {
      log_e("Internal Flash Program Failed at 0x%08X: %d", addr + i, status);
      HAL_FLASH_Lock();
      return -1;
    }
  }

  HAL_FLASH_Lock();
  log_i("Internal config saved to on-chip flash.");
  return 0;
}

static int sys_config_internal_load(void) {
  const internal_cfg_header_t *header =
      (const internal_cfg_header_t *)SYS_CFG_INTERNAL_ADDR;

  if (header->magic != SYS_CFG_MAGIC) {
    return -1;
  }

  if (header->length != sizeof(sys_config_t)) {
    return -2;
  }

  uint32_t sum = calculate_checksum(
      (const void *)(SYS_CFG_INTERNAL_ADDR + sizeof(internal_cfg_header_t)),
      sizeof(sys_config_t));
  if (sum != header->checksum) {
    log_w("Internal config checksum mismatch!");
    return -3;
  }

  memcpy(&g_sys_config,
         (const void *)(SYS_CFG_INTERNAL_ADDR + sizeof(internal_cfg_header_t)),
         sizeof(sys_config_t));
  log_i("Internal config loaded from on-chip flash.");
  return 0;
}

int sys_config_init(void) {
  log_i("Loading system configuration...");

  sys_config_set_defaults(); // Load hardcoded defaults first

  static char buffer[MAX_CFG_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  bool loaded = false;
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
    log_w("Config file error, trying internal flash...");
    if (sys_config_internal_load() != 0) {
      log_w("Internal flash config not found, using hardcoded defaults.");
      // defaults already set at the beginning
      sys_config_internal_save(); // Factory reset internal flash
    }
    sys_config_save(); // Repair/Create the JSON file
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
