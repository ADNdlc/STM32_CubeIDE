/*
 * w25qxxx_common.h
 *
 *  Created on: Dec 24, 2024
 *      Author: Antigravity
 *
 *  W25Qxxx 系列 Flash 通用驱动
 *  支持 W25Q256, W25Q128, W25Q64 等
 */

#ifndef DEVICE_W25QXXX_W25QXXX_COMMON_H_
#define DEVICE_W25QXXX_W25QXXX_COMMON_H_

#include "flash_dependencies.h"
#include "flash_driver.h"
#include "spi_adapter.h"
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// ============== W25Qxxx 指令定义 ==============

// 读取指令
#define W25X_CMD_READ_DATA 0x03      // 读数据（3字节地址）
#define W25X_CMD_FAST_READ 0x0B      // 快速读取
#define W25X_CMD_FAST_READ_DUAL 0x3B // 双线快速读
#define W25X_CMD_FAST_READ_QUAD 0x6B // 四线快速读
#define W25X_CMD_READ_DATA_4B 0x13   // 读数据（4字节地址）
#define W25X_CMD_FAST_READ_4B 0x0C   // 快速读取（4字节地址）

// 写入指令
#define W25X_CMD_WRITE_ENABLE 0x06      // 写使能
#define W25X_CMD_WRITE_DISABLE 0x04     // 写禁止
#define W25X_CMD_PAGE_PROGRAM 0x02      // 页编程（3字节地址）
#define W25X_CMD_PAGE_PROGRAM_4B 0x12   // 页编程（4字节地址）
#define W25X_CMD_QUAD_PAGE_PROGRAM 0x32 // 四线页编程

// 擦除指令
#define W25X_CMD_SECTOR_ERASE 0x20       // 扇区擦除（4KB）
#define W25X_CMD_BLOCK_ERASE_32K 0x52    // 块擦除（32KB）
#define W25X_CMD_BLOCK_ERASE_64K 0xD8    // 块擦除（64KB）
#define W25X_CMD_CHIP_ERASE 0xC7         // 全片擦除
#define W25X_CMD_SECTOR_ERASE_4B 0x21    // 扇区擦除（4字节地址）
#define W25X_CMD_BLOCK_ERASE_64K_4B 0xDC // 块擦除（4字节地址）

// 状态寄存器
#define W25X_CMD_READ_SR1 0x05  // 读状态寄存器1
#define W25X_CMD_READ_SR2 0x35  // 读状态寄存器2
#define W25X_CMD_READ_SR3 0x15  // 读状态寄存器3
#define W25X_CMD_WRITE_SR1 0x01 // 写状态寄存器1
#define W25X_CMD_WRITE_SR2 0x31 // 写状态寄存器2
#define W25X_CMD_WRITE_SR3 0x11 // 写状态寄存器3

// ID 读取
#define W25X_CMD_READ_JEDEC_ID 0x9F  // 读取 JEDEC ID
#define W25X_CMD_READ_DEVICE_ID 0x90 // 读取设备 ID
#define W25X_CMD_READ_UNIQUE_ID 0x4B // 读取唯一 ID

// 电源管理
#define W25X_CMD_POWER_DOWN 0xB9         // 掉电
#define W25X_CMD_RELEASE_POWER_DOWN 0xAB // 释放掉电

// 地址模式
#define W25X_CMD_ENTER_4B_MODE 0xB7 // 进入4字节地址模式
#define W25X_CMD_EXIT_4B_MODE 0xE9  // 退出4字节地址模式

// 复位
#define W25X_CMD_ENABLE_RESET 0x66 // 使能复位
#define W25X_CMD_RESET 0x99        // 复位

// 状态寄存器1位定义
#define W25X_SR1_BUSY 0x01 // 忙标志
#define W25X_SR1_WEL 0x02  // 写使能锁存
#define W25X_SR1_BP0 0x04  // 块保护位0
#define W25X_SR1_BP1 0x08  // 块保护位1
#define W25X_SR1_BP2 0x10  // 块保护位2
#define W25X_SR1_TB 0x20   // 顶部/底部保护
#define W25X_SR1_SEC 0x40  // 扇区/块保护
#define W25X_SR1_SRP0 0x80 // 状态寄存器保护0

