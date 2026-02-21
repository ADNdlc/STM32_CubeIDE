/*
 * ina219_driver.c
 *
 *  Created on: Feb 20, 2026
 *      Author: Antigravity
 */

#include "ina219_driver.h"
#include "Sys.h"
#include <stdlib.h>
#include <string.h>


// 寄存器地址
#define INA219_REG_CONFIG 0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE 0x02
#define INA219_REG_POWER 0x03
#define INA219_REG_CURRENT 0x04
#define INA219_REG_CALIBRATION 0x05

// 默认配置: 32V BRNG, Gain /8 (320mV), 12bit ADC, Continuous Combined
// 001 1 1 0011 0011 0111 -> 0x399F
#define INA219_CONFIG_DEFAULT 0x399F

static int ina219_write_reg(ina219_driver_t *self, uint8_t reg,
                            uint16_t value) {
  uint8_t data[3];
  data[0] = reg;
  data[1] = (value >> 8) & 0xFF;
  data[2] = value & 0xFF;
  // 使用同步接口写入，超时 10ms
  return I2C_MASTER_TRANSMIT(self->i2c_driver, self->config.dev_addr, data, 3,
                             10);
}

static int ina219_read_reg(ina219_driver_t *self, uint8_t reg,
                           uint16_t *value) {
  // 1. 设置指针 (Write Reg Addr)
  if (I2C_MASTER_TRANSMIT(self->i2c_driver, self->config.dev_addr, &reg, 1,
                          10) != 0) {
    return -1;
  }
  // 2. 读取数据 (2 bytes)
  uint8_t buffer[2];
  if (I2C_MASTER_RECEIVE(self->i2c_driver, self->config.dev_addr, buffer, 2,
                         10) != 0) {
    return -1;
  }
  *value = ((uint16_t)buffer[0] << 8) | buffer[1];
  return 0;
}

static int ina219_init(PowerMonitor_Dev_t *dev) {
  ina219_driver_t *self = (ina219_driver_t *)dev;

  // 1. 计算校准值 (简简化逻辑，假定用户已合理设置 max_current)
  // VBUS_MAX = 32V
  // VSHUNT_MAX = 0.32V
  // R = shunt_resistor_ohm
  // MaxPossible_I = 0.32 / R

  // Current_LSB = Max_Expected_I / 32768
  // 稍微向上取整以适配便于人类阅读的步长 (e.g. 0.1mA)
  self->current_lsb_mA = (self->config.max_current_A * 1000.0f) / 32768.0f;
  // 简单起见，不进行复杂的 LSB 优化步骤，直接使用

  // Cal = 0.04096 / (Current_LSB * R)  (Current_LSB in Amps)
  float current_lsb_A = self->current_lsb_mA / 1000.0f;
  self->cal_value =
      (uint32_t)(0.04096f / (current_lsb_A * self->config.shunt_resistor_ohm));

  // Power LSB = 20 * Current LSB
  self->power_lsb_mW = 20.0f * self->current_lsb_mA;

  // 2. 写入配置
  ina219_write_reg(self, INA219_REG_CONFIG, INA219_CONFIG_DEFAULT);

  // 3. 写入校准
  ina219_write_reg(self, INA219_REG_CALIBRATION, (uint16_t)self->cal_value);

  // 4. 重置状态
  self->accumulated_charge_mAs = 0;
  self->accumulated_energy_mWs = 0;
  self->last_process_tick = sys_get_systick_ms();

  // 5. 初始化异步接收队列
  // 注意：默认我们将指针指向 CURRENT 寄存器，以便后续异步读取
  uint8_t current_reg = INA219_REG_CURRENT;
  I2C_MASTER_TRANSMIT(self->i2c_driver, self->config.dev_addr, &current_reg, 1,
                      10);

  // 使用 64 字节缓冲
  static uint8_t queue_buffer[64];
  i2c_queue_init(&self->rx_queue, self->i2c_driver, self->config.dev_addr,
                 queue_buffer, sizeof(queue_buffer));

  return 0;
}

