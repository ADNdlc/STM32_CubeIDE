/**
 * @file    elog_init.c
 * @brief   EasyLogger initialization and configuration
 * 
 * This file contains the initialization function for EasyLogger that sets up
 * logging levels, formats, and starts the logger. It should be called at
 * the beginning of the main function to enable logging throughout the application.
 */

#include "elog.h"
#include "main.h"

/* Private function prototypes */
static void elog_content_config(void);

/**
 * @brief  Initialize and configure EasyLogger
 * 
 * This function initializes EasyLogger, configures the log format for each level,
 * and starts the logger. It should be called once at the beginning of the main function.
 * 
 * @retval ELOG_NO_ERR if successful, error code otherwise
 */
ElogErrCode elog_init_and_config(void) {
    ElogErrCode result = ELOG_NO_ERR;
    
    /* Initialize EasyLogger */
    result = elog_init();
    if (result != ELOG_NO_ERR) {
        return result;
    }
    
    /* Configure logger format settings */
    elog_content_config();
    
    /* Start EasyLogger */
    elog_start();
    
    /* Log initialization success */
    log_i("EasyLogger", "Logger initialized successfully");
    
    return result;
}

/**
 * @brief  Configure EasyLogger output formats
 * 
 * 配置各个级别的输出内容
 * 
 */
static void elog_content_config(void) {
    /* 断言: 所有信息
     * 包括: (等级,标签,时间戳,进程/线程信息,文件/行信息,函数名)
     */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    
    /* ERROR level: Essential information for runtime errors
     * Shows level, tag, and timestamp for quick identification */
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    
    /* WARN level: Important warnings that may require attention
     * Same format as ERROR to maintain consistency */
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    
    /* INFO level: General information about system operation
     * Same format as ERROR and WARN for clean, readable logs */
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    
    /* DEBUG level: Detailed information for debugging
     * Excludes function name to reduce verbosity while keeping other useful info */
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    
    /* VERBOSE level: Most detailed logging level
     * Same as DEBUG level to avoid excessive verbosity */
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    
    elog_set_filter_lvl(ELOG_LVL_VERBOSE);
}
