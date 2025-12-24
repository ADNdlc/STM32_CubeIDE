/*
 * w25q256_driver.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  W25Q256 Flash 驱动
 *  32MB, 支持4字节地址模式
 */

#ifndef DEVICE_W25QXXX_W25Q256_DRIVER_H_
#define DEVICE_W25QXXX_W25Q256_DRIVER_H_

#include "flash_dependencies.h"
#include "flash_driver.h"
#include "spi_adapter.h"


#ifdef __cplusplus
extern "C" {
#endif

// 创建 W25Q256 驱动实例
// spi: SPI 适配器
// deps: 依赖注入
// 返回: 驱动实例，失败返回 NULL
flash_driver_t *w25q256_driver_create(spi_adapter_t *spi,
                                      flash_dependencies_t *deps);

// 销毁 W25Q256 驱动实例
void w25q256_driver_destroy(flash_driver_t *driver);

// 获取 W25Q256 驱动的操作接口（用于静态分配场景）
const flash_driver_ops_t *w25q256_get_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_W25QXXX_W25Q256_DRIVER_H_ */
