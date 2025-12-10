#include "stm32_sys.h"
#include "stm32h7xx_hal.h"
#include "stm32_h7_malloc.h"
#include <stddef.h>
#include <stdint.h>

static uint32_t g_fac_us = 0; /* us延时倍乘数 */

static void stm32_delay_us(uint32_t us)
{
  uint32_t ticks = us * g_fac_us;
  uint32_t told, tnow, tcnt = 0;
  uint32_t reload = SysTick->LOAD; /* LOAD的值 */
  told = SysTick->VAL;             /* 刚进入时的计数器值 */
  while (1)
  {
    tnow = SysTick->VAL;
    if (tnow != told)
    {
      if (tnow < told)
      {
        tcnt += told - tnow;
      }
      else
      {
        tcnt += reload - tnow + told;
      }
      told = tnow;
      if (tcnt >= ticks)
      {
        break;
      }
    }
  }
}

static void stm32_delay_ms(uint32_t ms)
{
  while (ms)
  {
    stm32_delay_us(1000);
    ms--;
  }
}

static uint32_t stm32_get_systick_ms(void) { return HAL_GetTick(); }

static uint32_t stm32_get_systick_us(void)
{
  uint32_t load = SysTick->LOAD;
  if (load == 0)
    return 0; // Prevent div by zero if not init

  uint32_t ms_start = HAL_GetTick();
  uint32_t val = SysTick->VAL;
  uint32_t ms_now = HAL_GetTick();

  // Handle potential wrap around during reading
  if (ms_now != ms_start)
  {
    val = SysTick->VAL;
    ms_now = HAL_GetTick();
  }

  uint32_t ticks_per_us = SystemCoreClock / 1000000;
  if (ticks_per_us == 0)
    ticks_per_us = 1; // Safety

  uint32_t us_offset = (load - val) / ticks_per_us;

  return ms_now * 1000 + us_offset;
}

static void *stm32_sys_malloc(SysMemTag tag, uint32_t size)
{
  uint8_t memx;
  switch (tag)
  {
  case SYS_MEM_INTERNAL:
    memx = SRAMIN;
    break;
  case SYS_MEM_EXTERNAL:
    memx = SRAMEX;
    break;
  case SYS_MEM_CUSTOM:
    memx = SRAMDTCM;
    break;
  default:
    return NULL;
  }
  return mymalloc(memx, size);
}

static void stm32_sys_free(SysMemTag tag, void *ptr)
{
  uint8_t memx;
  switch (tag)
  {
  case SYS_MEM_INTERNAL:
    memx = SRAMIN;
    break;
  case SYS_MEM_EXTERNAL:
    memx = SRAMEX;
    break;
  case SYS_MEM_CUSTOM:
    memx = SRAMDTCM;
    break;
  default:
    return;
  }
  myfree(memx, ptr);
}

static void *stm32_sys_realloc(SysMemTag tag, void *ptr, uint32_t size)
{
  uint8_t memx;
  switch (tag)
  {
  case SYS_MEM_INTERNAL:
    memx = SRAMIN;
    break;
  case SYS_MEM_EXTERNAL:
    memx = SRAMEX;
    break;
  case SYS_MEM_CUSTOM:
    memx = SRAMDTCM;
    break;
  default:
    return NULL;
  }
  return myrealloc(memx, ptr, size);
}

static int stm32_sys_init(void)
{
  // 一定要先更新SystemCoreClock
  SystemCoreClockUpdate();
  g_fac_us = SystemCoreClock / 1000000; // 获取系统频率

  // Initialize memory pools
  my_mem_init(SRAMIN);
  my_mem_init(SRAMEX);
  my_mem_init(SRAMDTCM);

  return 0;
}

const SysCoreOps stm32_sys_core_ops = {.delay_ms = stm32_delay_ms,
                                       .delay_us = stm32_delay_us,
                                       .get_systick_ms = stm32_get_systick_ms,
                                       .get_systick_us = stm32_get_systick_us,
                                       .malloc = stm32_sys_malloc,
                                       .free = stm32_sys_free,
                                       .realloc = stm32_sys_realloc,
                                       .init = stm32_sys_init};
