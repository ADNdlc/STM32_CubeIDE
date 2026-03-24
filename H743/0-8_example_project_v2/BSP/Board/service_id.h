#ifndef BOARD_SERVICE_ID_H_
#define BOARD_SERVICE_ID_H_

/**
 * @brief WiFi 服务逻辑 ID
 */
typedef enum {
    WIFI_ID_MAIN = 0,
    WIFI_MAX_DEVICES
} wifi_id_t;

/**
 * @brief MQTT 服务逻辑 ID
 */
typedef enum {
    MQTT_ID_MAIN = 0,
    MQTT_MAX_DEVICES
} mqtt_id_t;

/**
 * @brief SNTP 服务逻辑 ID
 */
typedef enum {
    SNTP_ID_MAIN = 0,
    SNTP_MAX_DEVICES
} sntp_id_t;

#endif /* BOARD_SERVICE_ID_H_ */