// ============== 制造商 ID ==============
#define W25X_MANUFACTURER_WINBOND 0xEF

// ============== 芯片型号定义 ==============

typedef enum {
  W25Q_TYPE_UNKNOWN = 0,
  W25Q_TYPE_W25Q16,  // 2MB
  W25Q_TYPE_W25Q32,  // 4MB
  W25Q_TYPE_W25Q64,  // 8MB
  W25Q_TYPE_W25Q128, // 16MB
  W25Q_TYPE_W25Q256, // 32MB
  W25Q_TYPE_W25Q512, // 64MB
} w25q_chip_type_t;

// ============== W25Qxxx 私有数据 ==============

typedef struct {
  spi_adapter_t *spi;         // SPI 适配器
  flash_dependencies_t *deps; // 依赖
  w25q_chip_type_t chip_type; // 芯片型号
  uint8_t addr_mode;          // 地址模式：3或4字节
  uint8_t use_qspi;           // 是否使用QSPI模式
} w25qxxx_priv_t;

// ============== 通用操作函数 ==============

// 根据 JEDEC ID 识别芯片型号
w25q_chip_type_t w25qxxx_identify_chip(uint32_t jedec_id);

// 根据芯片型号获取容量
uint32_t w25qxxx_get_capacity(w25q_chip_type_t type);

// 根据芯片型号获取名称
const char *w25qxxx_get_name(w25q_chip_type_t type);

// 填充芯片信息
void w25qxxx_fill_info(flash_info_t *info, w25q_chip_type_t type,
                       uint32_t jedec_id);

// ============== 底层SPI操作（供驱动实现使用） ==============

// 写使能
flash_error_t w25qxxx_write_enable(w25qxxx_priv_t *priv);

// 等待忙结束
flash_error_t w25qxxx_wait_busy(w25qxxx_priv_t *priv, uint32_t timeout_ms);

// 读取状态寄存器1
uint8_t w25qxxx_read_sr1(w25qxxx_priv_t *priv);

// 读取 JEDEC ID
uint32_t w25qxxx_read_jedec_id(w25qxxx_priv_t *priv);

// 发送命令（无数据）
flash_error_t w25qxxx_send_cmd(w25qxxx_priv_t *priv, uint8_t cmd);

// 读取数据（3字节地址）
flash_error_t w25qxxx_read_data_3b(w25qxxx_priv_t *priv, uint32_t addr,
                                   uint8_t *buf, uint32_t len);

// 读取数据（4字节地址）
flash_error_t w25qxxx_read_data_4b(w25qxxx_priv_t *priv, uint32_t addr,
                                   uint8_t *buf, uint32_t len);

// 页编程（3字节地址）
flash_error_t w25qxxx_page_program_3b(w25qxxx_priv_t *priv, uint32_t addr,
                                      const uint8_t *buf, uint32_t len);

// 页编程（4字节地址）
flash_error_t w25qxxx_page_program_4b(w25qxxx_priv_t *priv, uint32_t addr,
                                      const uint8_t *buf, uint32_t len);

// 扇区擦除（3字节地址）
flash_error_t w25qxxx_sector_erase_3b(w25qxxx_priv_t *priv, uint32_t addr);

// 扇区擦除（4字节地址）
flash_error_t w25qxxx_sector_erase_4b(w25qxxx_priv_t *priv, uint32_t addr);

// 块擦除（3字节地址）
flash_error_t w25qxxx_block_erase_3b(w25qxxx_priv_t *priv, uint32_t addr);

// 块擦除（4字节地址）
flash_error_t w25qxxx_block_erase_4b(w25qxxx_priv_t *priv, uint32_t addr);

// 全片擦除
flash_error_t w25qxxx_chip_erase(w25qxxx_priv_t *priv);

// 进入4字节地址模式
flash_error_t w25qxxx_enter_4b_mode(w25qxxx_priv_t *priv);

// 退出4字节地址模式
flash_error_t w25qxxx_exit_4b_mode(w25qxxx_priv_t *priv);

// 电源管理
flash_error_t w25qxxx_power_down(w25qxxx_priv_t *priv);
flash_error_t w25qxxx_power_up(w25qxxx_priv_t *priv);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_W25QXXX_W25QXXX_COMMON_H_ */
