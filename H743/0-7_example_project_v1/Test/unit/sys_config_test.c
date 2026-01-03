#include "sys_init.h"
#include "elog.h"
#include "factory/flash_factory.h"
#include "flash_handler.h"
#include "home/System/net_mgr.h"
#include "home/System/sys_config.h"
#include "home/System/sys_state.h"
#include "lv_port_fs.h"
#include "project_cfg.h"
#include "strategy/lfs_strategy.h"
#include <stddef.h>

#define LOG_TAG "SYS_CFG_TEST"
#define TEST_EXT_QSPI FLASH_EXT_QSPI
#define SYS_STORE_TEST_POINT "/sys"
/**
 * @brief 测试sys_config是否能成功从外部flash加载配置信息或在失败时保障配置信息不为空
 */
void sys_config_test_run(void) {
    log_i("Starting sys_config test...");
    
    // 初始化系统服务直到"系统组件初始化"之前
    log_i("Initializing system services up to config loading...");
    
    // 执行flash_handler初始化和文件系统初始化，但不执行sys_config_init
    flash_handler_init();
    
    // 获取存储设备
    block_device_t *sys_dev = flash_factory_get(TEST_EXT_QSPI);
    if (!sys_dev) {
        log_e("Failed to get system storage device.");
        return;
    }
    
    // 创建LFS策略和设备配置
    lfs_strategy_config_t lfs_cfg = {
        .read_size = 16,
        .prog_size = 16,
        .cache_size = 256,
        .lookahead_size = 32,
        .block_cycles = 200,
    };
    
    flash_strategy_t *lfs_strat = lfs_strategy_create(&lfs_cfg);
    if (!lfs_strat) {
        log_e("LFS Strategy Create Failed");
        return;
    }
    
    // 注册系统资源挂载点
    if (flash_handler_register(SYS_STORE_TEST_POINT, sys_dev, lfs_strat) != 0) {
        log_e("LFS Handler Register Failed");
        return;
    }
    
    log_i("System storage mounted at %s", SYS_STORE_TEST_POINT);
    
    // 测试1: 验证sys_config_init是否能正确加载配置或使用默认值
    log_i("Test 1: Initializing system config...");
    int result = sys_config_init();
    
    if (result == 0) {
        log_i("sys_config_init() returned success (0)");
    } else {
        log_w("sys_config_init() returned error (%d)", result);
    }
    
    // 测试2: 验证配置信息不为空
    log_i("Test 2: Verifying config is not empty...");
    const sys_config_t *config = sys_config_get();
    
    // 检查网络配置
    if (strlen(config->net.ssid) > 0) {
        log_i("Network SSID: %s", config->net.ssid);
    } else {
        log_w("Network SSID is empty, using default");
    }
    
    if (strlen(config->net.password) > 0) {
        log_i("Network password length: %d", (int)strlen(config->net.password));
    } else {
        log_w("Network password is empty, using default");
    }
    
    log_i("Auto connect: %s", config->net.auto_connect ? "true" : "false");
    
    // 检查云配置
    log_i("Cloud platform: %d", (int)config->cloud.platform);
    
    if (strlen(config->cloud.product_id) > 0) {
        log_i("Cloud product ID: %s", config->cloud.product_id);
    } else {
        log_w("Cloud product ID is empty, using default");
    }
    
    if (strlen(config->cloud.device_id) > 0) {
        log_i("Cloud device ID: %s", config->cloud.device_id);
    } else {
        log_w("Cloud device ID is empty, using default");
    }
    
    if (strlen(config->cloud.device_secret) > 0) {
        log_i("Cloud device secret length: %d", (int)strlen(config->cloud.device_secret));
    } else {
        log_w("Cloud device secret is empty, using default");
    }
    
    // 测试3: 尝试保存配置
    log_i("Test 3: Testing config save...");
    int save_result = sys_config_save();
    
    if (save_result == 0) {
        log_i("sys_config_save() returned success (0)");
    } else {
        log_w("sys_config_save() returned error (%d)", save_result);
    }
    
    // 测试4: 读取保存的配置，再次验证
    log_i("Test 4: Re-initializing to verify saved config...");
    int reinit_result = sys_config_init();
    
    if (reinit_result == 0) {
        log_i("Re-initialization successful");
    } else {
        log_w("Re-initialization failed (%d)", reinit_result);
    }
    
    const sys_config_t *new_config = sys_config_get();
    
    // 比较两次获取的配置是否一致（基本检查）
    if (strcmp(config->net.ssid, new_config->net.ssid) == 0) {
        log_i("Configuration persistence verified - SSID matches after re-initialization");
    } else {
        log_w("Configuration may not persist correctly - SSID differs after re-initialization");
    }
    
    log_i("sys_config test completed.");
}

/**
 * @brief 测试配置保存和加载
 */
void sys_config_save_load_test(void) {
    log_i("Starting save/load test...");
    
    // 初始化配置
    sys_config_init();
    
    // 修改一些配置值
    net_config_t new_net_config = {
        .ssid = "test_network",
        .password = "test_password123",
        .auto_connect = true
    };
    
    cloud_config_t new_cloud_config = {
        .platform = CLOUD_PLATFORM_ALIYUN,
        .device_id = "test_device_id",
        .product_id = "test_product_id",
        .device_secret = "test_secret_key"
    };
    
    sys_config_set_net(&new_net_config);
    sys_config_set_cloud(&new_cloud_config);
    
    log_i("Updated network config - SSID: %s, Password: %s", 
          new_net_config.ssid, new_net_config.password);
    log_i("Updated cloud config - Platform: %d, Device ID: %s", 
          (int)new_cloud_config.platform, new_cloud_config.device_id);
    
    // 保存配置
    int save_result = sys_config_save();
    if (save_result == 0) {
        log_i("Configuration saved successfully");
    } else {
        log_e("Failed to save configuration");
        return;
    }
    
    // 重新加载配置
    int init_result = sys_config_init();
    if (init_result == 0) {
        log_i("Configuration reloaded successfully");
    } else {
        log_e("Failed to reload configuration");
        return;
    }
    
    // 验证配置是否正确加载
    const sys_config_t *config = sys_config_get();
    
    if (strcmp(config->net.ssid, "test_network") == 0 &&
        strcmp(config->net.password, "test_password123") == 0 &&
        config->net.auto_connect == true &&
        config->cloud.platform == CLOUD_PLATFORM_ALIYUN) {
        log_i("Save/load test passed - configuration matches expected values");
    } else {
        log_w("Save/load test failed - configuration does not match expected values");
    }
    
    log_i("Save/load test completed.");
}
