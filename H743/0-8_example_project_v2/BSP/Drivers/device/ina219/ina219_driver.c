/*
 * ina219_driver.c
 *
 *  Created on: Feb 20, 2026
 *      Author: Antigravity
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "INA219"
#include "elog.h"

#include "MemPool.h"
#include "Sys.h"
#include "ina219_driver.h"

#define INA219_MEMSOURCE SYS_MEM_INTERNAL
// 驱动程序配置
#define INA219_SAMPLING_PERIOD_MS 100   // 10Hz 采样频率
#define INA219_CURRENT_DEADZONE_MA 0.5f // 电流计死区
#define INA219_ADC_AVG_64                                                      \
  (INA219_CFG_BADCRES_12BIT_64S_34MS |                                         \
   INA219_CFG_SADCRES_12BIT_64S_34MS) // 芯片ADC周期
#define INA219_DEFAULT_CONFIG                                                  \
  (INA219_CFG_BVOLT_RANGE_32V | INA219_CFG_SVOLT_RANGE_320MV |                 \
   INA219_ADC_AVG_64 | INA219_CFG_MODE_SANDBVOLT_CONTINUOUS)

/*----------------------------------------------------------------------------*/
// 私有函数声明
static void ina219_i2c_callback(void *context, i2c_event_t event, void *args);
static void ina219_timer_callback(void *context);
static void ina219_fsm_step(ina219_driver_t *self, ina219_state_t next_state);
static int ina219_compute_calibration(ina219_driver_t *self);
// 内部寄存器地址
#define INA219_REG_CONFIG (uint8_t)(0x00)       // 配置寄存器 (R/W)
#define INA219_REG_SHUNTVOLTAGE (uint8_t)(0x01) // 分流电压寄存器 (R)
#define INA219_REG_BUSVOLTAGE (uint8_t)(0x02)   // 总线电压寄存器 (R)
#define INA219_REG_POWER (uint8_t)(0x03)        // 功率寄存器 (R)
#define INA219_REG_CURRENT (uint8_t)(0x04)      // 电流寄存器 (R)
#define INA219_REG_CALIBRATION (uint8_t)(0x05)  // 校准寄存器 (R/W)

/*----------------------------------------------------------------------------*/
// 配置位宏
#define INA219_CFGB_RESET(x)                                                   \
  (uint16_t)((x & 0x01) << 15) // 复位 1位,置1产生系统复位,该位自清零
#define INA219_CFGB_BUSV_RANGE(x)                                              \
  (uint16_t)((x & 0x01) << 13) // 总线电压范围 1位(0=16V，1=32V)
#define INA219_CFGB_PGA_RANGE(x)                                               \
  (uint16_t)((x & 0x03)                                                        \
             << 11) // 增益范围 2位(00=±40mV,01=±80mV,10=160mV,11=320mV)
#define INA219_CFGB_BADC_RES_AVG(x)                                            \
  (uint16_t)((x & 0x0F) << 7) // 总线ADC分辨率/平均值设置 4位(总线电压寄存器02h)
#define INA219_CFGB_SADC_RES_AVG(x)                                            \
  (uint16_t)((x & 0x0F) << 3) // 分流ADC分辨率/平均值设置 4位(分流电压寄存器01h)
#define INA219_CFGB_MODE(x) (uint16_t)(x & 0x07) // 工作模式 3位

/*----------------------------------------------------------------------------*/
// 寄存器配置
#define INA219_CFG_RESET INA219_CFGB_RESET(1) // 复位

#define INA219_CFG_BVOLT_RANGE_MASK                                            \
  INA219_CFGB_BUSV_RANGE(1)                                  // 总线电压范围掩码
#define INA219_CFG_BVOLT_RANGE_16V INA219_CFGB_BUSV_RANGE(0) // 0-16V
#define INA219_CFG_BVOLT_RANGE_32V INA219_CFGB_BUSV_RANGE(1) // 0-32V (默认)

#define INA219_CFG_SVOLT_RANGE_MASK INA219_CFGB_PGA_RANGE(3) // 分流电压范围掩码
#define INA219_CFG_SVOLT_RANGE_40MV                                            \
  INA219_CFGB_PGA_RANGE(0) // Gain 1, 40mV Range
