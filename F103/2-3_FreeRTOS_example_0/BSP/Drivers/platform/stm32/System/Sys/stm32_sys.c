#include "stm32_sys.h"
#include "main.h"
#include "Project_cfg.h"  // 包含项目配置
#include <stddef.h>
#include <stdint.h>

#ifdef FREERTOS_ENABLED
#include "FreeRTOS.h"
#include "task.h"
#endif

static uint32_t g_fac_us = 0; /* us延时倍乘数 */

void platform_sys_init(void) {
  // 一定要先更新SystemCoreClock
  SystemCoreClockUpdate();
  g_fac_us = SystemCoreClock / 1000000; // 获取系统频率
}

void platform_delay_us(uint32_t us) {
#ifdef FREERTOS_ENABLED
  // 在FreeRTOS环境下，对于小的us延时(<1ms)，使用忙等待
  // 对于大的us延时(>=1ms)，转换为ms延时使用vTaskDelay
  if (us >= 1000) {
    vTaskDelay((us / 1000) / portTICK_PERIOD_MS);
    us %= 1000;
  }
  // 剩余的小于1ms的部分使用忙等待
  if (us > 0) {
    uint32_t ticks = us * g_fac_us;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD; /* LOAD的值 */
    told = SysTick->VAL;             /* 刚进入时的计数器值 */
    while (1) {
      tnow = SysTick->VAL;
      if (tnow != told) {
        if (tnow < told) {
          tcnt += told - tnow;
        } else {
          tcnt += reload - tnow + told;
        }
        told = tnow;
        if (tcnt >= ticks) {
          break;
        }
      }
    }
  }
#else
  // 在裸机环境中，使用忙等待
  uint32_t ticks = us * g_fac_us;
  uint32_t told, tnow, tcnt = 0;
  uint32_t reload = SysTick->LOAD; /* LOAD的值 */
  told = SysTick->VAL;             /* 刚进入时的计数器值 */
  while (1) {
    tnow = SysTick->VAL;
    if (tnow != told) {
      if (tnow < told) {
        tcnt += told - tnow;
      } else {
        tcnt += reload - tnow + told;
      }
      told = tnow;
      if (tcnt >= ticks) {
        break;
      }
    }
  }
#endif
}

void platform_delay_ms(uint32_t ms) {
#ifdef FREERTOS_ENABLED
  vTaskDelay(ms / portTICK_PERIOD_MS);
#else
  // 在裸机环境中，使用HAL延时
  HAL_Delay(ms);
#endif
}

uint32_t platform_get_systick_ms(void) { 
#ifdef FREERTOS_ENABLED
  // 检查调度器是否已经启动
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
  } else {
    // 调度器未启动，使用HAL_GetTick
    return HAL_GetTick();
  }
#else
  // 在裸机环境中，使用HAL_GetTick
  return HAL_GetTick();
#endif
}

uint32_t platform_get_systick_us(void) {
#ifdef FREERTOS_ENABLED
  // 检查调度器是否已经启动
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    // 使用FreeRTOS tick计算微秒
    uint32_t current_ticks = xTaskGetTickCount();
    return (current_ticks * 1000000) / configTICK_RATE_HZ;
  } else {
    // 调度器未启动，使用SysTick计算微秒
    uint32_t load = SysTick->LOAD;
    if (load == 0)
      return 0; // Prevent div by zero if not init

    uint32_t ms_start = HAL_GetTick();
    uint32_t val = SysTick->VAL;
    uint32_t ms_now = HAL_GetTick();

    // Handle potential wrap around during reading
    if (ms_now != ms_start) {
      val = SysTick->VAL;
      ms_now = HAL_GetTick();
    }

    uint32_t ticks_per_us = SystemCoreClock / 1000000;
    if (ticks_per_us == 0)
      ticks_per_us = 1; // Safety

    uint32_t us_offset = (load - val) / ticks_per_us;
    return ms_now * 1000 + us_offset;
  }
#else
  // 在裸机环境中，使用SysTick计算微秒
  uint32_t load = SysTick->LOAD;
  if (load == 0)
    return 0; // Prevent div by zero if not init

  uint32_t ms_start = HAL_GetTick();
  uint32_t val = SysTick->VAL;
  uint32_t ms_now = HAL_GetTick();

  // Handle potential wrap around during reading
  if (ms_now != ms_start) {
    val = SysTick->VAL;
    ms_now = HAL_GetTick();
  }

  uint32_t ticks_per_us = SystemCoreClock / 1000000;
  if (ticks_per_us == 0)
    ticks_per_us = 1; // Safety

  uint32_t us_offset = (load - val) / ticks_per_us;
  return ms_now * 1000 + us_offset;
#endif
}

uint32_t platform_get_CoreClock(void) {
  return SystemCoreClock/1000000;
}

void platform_display_irq(){
  __disable_irq();
}

void platform_enable_irq(){
  __enable_irq();
}