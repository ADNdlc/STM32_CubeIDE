/*
 * stm32_timer_driver.c
 */

#include "stm32_timer_driver.h"
#include "MemPool.h"
#include <stdlib.h>
#include <stdbool.h>

#define MAX_TIMER_INSTANCES 8
static stm32_timer_driver_t *timer_instances[MAX_TIMER_INSTANCES] = {0};

// ---------------- 内部辅助函数 ----------------

static void register_timer_instance(stm32_timer_driver_t *driver) {
  for (int i = 0; i < MAX_TIMER_INSTANCES; i++) {
    if (timer_instances[i] == NULL) {
      timer_instances[i] = driver;
      break;
    }
  }
}

static void unregister_timer_instance(stm32_timer_driver_t *driver) {
  for (int i = 0; i < MAX_TIMER_INSTANCES; i++) {
    if (timer_instances[i] == driver) {
      timer_instances[i] = NULL;
      break;
    }
  }
}

static stm32_timer_driver_t *find_timer_driver(TIM_TypeDef *instance) {
  for (int i = 0; i < MAX_TIMER_INSTANCES; i++) {
    if (timer_instances[i] && timer_instances[i]->htim->Instance == instance) {
      return timer_instances[i];
    }
  }
  return NULL;
}

/**
 * @brief 获取定时器输入时钟频率 (兼容 F1/F4/G4 等常规 APB 架构)
 */
