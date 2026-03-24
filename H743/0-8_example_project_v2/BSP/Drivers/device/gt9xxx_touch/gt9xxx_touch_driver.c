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
#include "MemPool.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>


//=========================
// 触摸点寄存器地址表
//=========================

static const uint16_t GT9XXX_TPX_TBL[10] = {
    GT9XXX_TP1_REG, GT9XXX_TP2_REG,  GT9XXX_TP3_REG, GT9XXX_TP4_REG,
    GT9XXX_TP5_REG, GT9XXX_TP6_REG,  GT9XXX_TP7_REG, GT9XXX_TP8_REG,
    GT9XXX_TP9_REG, GT9XXX_TP10_REG,
};

//=========================
// 私有辅助函数
//=========================

#define isPressed(drv) (drv)->base.touch_data.state
#define Call_Touch_Data(dev) (drv)->base.touch_data
/**
 * @brief 向 GT9xxx 写入数据
 */
static inline int gt9xxx_wr_reg(gt9xxx_touch_driver_t *drv, uint16_t reg,
                                uint8_t *buf, uint8_t len) {
  return I2C_MEM_WRITE(drv->bus.i2c, drv->bus.addr_mode << 1, reg,
                       I2C_MEMADD_SIZE_16BIT, buf, len, 100);
}

/**
 * @brief 从 GT9xxx 读取数据
 */
static int gt9xxx_rd_reg(gt9xxx_touch_driver_t *drv, uint16_t reg, uint8_t *buf,
                         uint8_t len) {
  return I2C_MEM_READ(drv->bus.i2c, drv->bus.addr_mode << 1, reg,
                      I2C_MEMADD_SIZE_16BIT, buf, len, 100);
}

/**
 * @brief 设置 RST 引脚电平
 */
static inline void gt9xxx_rst_set(gt9xxx_touch_driver_t *drv, uint8_t value) {
  GPIO_WRITE(drv->bus.rst_gpio, value);
}

/**
 * @brief 设置 INT 引脚电平（用于设置地址模式）
 */
static inline void gt9xxx_int_set(gt9xxx_touch_driver_t *drv, uint8_t value) {
  GPIO_WRITE(drv->bus.int_gpio, value);
}

//=========================
// touch_driver 接口实现
//=========================

/**
 * @brief 初始化 GT9xxx 触摸屏
 */
static int gt9xxx_init(touch_driver_t *self) {
  gt9xxx_touch_driver_t *drv = (gt9xxx_touch_driver_t *)self;
  uint8_t temp[5];

  // 1. 设置复位时序确定 I2C 地址
  GPIO_SET_MODE(drv->bus.int_gpio, GPIO_PushPullOutput);
  if (drv->bus.addr_mode == GT9XXX_ADDR_0x14) {
    gt9xxx_int_set(drv, 1); // INT=HIGH for 0x14
  } else {
    gt9xxx_int_set(drv, 0); // INT=LOW for 0x5D
  }

  gt9xxx_rst_set(drv, 0); // 复位
  sys_delay_ms(10);
  gt9xxx_rst_set(drv, 1); // 释放复位
  sys_delay_ms(10);

  // 释放 INT 引脚为浮空输入模式
  GPIO_SET_MODE(drv->bus.int_gpio, GPIO_FloatInput);
  sys_delay_ms(100);

  // 2. 读取产品 ID
  if (gt9xxx_rd_reg(drv, GT9XXX_PID_REG, temp, 4) != 0) {
    return -1;
  }
  temp[4] = 0;

  // 3. 验证并识别芯片型号
  if (strcmp((char *)temp, "911") == 0 || strcmp((char *)temp, "9147") == 0 ||
      strcmp((char *)temp, "1158") == 0) {
    drv->max_points = 5;
  } else if (strcmp((char *)temp, "9271") == 0) {
    drv->max_points = 10;
  } else {
    // 默认回退到 5 点
    drv->max_points = 5;
  }
  strncpy(drv->device_id, (char *)temp, sizeof(drv->device_id) - 1);

  // 4. 软复位
  temp[0] = 0x02;
  gt9xxx_wr_reg(drv, GT9XXX_CTRL_REG, temp, 1);
  sys_delay_ms(10);
  temp[0] = 0x00;
  gt9xxx_wr_reg(drv, GT9XXX_CTRL_REG, temp, 1);

  return 0;
}

/**
 * @brief 扫描触摸屏获取触摸数据
 */
static int gt9xxx_scan(touch_driver_t *self) {
  gt9xxx_touch_driver_t *drv = (gt9xxx_touch_driver_t *)self;
  uint8_t state = 0;
  uint8_t cmd = 0;
  uint8_t temp[4];

  // 读取触摸状态
  if (gt9xxx_rd_reg(drv, GT9XXX_GSTID_REG, &state, 1) != 0) {
    return -2;
  }

  // 检查是否有有效采集数据 (Buffer Ready bit)
  if (!(state & 0x80)) {
    return 0; // 无新坐标
  }

  // 获取触摸点数量
  uint8_t touch_count = state & 0x0F;
  if (touch_count > TOUCH_MAX_POINTS) {
    touch_count = 0; // 异常
  }

  drv->base.touch_data.point_count = touch_count;
  drv->base.touch_data.state =
      (touch_count > 0) ? TOUCH_STATE_PRESSED : TOUCH_STATE_RELEASED;

  // 读取每个触摸点的坐标
  for (uint8_t i = 0; i < touch_count; i++) {
    if (gt9xxx_rd_reg(drv, GT9XXX_TPX_TBL[i], temp, 4) != 0) {
      break;
    }
    drv->base.touch_data.points[i].x = ((uint16_t)temp[1] << 8) | temp[0];
    drv->base.touch_data.points[i].y = ((uint16_t)temp[3] << 8) | temp[2];
  }

  // 清除标志位，准备下一次采集
  cmd = 0;
  gt9xxx_wr_reg(drv, GT9XXX_GSTID_REG, &cmd, 1);

  return (touch_count > 0) ? 1 : 0;
}

static Touch_Data_t *gt9xxx_get_touch_data(touch_driver_t *self) {
  return &self->touch_data;
}

// 接口实现
static const touch_driver_ops_t gt9xxx_touch_ops = {
    .init = gt9xxx_init,
    .get_touch_data = gt9xxx_get_touch_data,
    .scan = gt9xxx_scan,
};

//===========
// 公共函数
//===========
touch_driver_t *gt9xxx_touch_create(const gt9xxx_bus_t *bus) {
  if (bus == NULL || bus->i2c == NULL || bus->rst_gpio == NULL ||
		  bus->int_gpio == NULL) {
    return NULL;
  }

  gt9xxx_touch_driver_t *drv = (gt9xxx_touch_driver_t *)sys_malloc(
      SYS_MEM_INTERNAL, sizeof(gt9xxx_touch_driver_t));
  if (drv) {
    memset(drv, 0, sizeof(gt9xxx_touch_driver_t));
    drv->base.ops = &gt9xxx_touch_ops;
    drv->bus = *bus;
  }

  return (touch_driver_t *)drv;
}

void gt9xxx_touch_destroy(gt9xxx_touch_driver_t *drv) {
  if (drv) {
    sys_free(SYS_MEM_INTERNAL, drv);
  }
}
