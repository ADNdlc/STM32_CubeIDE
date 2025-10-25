.
├── Application/                    # 应用程序代码
│   ├── RAMspeed_test/             # RAM速度测试相关代码
│   │   ├── speed_test.c
│   │   └── speed_test.h
│   └── SDRAM_test/                # SDRAM测试相关代码
│       ├── sdram_test.c
│       └── sdram_test.h
├── Core/                          
│   ├── Inc/                       
│   │   ├── dma.h                  
│   │   ├── fmc.h                  
│   │   ├── gpio.h                 
│   │   ├── main.h                 
│   │   ├── rtc.h                  
│   │   ├── stm32h7xx_hal_conf.h   
│   │   ├── stm32h7xx_it.h         
│   │   ├── tim.h                 
│   │   └── usart.h                
│   └── Src/                      
│       ├── dma.c              
│       ├── fmc.c              
│       ├── gpio.c 
│       ├── main.c
│       ├── rtc.c
│       ├── stm32h7xx_hal_msp.c
│       ├── stm32h7xx_it.c 
│       ├── syscalls.c 
│       ├── sysmem.c
│       ├── system_stm32h7xx.c
│       ├── tim.c
│       └── usart.c
├── Drivers/                       # 驱动程序
│   ├── BSP/
│   │   ├── LED/                   # LED驱动
│   │   │   ├── LED.h
│   │   │   └── LED_control.c
│   │   ├── dht11/                 # DHT11温湿度传感器驱动
│   │   │   ├── dht11.c
│   │   │   ├── dht11.h
│   │   │   ├── dht11_MQTT.c
│   │   │   └── dht11_MQTT.h
│   │   ├── esp_at/                # ESP-AT通信模块
│   │   │   ├── Sensor_Data.c
│   │   │   ├── Sensor_Data.h
│   │   │   ├── at_controller.c
│   │   │   ├── at_controller.h
│   │   │   ├── at_dispatcher.c
│   │   │   ├── at_dispatcher.h
│   │   │   ├── at_parser.c
│   │   │   ├── at_parser.h
│   │   │   ├── at_uart.c
│   │   │   ├── at_uart.h
│   │   │   └── esp_app/
│   │   ├── key/                   # 按键驱动
│   │   │   ├── Button_event.c
│   │   │   ├── Button_event.h
│   │   │   ├── Key.c
│   │   │   └── Key.h
│   │   ├── malloc/                # 内存分配
│   │   │   ├── malloc.c
│   │   │   └── malloc.h
│   │   └── sdram/                 # SDRAM驱动
│   │       ├── W9825G6KH.c
│   │       └── W9825G6KH.h
│   ├── CMSIS/
│   │   ├── Device\ST\STM32H7xx
│   │   ├── Include/
│   │   └── LICENSE.txt
│   ├── STM32H7xx_HAL_Driver/
│   │   ├── Inc/
│   │   ├── Src/
│   │   └── LICENSE.txt
│   └── SYSTEM/                    # 系统组件
│       ├── delay/
│       ├── sys/
│       ├── usart/
│       └── readme.txt
├── Debug/
└── cmake-build-debug/

以下是图形界面的结构
