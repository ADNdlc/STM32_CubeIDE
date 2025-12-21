/*
 * rtc_factory.c
 *
 *  Created on: Dec 21, 2025
 *      Author: 12114
 */

#include "rtc_factory.h"
#include "device_mapping.h"
#include "factory_config.h"

#if (RTC_DRIVER_PLATFORM == PLATFORM_STM32)
#include "stm32_rtc_driver.h"
#endif

static rtc_driver_t *rtc_drivers[RTC_MAX_DEVICES] = {NULL};

rtc_driver_t *rtc_driver_get(rtc_device_id_t id)
{
  if (id >= RTC_MAX_DEVICES)
  {
    return NULL;
  }
  
  if (rtc_drivers[id] == NULL)
  {
    const rtc_mapping_t *mapping = &rtc_mappings[id]; // 获取设备映射信息
    // 根据平台配置创建相应的RTC驱动实例
    #if (RTC_DRIVER_PLATFORM == PLATFORM_STM32)
    rtc_drivers[id] = stm32_rtc_driver_create(mapping->hrtc);
    #else
    #error "未定义RTC_DRIVER_PLATFORM或平台不支持"
    #endif
  }
  
  return rtc_drivers[id];
}
