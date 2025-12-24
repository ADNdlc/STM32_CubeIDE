/*
 * flash_factory.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  Flash 驱动工厂模式接口
 *  统一的Flash驱动实例创建流程
 */

#ifndef INTERFACE_FLASH_FACTORY_H_
#define INTERFACE_FLASH_FACTORY_H_

#include "flash_dependencies.h"
#include "flash_driver.h"
#include "flash_handler.h"
#include "spi_adapter.h"


#ifdef __cplusplus
extern "C" {
#endif

// Flash 芯片类型
typedef enum {
  FLASH_CHIP_W25Q256 = 0,
  FLASH_CHIP_W25Q128,
  FLASH_CHIP_W25Q64,
  FLASH_CHIP_W25Q32,
  FLASH_CHIP_W25Q16,
  FLASH_CHIP_CUSTOM,
} flash_chip_type_t;

// 工厂配置
typedef struct {
  flash_chip_type_t chip_type; // 芯片类型
  spi_adapter_t *spi_adapter;  // SPI 适配器
  flash_dependencies_t *deps;  // 依赖（可选，NULL使用默认）
  const char *device_name;     // 设备名称
} flash_factory_config_t;

// ============== 工厂方法 ==============

// 创建 Flash 驱动实例
// 根据配置创建对应类型的驱动
// 返回：驱动实例，失败返回 NULL
flash_driver_t *flash_factory_create(const flash_factory_config_t *config);

// 销毁 Flash 驱动实例
void flash_factory_destroy(flash_driver_t *driver);

// ============== 预配置工厂方法 ==============

// 创建 W25Q256 驱动（使用QSPI）
flash_driver_t *flash_factory_create_w25q256_qspi(spi_adapter_t *adapter,
                                                  flash_dependencies_t *deps);

// 创建 W25Q256 驱动（使用标准SPI）
flash_driver_t *flash_factory_create_w25q256_spi(spi_adapter_t *adapter,
                                                 flash_dependencies_t *deps);

// 创建 W25Q128 驱动
flash_driver_t *flash_factory_create_w25q128(spi_adapter_t *adapter,
                                             flash_dependencies_t *deps);

// ============== 完整系统创建 ==============

// 创建完整的 Flash 系统（Handler + 驱动）
// 这是一站式创建方法，适用于快速启动
flash_handler_t *
flash_factory_create_system(const flash_factory_config_t *config);

// 销毁完整系统
void flash_factory_destroy_system(flash_handler_t *handler);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_FLASH_FACTORY_H_ */
