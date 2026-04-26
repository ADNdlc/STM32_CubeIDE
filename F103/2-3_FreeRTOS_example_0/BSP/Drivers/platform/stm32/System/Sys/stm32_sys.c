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
  // 在FreeRTOS环境下且调度器已启动时，对于大的us延时(>=1ms)，使用vTaskDelay
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED && us >= 1000) {
    vTaskDelay((us / 1000) / portTICK_PERIOD_MS);
    us %= 1000;
  }
#endif

  // 剩余的小于1ms的部分（或调度器未启动时）使用忙等待
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
}

void platform_delay_ms(uint32_t ms) {
#ifdef FREERTOS_ENABLED
  // 只有调度器启动后才能使用 vTaskDelay
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
  } else {
    HAL_Delay(ms);
  }
#else
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
  // 由于本工程 HAL Timebase 使用了 TIM4，且配置为 1MHz (1us/tick)
  // 我们可以通过 HAL_GetTick() 获取毫秒，再结合 TIM4->CNT 获取微秒偏移
  uint32_t ms = platform_get_systick_ms();
  uint32_t us_offset = TIM4->CNT;

  // 再次检查以处理在读取过程中发生的毫秒翻转
  if (platform_get_systick_ms() != ms) {
    ms = platform_get_systick_ms();
    us_offset = TIM4->CNT;
  }

  return ms * 1000 + us_offset;
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