#define INA219_CFG_SVOLT_RANGE_80MV                                            \
  INA219_CFGB_PGA_RANGE(1) // Gain /2, 80mV Range
#define INA219_CFG_SVOLT_RANGE_160MV                                           \
  INA219_CFGB_PGA_RANGE(2) // Gain /4, 160mV Range
#define INA219_CFG_SVOLT_RANGE_320MV                                           \
  INA219_CFGB_PGA_RANGE(3) // Gain /8, 320mV Range (default)

#define INA219_CFG_BADCRES_MASK                                                \
  INA219_CFGB_BADC_RES_AVG(15) // 总线电压ADC分辨率/平均值设置掩码
#define INA219_CFG_BADCRES_9BIT_1S_84US                                        \
  INA219_CFGB_BADC_RES_AVG(0) // 1 x 9-bit 单次采样, 84us
#define INA219_CFG_BADCRES_10BIT_1S_148US                                      \
  INA219_CFGB_BADC_RES_AVG(1) // 1 x 10-bit 采样, 148us
#define INA219_CFG_BADCRES_11BIT_1S_276US                                      \
  INA219_CFGB_BADC_RES_AVG(2) // 1 x 11-bit 采样, 276us
#define INA219_CFG_BADCRES_12BIT_1S_532US                                      \
  INA219_CFGB_BADC_RES_AVG(3) // 1 x 12-bit 采样 (default), 532us
#define INA219_CFG_BADCRES_12BIT_2S_1MS                                        \
  INA219_CFGB_BADC_RES_AVG(9) // 2 x 12-bit 多次采样均值, 1.06ms
#define INA219_CFG_BADCRES_12BIT_4S_2MS                                        \
  INA219_CFGB_BADC_RES_AVG(10) // 4 x 12-bit 均值, 2.13ms
#define INA219_CFG_BADCRES_12BIT_8S_4MS                                        \
  INA219_CFGB_BADC_RES_AVG(11) // 8 x 12-bit 均值, 4.26ms
#define INA219_CFG_BADCRES_12BIT_16S_8MS                                       \
  INA219_CFGB_BADC_RES_AVG(12) // 16 x 12-bit 均值, 8.51ms
#define INA219_CFG_BADCRES_12BIT_32S_17MS                                      \
  INA219_CFGB_BADC_RES_AVG(13) // 32 x 12-bit 均值, 17.02ms
#define INA219_CFG_BADCRES_12BIT_64S_34MS                                      \
  INA219_CFGB_BADC_RES_AVG(14) // 64 x 12-bit 均值,  34.05ms
#define INA219_CFG_BADCRES_12BIT_128S_68MS                                     \
  INA219_CFGB_BADC_RES_AVG(15) // 128 x 12-bit 均值, 68.10ms

#define INA219_CFG_SADCRES_MASK                                                \
  INA219_CFGB_SADC_RES_AVG(15) // 分流电阻电压ADC分辨率/平均值设置掩码
#define INA219_CFG_SADCRES_9BIT_1S_84US                                        \
  INA219_CFGB_SADC_RES_AVG(0) // 1 x 9-bit Shunt sample, 84us
#define INA219_CFG_SADCRES_10BIT_1S_148US                                      \
  INA219_CFGB_SADC_RES_AVG(1) // 1 x 10-bit Shunt sample
#define INA219_CFG_SADCRES_11BIT_1S_276US                                      \
  INA219_CFGB_SADC_RES_AVG(2) // 1 x 11-bit Shunt sample
#define INA219_CFG_SADCRES_12BIT_1S_532US                                      \
  INA219_CFGB_SADC_RES_AVG(3) // 1 x 12-bit Shunt sample (default)
#define INA219_CFG_SADCRES_12BIT_2S_1MS                                        \
  INA219_CFGB_SADC_RES_AVG(9) // 2 x 12-bit Shunt samples averaged together
#define INA219_CFG_SADCRES_12BIT_4S_2MS                                        \
  INA219_CFGB_SADC_RES_AVG(10) // 4 x 12-bit Shunt samples averaged together
#define INA219_CFG_SADCRES_12BIT_8S_4MS                                        \
  INA219_CFGB_SADC_RES_AVG(11) // 8 x 12-bit Shunt samples averaged together