static uint32_t get_timer_clock_freq(TIM_HandleTypeDef *htim) {
  uint32_t pclk = 0;
  uint32_t ppre = 0;
  uint32_t instance_addr = (uint32_t)htim->Instance;

  // 1. 判断是 APB1 还是 APB2
  // 大多数 STM32 中，APB2 外设地址从 0x40010000 开始
  if (instance_addr >= APB2PERIPH_BASE) {
    pclk = HAL_RCC_GetPCLK2Freq();
    // 获取 APB2 分频系数
#if defined(RCC_CFGR_PPRE2)
    ppre = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;
#elif defined(RCC_D2CFGR_D2PPRE2) // H7
    ppre = (RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos;
#endif
  } else {
    pclk = HAL_RCC_GetPCLK1Freq();
    // 获取 APB1 分频系数
#if defined(RCC_CFGR_PPRE1)
    ppre = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
#elif defined(RCC_D2CFGR_D2PPRE1) // H7
    ppre = (RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos;
#endif
  }

  // 2. STM32 定时器倍频规则：如果 APB 预分频系数 != 1 (即 ppre >= 4)，则频率 x2
  // ppre 格式通常是: 0xx: /1, 100: /2, 101: /4 ...
  // 所以只要最高位是 1 (值 >= 4)，就代表进行了分频
  if (ppre >= 4) {
    return pclk * 2;
  }
  return pclk;
}

// ---------------- 驱动接口实现 ----------------

static int stm32_timer_start(timer_driver_t *self) {
  stm32_timer_driver_t *driver = (stm32_timer_driver_t *)self;
  // 清除更新标志位，防止启动时立即进入中断（如果是停止状态下修改过配置）
  __HAL_TIM_CLEAR_FLAG(driver->htim, TIM_FLAG_UPDATE);
  
  if (HAL_TIM_Base_Start_IT(driver->htim) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int stm32_timer_stop(timer_driver_t *self) {
  stm32_timer_driver_t *driver = (stm32_timer_driver_t *)self;
  if (HAL_TIM_Base_Stop_IT(driver->htim) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int stm32_timer_set_period(timer_driver_t *self, uint32_t period_ms) {
  stm32_timer_driver_t *driver = (stm32_timer_driver_t *)self;
  if (period_ms == 0) return -1;

  uint32_t timer_clk = get_timer_clock_freq(driver->htim);
  // 计算所需的总 Tick 数
  // 使用 uint64_t 防止 (timer_clk * period_ms) 溢出
  uint64_t total_ticks = (uint64_t)timer_clk * period_ms / 1000;

  // 动态计算 PSC 和 ARR
  uint32_t psc = 0;
  uint32_t arr = total_ticks - 1;

  // 检测是否为 32 位定时器 (F4/H7 的 TIM2 和 TIM5)
  bool is_32bit = false;
#if defined(TIM2)
  if (driver->htim->Instance == TIM2) is_32bit = true;
#endif
#if defined(TIM5)
  if (driver->htim->Instance == TIM5) is_32bit = true;
#endif

  uint32_t max_arr_val = is_32bit ? 0xFFFFFFFF : 0xFFFF;

  // 如果 ARR 超过了当前定时器的最大值，增加预分频 PSC
  while (arr > max_arr_val) {
    psc++;
    // 重新计算 ARR
    arr = (total_ticks / (psc + 1)) - 1;
  }

  // 检查 PSC 是否溢出 (PSC 始终是 16 位的)
  if (psc > 0xFFFF) {
    return -1; // 周期太长，硬件无法支持
  }

  // 更新寄存器
  __HAL_TIM_SET_PRESCALER(driver->htim, psc);
  __HAL_TIM_SET_AUTORELOAD(driver->htim, arr);

  // 关键修复：生成更新事件以立即加载 PSC 和 ARR，但必须紧接着清除中断标志
  driver->htim->Instance->EGR = TIM_EGR_UG;
  __HAL_TIM_CLEAR_FLAG(driver->htim, TIM_FLAG_UPDATE);

  driver->period_ms = period_ms;
  return 0;
}

static int stm32_timer_set_callback(timer_driver_t *self,
                                    timer_callback_t callback, void *context) {
  stm32_timer_driver_t *driver = (stm32_timer_driver_t *)self;
  driver->callback = callback;
  driver->callback_context = context;
  return 0;
}

// 定义 V-Table
static const timer_driver_ops_t stm32_timer_ops = {
    .start = stm32_timer_start,
    .stop = stm32_timer_stop,
    .set_period = stm32_timer_set_period,
    .set_callback = stm32_timer_set_callback,
};

// ---------------- 构造与析构 ----------------

timer_driver_t *stm32_timer_driver_create(TIM_HandleTypeDef *htim) {
  if (!htim) return NULL;

  // 检查该句柄是否已经被注册过，防止重复创建
  if (find_timer_driver(htim->Instance) != NULL) {
      return (timer_driver_t *)find_timer_driver(htim->Instance);
  }

#ifdef USE_MEMPOOL
  stm32_timer_driver_t *driver =
      (stm32_timer_driver_t *)sys_malloc(TIMER_MEMSOURCE, sizeof(stm32_timer_driver_t));
#else
stm32_timer_driver_t *driver =
    (stm32_timer_driver_t *)malloc(sizeof(stm32_timer_driver_t));
#endif


  if (driver) {
    driver->base.ops = &stm32_timer_ops;
    driver->htim = htim;
    driver->callback = NULL;
    driver->callback_context = NULL;
    driver->period_ms = 0;
    
    register_timer_instance(driver);
  }
  return (timer_driver_t *)driver;
}

void stm32_timer_driver_destroy(timer_driver_t *driver_base) {
  if (!driver_base) return;

  stm32_timer_driver_t *driver = (stm32_timer_driver_t *)driver_base;

  // 1. 停止定时器
  HAL_TIM_Base_Stop_IT(driver->htim);

  // 2. 从全局实例表中移除
  unregister_timer_instance(driver);

  // 3. 释放内存
  free(driver);
}

// ---------------- HAL 中断回调 ----------------

/**
 * @brief HAL 库统一的定时器中断回调
 * 注意：所有开启中断的 TIM 都会进入这里
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // 1. 保护系统心跳 (假设 System Tick 使用的是某个 Timer)
  // 如果你的 SysTick 使用的是 Systick 硬件定时器，这步可忽略
  // 但 HAL 库有时会用 TIM6/7 做时基，必须保护
  if (htim->Instance == uwTickPrio) { // 注意：这里通常需要检查具体的 TIM Instance
      return; 
  }

  // 2. 查找并执行我们的驱动回调
  stm32_timer_driver_t *driver = find_timer_driver(htim->Instance);
  if (driver && driver->callback) {
    driver->callback(driver->callback_context);
  }
}
