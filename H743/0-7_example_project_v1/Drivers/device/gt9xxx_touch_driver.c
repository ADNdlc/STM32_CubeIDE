/*
 * gt9xxx_touch_driver.c
 *
 *  Created on: Dec 13, 2025
 *      Author: Antigravity
 *
 *  GT9xxx 系列触摸屏驱动实现（平台无关）
 *  通过依赖注入的 i2c_driver 和 gpio_driver 实现平台解耦
 */

#include "gt9xxx_touch_driver.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

//==============================================================================
// 触摸点寄存器地址表
//==============================================================================

static const uint16_t GT9XXX_TPX_TBL[10] = {
    GT9XXX_TP1_REG, GT9XXX_TP2_REG,  GT9XXX_TP3_REG, GT9XXX_TP4_REG,
    GT9XXX_TP5_REG, GT9XXX_TP6_REG,  GT9XXX_TP7_REG, GT9XXX_TP8_REG,
    GT9XXX_TP9_REG, GT9XXX_TP10_REG,
};

//==============================================================================
// 私有辅助函数
//==============================================================================

/**
 * @brief 向 GT9xxx 写入数据
 */
static int gt9xxx_wr_reg(gt9xxx_touch_driver_t *drv, uint16_t reg, uint8_t *buf,
                         uint8_t len) {
  return I2C_WRITE_REG(drv->config.i2c, drv->config.addr_mode, reg, buf, len);
}

/**
 * @brief 从 GT9xxx 读取数据
 */
static int gt9xxx_rd_reg(gt9xxx_touch_driver_t *drv, uint16_t reg, uint8_t *buf,
                         uint8_t len) {
  return I2C_READ_REG(drv->config.i2c, drv->config.addr_mode, reg, buf, len);
}

/**
 * @brief 设置 RST 引脚电平
 */
static inline void gt9xxx_rst_set(gt9xxx_touch_driver_t *drv, uint8_t value) {
  GPIO_WRITE(drv->config.rst_gpio, value);
}

/**
 * @brief 设置 INT 引脚电平（用于设置地址模式）
 */
static inline void gt9xxx_int_set(gt9xxx_touch_driver_t *drv, uint8_t value) {
  GPIO_WRITE(drv->config.int_gpio, value);
}

/**
 * @brief 读取 INT 引脚电平
 */
static inline uint8_t gt9xxx_int_read(gt9xxx_touch_driver_t *drv) {
  return GPIO_READ(drv->config.int_gpio);
}

//==============================================================================
// touch_driver 接口实现
//==============================================================================

/**
 * @brief 初始化 GT9xxx 触摸屏
 */
static int gt9xxx_init(touch_driver_t *self) {
  gt9xxx_touch_driver_t *drv = (gt9xxx_touch_driver_t *)self;
  uint8_t temp[5];

  if (drv->initialized) {
    return 0;
  }

  // 初始化 I2C
  I2C_INIT(drv->config.i2c);

  // 复位时序：根据地址模式设置 INT 引脚状态
  // 地址 0x14: 释放 RST 时 INT 为高
  // 地址 0x5D: 释放 RST 时 INT 为低
  if (drv->config.addr_mode == GT9XXX_ADDR_0x14) {
    gt9xxx_int_set(drv, 1); // INT=HIGH for 0x14
  } else {
    gt9xxx_int_set(drv, 0); // INT=LOW for 0x5D
  }

  gt9xxx_rst_set(drv, 0); // 复位
  sys_delay_ms(10);
  gt9xxx_rst_set(drv, 1); // 释放复位
  sys_delay_ms(10);

  // INT 引脚转为输入模式（浮空）
  // 注意：这里假设 gpio_driver 在写入后自动保持状态
  // 实际应用中可能需要重新配置 GPIO 为输入模式
  sys_delay_ms(100);

  // 读取产品 ID
  if (gt9xxx_rd_reg(drv, GT9XXX_PID_REG, temp, 4) != 0) {
    return -1;
  }
  temp[4] = 0;

  // 验证产品 ID
  if (strcmp((char *)temp, "911") && strcmp((char *)temp, "9147") &&
      strcmp((char *)temp, "1158") && strcmp((char *)temp, "9271")) {
    // 未识别的触摸芯片
    return -2;
  }

  // 保存设备 ID
  strncpy(drv->device_id, (char *)temp, sizeof(drv->device_id) - 1);
  drv->device_id[sizeof(drv->device_id) - 1] = '\0';

  // 根据芯片型号设置最大触摸点数
  if (strcmp((char *)temp, "9271") == 0) {
    drv->max_points = 10; // GT9271 支持 10 点触摸
  } else {
    drv->max_points = 5; // 其他芯片支持 5 点触摸
  }

  // 软复位
  temp[0] = 0x02;
  gt9xxx_wr_reg(drv, GT9XXX_CTRL_REG, temp, 1);
  sys_delay_ms(10);

  // 结束复位，进入读坐标状态
  temp[0] = 0x00;
  gt9xxx_wr_reg(drv, GT9XXX_CTRL_REG, temp, 1);

  drv->initialized = 1;
  return 0;
}