#define INA219_CFG_SADCRES_12BIT_16S_8MS                                       \
  INA219_CFGB_SADC_RES_AVG(12) // 16 x 12-bit Shunt samples averaged together
#define INA219_CFG_SADCRES_12BIT_32S_17MS                                      \
  INA219_CFGB_SADC_RES_AVG(13) // 32 x 12-bit Shunt samples averaged together
#define INA219_CFG_SADCRES_12BIT_64S_34MS                                      \
  INA219_CFGB_SADC_RES_AVG(14) // 64 x 12-bit Shunt samples averaged together
#define INA219_CFG_SADCRES_12BIT_128S_68MS                                     \
  INA219_CFGB_SADC_RES_AVG(15) // 128 x 12-bit Shunt samples averaged together

#define INA219_CFG_MODE_MASK INA219_CFGB_MODE(7)            // 工作模式掩码
#define INA219_CFG_MODE_POWERDOWN INA219_CFGB_MODE(0)       // 停止工作
#define INA219_CFG_MODE_SVOLT_TRIGGERED INA219_CFGB_MODE(1) // 分流电压触发采样
#define INA219_CFG_MODE_BVOLT_TRIGGERED INA219_CFGB_MODE(2) // 总线电压触发采样
#define INA219_CFG_MODE_SANDBVOLT_TRIGGERED                                    \
  INA219_CFGB_MODE(3)                              // 分流和总线电压触发采样
#define INA219_CFG_MODE_ADCOFF INA219_CFGB_MODE(4) // ADC失能
#define INA219_CFG_MODE_SVOLT_CONTINUOUS INA219_CFGB_MODE(5) // 分流电压连续采样
#define INA219_CFG_MODE_BVOLT_CONTINUOUS INA219_CFGB_MODE(6) // 总线电压连续采样
#define INA219_CFG_MODE_SANDBVOLT_CONTINUOUS                                   \
  INA219_CFGB_MODE(7) // 分流和总线电压连续采样(默认)

/*----------------------------------------------------------------------------*/
// 总线电压寄存器
#define INA219_BVOLT_CNVR (uint16_t)(0x0002) // 转换完成
#define INA219_BVOLT_OVF (uint16_t)(0x0001)  // 数学溢出标志

