#ifndef BSP_STM32H743_BOARD_V1_DEVICE_MAPPING_H_
#define BSP_STM32H743_BOARD_V1_DEVICE_MAPPING_H_

#include "stm32h7xx_hal.h"

/*********************
 *      标识枚举
 *********************/
// GPIO 设备逻辑标识枚举
typedef enum {
  GPIO_LED_0 = 0,
  GPIO_BUTTON_KEYUP,
  GPIO_BUTTON_KEY0,
  GPIO_BUTTON_KEY1,
  GPIO_BUTTON_KEY2,
  // 触摸屏控制引脚
  GPIO_TOUCH_RST,
  GPIO_TOUCH_INT,
  // esp8266复位
  GPIO_ESP_RST,
  //...
  GPIO_MAX_DEVICES
} gpio_device_id_t;

// PWM 设备逻辑标识枚举
typedef enum {
  RGB_LED_RED = 0,
  RGB_LED_GREEN,
  RGB_LED_BLUE,
  PWM_LED_1,
  //...
  PWM_MAX_DEVICES
} pwm_device_id_t;

// USART 设备逻辑标识枚举
typedef enum {
  USART_LOGGER = 0,
  USART_ATCMD,
  //...
  USART_MAX_DEVICES
} usart_device_id_t;

// LCD 设备逻辑标识枚举
typedef enum {
  LCD_MAIN = 0,
  //...
  LCD_MAX_DEVICES
} lcd_device_id_t;

// SDRAM 设备逻辑标识枚举
typedef enum {
  SDRAM_MAIN = 0,
  //...
  SDRAM_MAX_DEVICES
} sdram_device_id_t;

// I2C 软件模拟设备逻辑标识枚举
typedef enum {
  I2C_SOFT_TOUCH = 0, // 触摸屏 I2C
  //...
  I2C_SOFT_MAX_DEVICES
} i2c_soft_device_id_t;

// 触摸屏设备逻辑标识枚举
typedef enum {
  TOUCH_MAIN = 0, // 主触摸屏
  //...
  TOUCH_MAX_DEVICES
} touch_device_id_t;

// RTC 设备逻辑标识枚举
typedef enum {
  RTC_DEVICE_0 = 0,
  // ...
  RTC_MAX_DEVICES
} rtc_device_id_t;

// SPI 设备逻辑标识枚举
typedef enum {
  SPI_1 = 0,
  //...
  SPI_MAX_DEVICES
} spi_device_id_t;

// QSPI 设备逻辑标识枚举
typedef enum {
  QSPI_1 = 0,
  //...
  QSPI_MAX_DEVICES
} qspi_device_id_t;

// Flash 设备类型枚举
typedef enum {
  FLASH_TYPE_NONE = 0,
  FLASH_TYPE_SPI,
  FLASH_TYPE_QSPI,
  FLASH_TYPE_NAND  // 新增NAND Flash类型
} flash_type_t;

// Flash 设备逻辑标识枚举
typedef enum {
  FLASH_EXT_SPI = 0, // 外部SPI Flash
  FLASH_EXT_QSPI,    // 外部QSPI Flash
  FLASH_EXT_NAND,    // 外部NAND Flash
  //...
  FLASH_MAX_DEVICES
} flash_device_id_t;

/*********************
 * 驱动和设备映射结构
 *********************/

// GPIO 设备映射结构体
typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
} gpio_mapping_t;

// PWM 设备映射结构体
typedef struct {
  TIM_HandleTypeDef *htim;
  uint32_t channel;
} pwm_mapping_t;

// USART 设备映射结构体
typedef struct {
  UART_HandleTypeDef *huart;
} usart_mapping_t;

// LCD 设备映射结构体
typedef struct {
  LTDC_HandleTypeDef *hltdc;
} lcd_mapping_t;

// SDRAM 设备映射结构体
typedef struct {
  SDRAM_HandleTypeDef *hsdram;
} sdram_mapping_t;

// I2C 软件模拟设备映射结构体
typedef struct {
  GPIO_TypeDef *scl_port; // SCL 引脚端口
  uint16_t scl_pin;       // SCL 引脚
  GPIO_TypeDef *sda_port; // SDA 引脚端口
  uint16_t sda_pin;       // SDA 引脚
  uint32_t delay_us;      // 延时时间（微秒）
} i2c_soft_mapping_t;

// 触摸屏设备映射结构体
typedef struct {
  i2c_soft_device_id_t i2c_id;  // 使用的 I2C 设备 ID
  gpio_device_id_t rst_gpio_id; // RST 引脚 GPIO ID
  gpio_device_id_t int_gpio_id; // INT 引脚 GPIO ID
  uint8_t i2c_addr_mode;        // I2C 地址模式 (0x14 或 0x5D)
} touch_mapping_t;

// RTC 设备映射结构体
typedef struct {
  RTC_HandleTypeDef *hrtc;
} rtc_mapping_t;

// SPI 设备映射结构体
typedef struct {
  SPI_HandleTypeDef *hspi;
  GPIO_TypeDef *cs_port;
  uint16_t cs_pin;
} spi_mapping_t;

// QSPI 设备映射结构体
typedef struct {
  QSPI_HandleTypeDef *hqspi;
} qspi_mapping_t;

// Flash 设备配置结构体 (使用ID引用)
typedef struct {
  flash_type_t type;
  union {
    spi_device_id_t spi_id;
    qspi_device_id_t qspi_id;
    NAND_HandleTypeDef *hnand;
  };
} flash_mapping_t;

// 导出映射表
extern const gpio_mapping_t gpio_mappings[GPIO_MAX_DEVICES];
extern const pwm_mapping_t pwm_mappings[PWM_MAX_DEVICES];
extern const usart_mapping_t usart_mappings[USART_MAX_DEVICES];
extern const lcd_mapping_t lcd_mappings[LCD_MAX_DEVICES];
extern const sdram_mapping_t sdram_mappings[SDRAM_MAX_DEVICES];
extern const i2c_soft_mapping_t i2c_soft_mappings[I2C_SOFT_MAX_DEVICES];
extern const touch_mapping_t touch_mappings[TOUCH_MAX_DEVICES];
extern const rtc_mapping_t rtc_mappings[RTC_MAX_DEVICES];
extern const spi_mapping_t spi_mappings[SPI_MAX_DEVICES];
extern const qspi_mapping_t qspi_mappings[QSPI_MAX_DEVICES];
extern const flash_mapping_t flash_mappings[FLASH_MAX_DEVICES];

#endif /* BSP_STM32H743_BOARD_V1_DEVICE_MAPPING_H_ */
