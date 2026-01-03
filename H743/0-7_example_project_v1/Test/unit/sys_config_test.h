#ifndef TEST_UNIT_SYS_CONFIG_TEST_H_
#define TEST_UNIT_SYS_CONFIG_TEST_H_

/**
 * @brief 测试sys_config是否能成功从外部flash加载配置信息或在失败时保障配置信息不为空
 */
void sys_config_test_run(void);

/**
 * @brief 测试配置保存和加载功能
 */
void sys_config_save_load_test(void);

#endif /* TEST_UNIT_SYS_CONFIG_TEST_H_ */