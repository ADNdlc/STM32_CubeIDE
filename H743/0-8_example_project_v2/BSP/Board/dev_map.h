/*
 * dev_map.h
 *
 *  Created on: Feb 6, 2026
 *      Author: 12114
 */

#ifndef BOARD_DEV_MAP_H_
#define BOARD_DEV_MAP_H_

#include <stdint.h>

// 定义串口逻辑ID，供上层应用和Factory层使用
typedef enum {
  USART_ID_CONSOLE = 0, // 控制台
  USART_ID_ESP8266,     // 通信模块
  USART_MAX_DEVICES
} usart_device_id_t;

// 定义映射项结构（屏蔽平台差异）
typedef struct {
  void *
      resource; // 物理资源句柄（在STM32中是huart指针，在Linux中可能是设备路径字符串）
} usart_mapping_t;

// 声明映射表
extern const usart_mapping_t usart_mappings[USART_MAX_DEVICES];

#endif /* BOARD_DEV_MAP_H_ */
