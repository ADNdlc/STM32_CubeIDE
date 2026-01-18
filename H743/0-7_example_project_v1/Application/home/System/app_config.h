/*
 * app_config.h
 *
 *  Created on: Jan 18, 2026
 *      Author: 12114
 */

#ifndef HOME_SYSTEM_APP_CONFIG_H_
#define HOME_SYSTEM_APP_CONFIG_H_

enum app_config_type_t { 
    APP_CONFIG_TYPE_INT,
    APP_CONFIG_TYPE_STRING,
    APP_CONFIG_TYPE_BOOL,
    APP_CONFIG_TYPE_DOUBLE,
};

typedef struct app_config_t { 
    const uint16_t key;
    uint8_t type;
    union{
        uint32_t Int;
        char *string;
        bool Bool;
        double Double;
    };
} app_config_t;

typedef struct app_settings_t { 
    app_config_t *configs;
    uint8_t count;
} app_settings_t;


#endif /* HOME_SYSTEM_APP_CONFIG_H_ */
