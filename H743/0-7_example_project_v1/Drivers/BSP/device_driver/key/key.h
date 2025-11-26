/*
 * key.h
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

#ifndef BSP_DEVICE_DRIVER_KEY_KEY_H_
#define BSP_DEVICE_DRIVER_KEY_KEY_H_


// 前向声明
typedef struct key_t key_t;
typedef struct key_operations key_operations;

// GPIO操作函数指针结构体
struct key_operations {
    uint8_t (*read_pin)(void* port, uint16_t pin);
};

key_t *key_create_with_ops(void* port, uint16_t pin, const key_operations* ops, uint8_t active_level);
void key_delete(key_t *self);



#endif /* BSP_DEVICE_DRIVER_KEY_KEY_H_ */
