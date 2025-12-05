/*
 * tim4_debug.h
 */

#ifndef TEST_UNIT_TIM4_DEBUG_H_
#define TEST_UNIT_TIM4_DEBUG_H_

// 诊断TIM4寄存器状态
void tim4_diagnose(void);

// 手动强制启动TIM4所有通道（用于测试）
void tim4_force_start_all_channels(void);

#endif /* TEST_UNIT_TIM4_DEBUG_H_ */
