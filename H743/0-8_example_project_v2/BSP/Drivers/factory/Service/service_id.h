#ifndef SERVICE_ID_H
#define SERVICE_ID_H

/**
 * @brief WiFi 服务逻辑 ID
 */
typedef enum {
    WIFI_ID_MAIN = 0,
    WIFI_ID_SEC,
    WIFI_ID_MAX
} wifi_id_t;

/**
 * @brief MQTT 服务逻辑 ID
 */
typedef enum {
    MQTT_ID_MAIN = 0,
    MQTT_ID_SEC,
    MQTT_ID_MAX
} mqtt_id_t;

/**
 * @brief SNTP 服务逻辑 ID
 */
typedef enum {
    SNTP_ID_MAIN = 0,
    SNTP_ID_MAX
} sntp_id_t;

#endif // SERVICE_ID_H
