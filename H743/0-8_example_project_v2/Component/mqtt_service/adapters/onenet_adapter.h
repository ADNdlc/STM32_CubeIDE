#ifndef ONENET_ADAPTER_H
#define ONENET_ADAPTER_H

#include "../mqtt_adapter.h"

/**
 * @brief OneNet 平台配置参数
 */
typedef struct {
    char product_id[64];
    char device_id[64];
    char device_secret[128];
} onenet_config_t;

/**
 * @brief 初始化 OneNet 适配器参数
 * @param config 参数配置
 */
void onenet_adapter_init(const onenet_config_t *config);

extern const mqtt_adapter_t g_onenet_adapter;	// onenet适配器实例

#endif // ONENET_ADAPTER_H
