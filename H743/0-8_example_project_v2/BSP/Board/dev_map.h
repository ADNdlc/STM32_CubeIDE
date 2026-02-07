/*
 * dev_map.h
 *
 *  Created on: Feb 6, 2026
 *      Author: 12114
 */

#ifndef BOARD_DEV_MAP_H_
#define BOARD_DEV_MAP_H_

#include "dev_map_config.h"

// 定义串口逻辑ID，供上层应用和Factory层使用
typedef enum {
  USART_ID_DEBUG = 0,   // 日志和调试
  USART_ID_ESP8266,     // 网络通信模块
  // ...
  USART_MAX_DEVICES
} usart_device_id_t;

// 定义映射项结构（屏蔽平台差异）
typedef struct {
  void * resource; // 物理资源句柄
} usart_mapping_t;

// 声明映射表
extern const usart_mapping_t usart_mappings[USART_MAX_DEVICES];

#endif /* BOARD_DEV_MAP_H_ */