/**
 * @brief 扫描触摸屏获取触摸数据
 * @return 0=无触摸, 1=有触摸, <0=错误
 */
static int gt9xxx_scan(touch_driver_t *self, touch_data_t *data) {
  gt9xxx_touch_driver_t *drv = (gt9xxx_touch_driver_t *)self;
  uint8_t temp[4];
  uint8_t state = 0;
  uint8_t cmd = 0;

  if (data == NULL) {
    return -1;
  }

  // 初始化输出
  data->count = 0;

  // 读取触摸状态
  if (gt9xxx_rd_reg(drv, GT9XXX_GSTID_REG, &state, 1) != 0) {
    return -2;
  }

  // 检查是否有有效数据
  if (state & 0x80) {
    // 清除状态标志
    cmd = 0;
    gt9xxx_wr_reg(drv, GT9XXX_GSTID_REG, &cmd, 1);
  } else {
    return 0; // 无触摸
  }

  // 获取触摸点数量
  uint8_t touch_count = state & 0x0F;
  if (touch_count == 0 || touch_count > drv->max_points) {
    return 0; // 无效触摸
  }

  // 读取每个触摸点的坐标
  for (uint8_t i = 0; i < touch_count; i++) {
    if (gt9xxx_rd_reg(drv, GT9XXX_TPX_TBL[i], temp, 4) != 0) {
      return -3;
    }
    data->points[i].x = ((uint16_t)temp[1] << 8) | temp[0];
    data->points[i].y = ((uint16_t)temp[3] << 8) | temp[2];
  }
  data->count = touch_count;

  return 1; // 有触摸
}

/**
 * @brief 获取支持的最大触摸点数
 */
static uint8_t gt9xxx_get_max_points(touch_driver_t *self) {
  gt9xxx_touch_driver_t *drv = (gt9xxx_touch_driver_t *)self;
  return drv->max_points;
}

/**
 * @brief 获取设备ID字符串
 */
static const char *gt9xxx_get_device_id(touch_driver_t *self) {
  gt9xxx_touch_driver_t *drv = (gt9xxx_touch_driver_t *)self;
  return drv->device_id;
}

//==============================================================================
// 虚函数表
//==============================================================================

static const touch_driver_ops_t gt9xxx_touch_ops = {
    .init = gt9xxx_init,
    .scan = gt9xxx_scan,
    .get_max_points = gt9xxx_get_max_points,
    .get_device_id = gt9xxx_get_device_id,
};

//==============================================================================
// 公共函数
//==============================================================================

gt9xxx_touch_driver_t *gt9xxx_touch_create(const gt9xxx_config_t *config) {
  if (config == NULL || config->i2c == NULL || config->rst_gpio == NULL ||
      config->int_gpio == NULL) {
    return NULL;
  }

  gt9xxx_touch_driver_t *drv =
      (gt9xxx_touch_driver_t *)malloc(sizeof(gt9xxx_touch_driver_t));
  if (drv) {
    memset(drv, 0, sizeof(gt9xxx_touch_driver_t));
    drv->base.ops = &gt9xxx_touch_ops;
    drv->config = *config;
    drv->max_points = 5; // 默认 5 点触摸
    drv->initialized = 0;
  }

  return drv;
}

void gt9xxx_touch_destroy(gt9xxx_touch_driver_t *drv) {
  if (drv) {
    free(drv);
  }
}