/*----------------------------------------------------------------------------*/
// 初始化核心计算参数
static int ina219_compute_calibration(ina219_driver_t *self) {
  // 1. 计算 Current_LSB = Max_Expected_Current / 2^15
  self->current_lsb = self->config->max_current_A * 1000.0f / 32768.0f;

  // 2. 计算 Calibration Register = 0.04096 / (Current_LSB * Rshunt)
  float cal = 0.04096f / ((self->current_lsb / 1000.0f) *
                          self->config->shunt_resistor_ohm);
  self->cal_value = (uint16_t)cal;

  // 3. 计算 Power_LSB = 20 * Current_LSB
  self->power_lsb = 20.0f * self->current_lsb;

  log_d("Calibration: LSB=%.6f A/bit, Cal=%u, PowerLSB=%.6f mW/bit",
        self->current_lsb / 1000.0f, self->cal_value, self->power_lsb);

  return 0;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief 状态机推动器
 * @param self 驱动实例
 * @param next_state 下一个状态
 */
static void ina219_fsm_step(ina219_driver_t *self, ina219_state_t next_state) {
  // log_d("FSM: %d -> %d", self->fsm_state, next_state);
  self->fsm_state = next_state;
  int status = 0;
  switch (self->fsm_state) {
  case INA219_STATE_WRITE_PTR_BUSV:
    self->i2c_buffer[0] = INA219_REG_BUSVOLTAGE;
    status = I2C_MASTER_TRANSMIT_ASYN(self->i2c_driver, self->config->dev_addr,
                                      self->i2c_buffer, 1);
    break;

  case INA219_STATE_READ_BUSV:
    status = I2C_MASTER_RECEIVE_ASYN(self->i2c_driver, self->config->dev_addr,
                                     self->i2c_buffer, 2);
    break;

  case INA219_STATE_WRITE_PTR_CURR:
    self->i2c_buffer[0] = INA219_REG_CURRENT;
    status = I2C_MASTER_TRANSMIT_ASYN(self->i2c_driver, self->config->dev_addr,
                                      self->i2c_buffer, 1);
    break;

  case INA219_STATE_READ_CURR:
    status = I2C_MASTER_RECEIVE_ASYN(self->i2c_driver, self->config->dev_addr,
                                     self->i2c_buffer, 2);
    break;

  case INA219_STATE_WRITE_PTR_POW:
    self->i2c_buffer[0] = INA219_REG_POWER;
    status = I2C_MASTER_TRANSMIT_ASYN(self->i2c_driver, self->config->dev_addr,
                                      self->i2c_buffer, 1);
    break;

  case INA219_STATE_READ_POW:
    status = I2C_MASTER_RECEIVE_ASYN(self->i2c_driver, self->config->dev_addr,
                                     self->i2c_buffer, 2);
    break;

  default:
    self->fsm_state = INA219_STATE_IDLE;
    return;
  }

  if (status != 0) {
    self->fsm_state = INA219_STATE_IDLE; // 出错重置
  }
}

// I2C 完成中断/DMA 回调
static void ina219_i2c_callback(void *context, i2c_event_t event, void *args) {
  ina219_driver_t *self = (ina219_driver_t *)context;
  if (event == I2C_EVENT_ERROR) {
    log_e("I2C Error at state %d", self->fsm_state);
    self->fsm_state = INA219_STATE_IDLE;
    return;
  }

  switch (self->fsm_state) {
  case INA219_STATE_WRITE_PTR_BUSV:                // 设置地址完成
    ina219_fsm_step(self, INA219_STATE_READ_BUSV); // 进入读取总线电压状态
    break;

  case INA219_STATE_READ_BUSV: { // 读取总线电压完成
    uint16_t raw_v = (self->i2c_buffer[0] << 8) | self->i2c_buffer[1];
    // 总线电压寄存器 [15:3] 包含数据, [2] CNVR, [1] OVF
    self->instant_data.voltage_mV = ((int16_t)raw_v >> 3) * 4.0f; // 4mV per LSB

    ina219_fsm_step(self,
                    INA219_STATE_WRITE_PTR_CURR); // 进入设置电流寄存器地址
    break;
  }

  case INA219_STATE_WRITE_PTR_CURR:
    ina219_fsm_step(self, INA219_STATE_READ_CURR); // 进入读取电流状态
    break;

  case INA219_STATE_READ_CURR: {
    int16_t raw_i = (int16_t)((self->i2c_buffer[0] << 8) | self->i2c_buffer[1]);
    self->instant_data.current_mA = raw_i * self->current_lsb;

    ina219_fsm_step(self, INA219_STATE_WRITE_PTR_POW); // 进入设置功率寄存器地址
    break;
  }

  case INA219_STATE_WRITE_PTR_POW:
    ina219_fsm_step(self, INA219_STATE_READ_POW); // 进入读取功率状态
    break;

  case INA219_STATE_READ_POW: {
    uint16_t raw_p = (self->i2c_buffer[0] << 8) | self->i2c_buffer[1];
    self->instant_data.power_mW = raw_p * self->power_lsb;

    // 4. 死区过滤：处理零位偏移噪声
    if (self->instant_data.current_mA > -INA219_CURRENT_DEADZONE_MA &&
        self->instant_data.current_mA < INA219_CURRENT_DEADZONE_MA) {
      self->instant_data.current_mA = 0;
      self->instant_data.power_mW = 0;
    }

    // 5. 完成一轮采样，进行电量统计
    double interval_h = INA219_SAMPLING_PERIOD_MS / 3600000.0;

    // 电荷累计 (mAh)：支持正负方向（充放电）
    self->accumulated_data.charge_mAh +=
        self->instant_data.current_mA * interval_h;

    // 能量累计 (mWh)：通常指做功消耗，取功率绝对值累计（或者仅正向累计）
    // 此处采用功率绝对值，防止噪声波动导致能量减少
    float p_abs = (self->instant_data.power_mW > 0)
                      ? self->instant_data.power_mW
                      : -self->instant_data.power_mW;
    self->accumulated_data.energy_mWh += p_abs * interval_h;

    log_d("Sample Done: %.2f mV, %.2f mA", self->instant_data.voltage_mV,
          self->instant_data.current_mA);

    self->fsm_state = INA219_STATE_IDLE; // 结束一轮,进入空闲
    break;
  }
  default:
    break;
  }
}

// 定时任务回调 (启动一轮采样和累计)
static void ina219_timer_callback(void *context) {
  ina219_driver_t *self = (ina219_driver_t *)context;
  if (self->fsm_state == INA219_STATE_IDLE) {
    ina219_fsm_step(self, INA219_STATE_WRITE_PTR_BUSV); // 结束空闲进入通信流程
  }
}

/*----------------------------------------------------------------------------*/
// 接口实现
/**
 * @brief 读取瞬时数据(电压,电流,功率)
 *
 * @param dev  驱动实例
 * @param data 返回值
 * @return int 0成功, 非 0失败
 */
static int ina219_read_instant(PowerMonitor_driver_t *drv,
                               Power_Instant_Data_t *data) {
  ina219_driver_t *self = (ina219_driver_t *)drv;
  memcpy(data, &self->instant_data, sizeof(Power_Instant_Data_t));
  return 0;
}

/**
 * @brief 读取累计数据(电量,电荷)
 *
 * @param dev  驱动实例
 * @param data 累计数据
 * @return int 0成功, 非 0失败
 */
static int ina219_read_accumulated(PowerMonitor_driver_t *drv,
                                   Power_Accumulated_Data_t *data) {
  ina219_driver_t *self = (ina219_driver_t *)drv;
  memcpy(data, &self->accumulated_data, sizeof(Power_Accumulated_Data_t));
  return 0;
}

/**
 * @brief 重置累计数据
 *
 * @param dev 驱动实例
 * @return int 0成功, 非 0失败
 */
static int ina219_reset_counters(PowerMonitor_driver_t *drv) {
  ina219_driver_t *self = (ina219_driver_t *)drv;
  self->accumulated_data.charge_mAh = 0;
  self->accumulated_data.energy_mWh = 0;
  return 0;
}

static const PowerMonitor_Ops_t ina219_ops = {
    .read_instant = ina219_read_instant,
    .read_accumulated = ina219_read_accumulated,
    .reset_counters = ina219_reset_counters,
    .set_over_current_limit = NULL};

PowerMonitor_driver_t *ina219_create(i2c_driver_t *i2c, timer_driver_t *timer,
                                     ina219_config_t *config) {
  if (!i2c || !timer || !config)
    return NULL;

  ina219_driver_t *self =
      (ina219_driver_t *)sys_malloc(INA219_MEMSOURCE, sizeof(ina219_driver_t));
  if (self) {
    memset(self, 0, sizeof(ina219_driver_t));
    self->base.ops = &ina219_ops;
    self->i2c_driver = i2c;
    self->timer = timer;
    self->config = config;

    // 1. 初始化计算
    ina219_compute_calibration(self);

    // 2. 硬件初始化 (同步，确保到位)
    uint8_t data[3];
    // 复位并配置
    data[0] = INA219_REG_CONFIG;
    data[1] = (INA219_DEFAULT_CONFIG >> 8) & 0xFF;
    data[2] = INA219_DEFAULT_CONFIG & 0xFF;
    I2C_MASTER_TRANSMIT(i2c, config->dev_addr, data, 3, 100);

    // 写入校准值
    data[0] = INA219_REG_CALIBRATION;
    data[1] = (self->cal_value >> 8) & 0xFF;
    data[2] = self->cal_value & 0xFF;
    I2C_MASTER_TRANSMIT(i2c, config->dev_addr, data, 3, 100);

    // 3. 注册回调与启动定时采样
    I2C_SET_CALLBACK(i2c, ina219_i2c_callback, self);       // 设置I2C回调
    TIMER_SET_CALLBACK(timer, ina219_timer_callback, self); // 设置定时任务回调
    TIMER_SET_PERIOD(timer, INA219_SAMPLING_PERIOD_MS);     // 设置任务间隔
    TIMER_START(timer);                                     // 启动任务

    log_d("Driver created at addr 0x%02X", config->dev_addr);
  } else {
    log_e("Failed to allocate driver memory");
  }
  return (PowerMonitor_driver_t *)self;
}
