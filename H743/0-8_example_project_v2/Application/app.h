/*
 * app.h
 *
 *  Created on: Dec 15, 2025
 *      Author: 12114
 */

#ifndef APP_H_
#define APP_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief App
 *
 *这里作为所有业务层程序入口
 *
 */

int app_init(void);
void app_run(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* APP_H_ */
