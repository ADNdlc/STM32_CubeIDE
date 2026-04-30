#ifndef BSP_DRIVERS_FACTORY_DEVICE_ABSOLUTE_ENCODER_FACTORY_H_
#define BSP_DRIVERS_FACTORY_DEVICE_ABSOLUTE_ENCODER_FACTORY_H_

#include "interface_inc.h"
#include "dev_map.h"

/**
 * @brief 获取绝对值编码器驱动实例
 * @param id 编码器逻辑 ID
 * @return 编码器驱动指针，失败返回 NULL
 */
absolute_encoder_driver_t *absolute_encoder_driver_get(absolute_encoder_id_t id);

#endif /* BSP_DRIVERS_FACTORY_DEVICE_ABSOLUTE_ENCODER_FACTORY_H_ */
