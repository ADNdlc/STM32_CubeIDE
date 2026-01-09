#include "project_cfg.h"
#include "sys_config.h"
#include <stdio.h>
#include <string.h>
#if !USE_Simulator
#include "cJSON.h"
#include "flash_handler.h"
#include "sys.h"
#endif

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
#define DEFAULT_CLOUD_DEVICE_ID "test2"
#define DEFAULT_CLOUD_DEVICE_SECRET                                    \
  "version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2Ftest2&et=" \
  "1855499668539&method=md5&sign=%2FHVmg4Xz2RfTRWEu44mApQ%3D%3D"

// Internal storage header
typedef struct
{
  uint32_t magic;
  uint32_t length;
  uint32_t checksum;
  uint8_t data[0];
} internal_cfg_header_t;

static sys_config_t
    g_sys_config; // 当前系统配置(初始化时从外部flash加载或使用默认值)

/**
 * @brief 将g_sys_config配置为默认值
 *
 */
static void sys_config_set_defaults(void)
{
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

int sys_config_init(void)
{
  log_i("Loading system configuration...");

  sys_config_set_defaults();
// sys_config_save(); // test
#if USE_Simulator
  log_i("Simulator mode, use default config.");
  return 0; // 模拟器环境无flash设备
#else

  static char buffer[MAX_CFG_BUFFER_SIZE]; // 配置文件操作缓冲
  memset(buffer, 0, sizeof(buffer));

  bool loaded = false;
  // 1.尝试从外部文件系统加载配置文件
  int res = flash_handler_read(SYS_CFG_FILE_PATH, 0, (uint8_t *)buffer,
                               sizeof(buffer) - 1);
  if (res == 0)
  {
    cJSON *root = cJSON_Parse(buffer); // 创建JSON对象
    if (root)
    {
      // 解析网络配置
      cJSON *net = cJSON_GetObjectItem(root, "network"); // json网络对象
      if (net)
      {
        cJSON *ssid = cJSON_GetObjectItem(net, "ssid");              // WiFi名
        cJSON *pwd = cJSON_GetObjectItem(net, "password");           // WiFi密码
        cJSON *auto_conn = cJSON_GetObjectItem(net, "auto_connect"); // 是否自动连接
        // 有值则复制到当前配置
        if (ssid && ssid->valuestring)
          strncpy(g_sys_config.net.ssid, ssid->valuestring,
                  sizeof(g_sys_config.net.ssid) - 1);
        if (pwd && pwd->valuestring)
          strncpy(g_sys_config.net.password, pwd->valuestring,
                  sizeof(g_sys_config.net.password) - 1);
        if (auto_conn)
          g_sys_config.net.auto_connect = cJSON_IsTrue(auto_conn);
      }

      // 解析云平台设置
      cJSON *cloud = cJSON_GetObjectItem(root, "cloud");
      if (cloud)
      {
        cJSON *plt = cJSON_GetObjectItem(cloud, "platform");         // 平台类型
        cJSON *dev_id = cJSON_GetObjectItem(cloud, "device_id");     // 设备ID
        cJSON *prod_id = cJSON_GetObjectItem(cloud, "product_id");   // 产品ID
        cJSON *secret = cJSON_GetObjectItem(cloud, "device_secret"); // 设备密钥
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
      loaded = true; // 成功从flash加载
    }
    else
    {
      log_e("Failed to parse config JSON.");
    }
  }
  // 加载失败
  if (!loaded)
  {
    log_w("Config file error, use defaults...");
    sys_config_set_defaults(); // 设为默认值
    sys_config_save();         // 尝试修复配置文件(使用默认值)
  }

  return 0;
#endif
}

/**
 * @brief 获取全局配置句柄(只读)
 *
 * @return const sys_config_t*
 */
const sys_config_t *sys_config_get(void) { return &g_sys_config; }

/**
 * @brief 更新网络配置
 *
 * @param net 新的网络配置
 */
void sys_config_set_net(const net_config_t *net)
{
  if (net)
  {
    memcpy(&g_sys_config.net, net, sizeof(net_config_t));
  }
}

/**
 * @brief 更新云平台配置
 *
 * @param cloud 新的云平台配置
 */
void sys_config_set_cloud(const cloud_config_t *cloud)
{
  if (cloud)
  {
    memcpy(&g_sys_config.cloud, cloud, sizeof(cloud_config_t));
  }
}

#if !USE_Simulator
/**
 * @brief 将g_sys_config当前配置以文件保存到sys使用的文件系统中
 *
 * @return int 0表示成功
 */
int sys_config_save(void)
{
  cJSON *root = cJSON_CreateObject(); // json根对象

  // Build Network JSON
  cJSON *net = cJSON_CreateObject();                           // 网络对象
  cJSON_AddStringToObject(net, "ssid", g_sys_config.net.ssid); // 为network添加字符串值
  cJSON_AddStringToObject(net, "password", g_sys_config.net.password);
  cJSON_AddBoolToObject(net, "auto_connect", g_sys_config.net.auto_connect); // 添加布尔值
  cJSON_AddItemToObject(root, "network", net);                               // network作为子对象添加到根对象中

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
  if (!out)
  {
    cJSON_Delete(root);
    return -1;
  }
  // 写入sys的文件系统(通过flash管理器)
  int res = flash_handler_write(SYS_CFG_FILE_PATH, 0, (const uint8_t *)out,
                                strlen(out));

  cJSON_free(out);
  cJSON_Delete(root);

  if (res == 0)
  {
    log_i("System config saved to flash.");
  }
  else
  {
    log_e("Failed to save system config to flash.");
  }

  return res;
}
#endif
