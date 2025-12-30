#ifndef DRIVERS_DEVICE_W25QXX_W25Q_ADAPTER_H_
#define DRIVERS_DEVICE_W25QXX_W25Q_ADAPTER_H_

#include "stm32h7xx_hal.h" // For errors or types if needed, or keep generic
#include <stddef.h>
#include <stdint.h>

/**
 * @brief W25QXX 通信传输适配器
 *
 */

typedef struct w25q_adapter_t w25q_adapter_t;

typedef struct {
  int (*init)(w25q_adapter_t *self);                  // 初始化
  int (*read_id)(w25q_adapter_t *self, uint32_t *id); // 读取ID
  int (*read)(w25q_adapter_t *self, uint32_t addr, uint8_t *buf,
              size_t size);                  // 读取
  int (*write_enable)(w25q_adapter_t *self); // 使能写入
  int (*program_page)(w25q_adapter_t *self, uint32_t addr,
                      const uint8_t *buf, // 写入
                      size_t size);
  int (*erase_sector)(w25q_adapter_t *self, uint32_t addr); // 擦除扇区
  int (*erase_block)(w25q_adapter_t *self, uint32_t addr);  // 擦除块
  int (*erase_chip)(w25q_adapter_t *self);                  // 擦除芯片
  int (*wait_busy)(w25q_adapter_t *self, uint32_t timeout); // 等待忙(写入/擦除)
  int (*enter_4byte_addr_mode)(w25q_adapter_t *self);       // 进入4字节地址模式
  int (*exit_4byte_addr_mode)(w25q_adapter_t *self);        // 退出4字节地址模式
  int (*enter_memory_mapped)(w25q_adapter_t *self);         // 进入内存映射模式
  int (*exit_memory_mapped)(w25q_adapter_t *self);          // 退出内存映射模式
} w25q_adapter_ops_t;

struct w25q_adapter_t {
  const w25q_adapter_ops_t *ops;
  void *user_data;
};

#endif /* DRIVERS_DEVICE_W25QXX_W25Q_ADAPTER_H_ */
