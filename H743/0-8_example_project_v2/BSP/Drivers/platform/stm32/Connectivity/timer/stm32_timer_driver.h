#ifndef BSP_DEVICE_DRIVER_PLATFORM_STM32_TIMER_DRIVER_H_
#define BSP_DEVICE_DRIVER_PLATFORM_STM32_TIMER_DRIVER_H_

#include "stm32H7xx_hal.h"
#include "timer_driver.h"

#define TIMER_MEMSOURCE SYS_MEM_INTERNAL

// STM32 定时器驱动结构体
typedef struct {
  timer_driver_t base;       // 基类
  TIM_HandleTypeDef *htim;   // HAL 句柄
  timer_callback_t callback; // 用户回调
  void *callback_context;    // 回调上下文
  uint32_t period_ms;        // 当前周期
} stm32_timer_driver_t;

/**
 * @brief 创建 STM32 定时器驱动实例
 * @param htim HAL 定时器句柄
 * @return timer_driver_t* 驱动实例指针
 */
timer_driver_t *stm32_timer_driver_create(TIM_HandleTypeDef *htim);

/**
 * @brief 销毁 STM32 定时器驱动实例，释放资源
 * @param driver 驱动实例指针
 */
void stm32_timer_driver_destroy(timer_driver_t *driver);

#endif /* BSP_DEVICE_DRIVER_PLATFORM_STM32_TIMER_DRIVER_H_ */
