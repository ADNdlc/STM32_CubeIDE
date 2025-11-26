/*
 * key.c
 *
 *  Created on: Nov 26, 2025
 *      Author: 12114
 */

typedef enum {
    KEY_STATE_OFF = 0,
    KEY_STATE_ON
} key_state_t;

typedef key_state_t (*key_get_state_func_t)(key_t *self);

struct key_t{
    const key_vtable_t *vtable; // 虚函数表指针
    void* port;
    uint16_t pin;
    const key_operations_t* ops;
    uint8_t active_level;
};

