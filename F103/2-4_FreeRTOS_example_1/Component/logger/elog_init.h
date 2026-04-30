/**
 * @file    elog_init.h
 * @brief   EasyLogger initialization and configuration header
 * 
 * This header file contains declarations for EasyLogger initialization
 * and configuration functions.
 */

#ifndef __ELOG_INIT_H__
#define __ELOG_INIT_H__

#include "elog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize and configure EasyLogger
 * 
 * This function initializes EasyLogger, configures the log format for each level,
 * and starts the logger. It should be called once at the beginning of the main function.
 * 
 * @retval ELOG_NO_ERR if successful, error code otherwise
 */
ElogErrCode elog_init_and_config(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELOG_INIT_H__ */