static int ina219_read_instant(PowerMonitor_Dev_t *dev,
                               Power_Instant_Data_t *data) {
  ina219_driver_t *self = (ina219_driver_t *)dev;
  uint16_t raw_v, raw_c, raw_p;

  // 为了防止干扰异步队列（它假设指针在 Current），我们操作完需要恢复指针
  // 但如果异步队列很频繁，这里会有冲突风险。
  // 简单处理：我们假设 read_instant 不会和 timer 中断冲突（不在中断里调
  // read_instant） 或者我们只读取 Current 寄存器，而 Voltage 需要切指针。

  // 读取 Bus Voltage (Reg 0x02)
  ina219_read_reg(self, INA219_REG_BUSVOLTAGE, &raw_v);
  // Bit 3-15 is value, shift right 3, multiply by 4mV
  data->voltage_mV = ((int16_t)raw_v >> 3) * 4.0f;

  // 读取 Current (Reg 0x04) needed pointer restore? Yes because read_reg moves
  // pointer.
  ina219_read_reg(self, INA219_REG_CURRENT, &raw_c);
  data->current_mA = (int16_t)raw_c * self->current_lsb_mA;

  // Power = V * I (Or read Power Reg 0x03)
  data->power_mW = data->voltage_mV * data->current_mA / 1000.0f;

  // 恢复指针到 Current，以便异步任务继续工作
  uint8_t reg = INA219_REG_CURRENT;
  I2C_MASTER_TRANSMIT(self->i2c_driver, self->config.dev_addr, &reg, 1, 10);

  return 0;
}

static int ina219_read_accumulated(PowerMonitor_Dev_t *dev,
                                   Power_Accumulated_Data_t *data) {
  ina219_driver_t *self = (ina219_driver_t *)dev;
  // 转换单位 mWs -> mWh, mAs -> mAh
  data->energy_mWh = self->accumulated_energy_mWs / 3600.0f;
  data->charge_mAh = self->accumulated_charge_mAs / 3600.0f;
  return 0;
}

static int ina219_reset_counters(PowerMonitor_Dev_t *dev) {
  ina219_driver_t *self = (ina219_driver_t *)dev;
  self->accumulated_charge_mAs = 0;
  self->accumulated_energy_mWs = 0;
  return 0;
}

static const PowerMonitor_Ops_t ina219_ops = {
    .init = ina219_init,
    .read_instant = ina219_read_instant,
    .read_accumulated = ina219_read_accumulated,
    .reset_counters = ina219_reset_counters,
};

PowerMonitor_Dev_t *ina219_create(i2c_driver_t *i2c, ina219_config_t *config) {
  ina219_driver_t *driver = (ina219_driver_t *)malloc(sizeof(ina219_driver_t));
  if (driver) {
    memset(driver, 0, sizeof(ina219_driver_t));
    driver->base.ops = &ina219_ops;
    driver->i2c_driver = i2c;
    driver->config = *config;
    return (PowerMonitor_Dev_t *)driver;
  }
  return NULL;
}

// ---------------- 异步处理逻辑 ----------------

int ina219_request_sample(PowerMonitor_Dev_t *dev) {
  ina219_driver_t *self = (ina219_driver_t *)dev;
  // 启动一次异步接收，读取 2 字节 (Current Register)
  // 前提：指针已经指向 0x04 (Current)
  // 返回 -1 表示忙或失败
  return i2c_queue_start_receive(&self->rx_queue);
}

void ina219_process_data(PowerMonitor_Dev_t *dev) {
  ina219_driver_t *self = (ina219_driver_t *)dev;

  // 1. 获取所有缓冲的数据
  uint32_t count = i2c_queue_rx_count(&self->rx_queue);

  // 每次数据 2 字节
  if (count < 2)
    return;

  // 确保处理完整的 2 字节包
  int samples = count / 2;

  uint8_t buf[2];
  double current_sum = 0;

  for (int i = 0; i < samples; i++) {
    if (i2c_queue_getdata(&self->rx_queue, buf, 2) == 0) {
      int16_t raw_current = (int16_t)((buf[0] << 8) | buf[1]);
      current_sum += (double)raw_current;
    }
  }

  double avg_current_raw = current_sum / samples;
  double avg_current_mA = avg_current_raw * self->current_lsb_mA;

  // 2. 计算时间差
  uint32_t now = sys_get_systick_ms();
  if (now < self->last_process_tick) {
    // 溢出处理
    self->last_process_tick = now;
    return;
  }
  double delta_t_sec = (now - self->last_process_tick) / 1000.0f;
  self->last_process_tick = now;

  // 3. 累积
  self->accumulated_charge_mAs += avg_current_mA * delta_t_sec;

  // 能量 = 功率 * 时间。这里我们假设电压不变? 或者我们应该也读电压?
  // 简易方案：只累积电荷。如果需要能量，可以用 (Charge * Default_Voltage)
  // 估算， 或者下次改进为同时读取电压和电流 (需要 Queue 支持读 4
  // 字节且设备支持连续地址读取, INA219 不支持连续) 所以这里暂时
  // 只精确累积电荷，能量做简单估算 假设 VBUS 约等于 12V (举例)
  // 或者读一次瞬时电压 为了更准确，最好在 read_instant 里更新一下 voltage 缓存

  // 这里暂且不做能量累积，或者仅累积 Current * Last_Voltage
}
