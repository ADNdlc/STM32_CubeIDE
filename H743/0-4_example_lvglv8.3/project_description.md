# Project Description: STM32H7 LVGL Example

## 1. Project Overview
This project is an embedded application based on the **STM32H743IITx** microcontroller, featuring a rich graphical user interface powered by **LVGL 8.3**. It serves as a comprehensive example and testbed for various hardware peripherals and software components, including WiFi connectivity, sensors, external memory, and file systems.

## 2. Hardware Configuration
- **Microcontroller**: STM32H743IITx (ARM Cortex-M7)
- **External Memory**:
    - **SDRAM**: W9825G6KH (32MB) for display buffer and heap.
    - **SD Card**: SDMMC interface for file storage.
- **Display & Input**:
    - **LCD**: RGB Interface LCD.
    - **Touch**: GT9147 Capacitive Touch Controller.
- **Connectivity**:
    - **WiFi**: ESP8266/ESP32 via AT Commands (UART).
- **Sensors**:
    - **DHT11**: Temperature and Humidity Sensor.
- **User Interface**:
    - **LEDs**: Status indicators.
    - **Buttons**: User input.

## 3. Software Architecture

### 3.1. Directory Structure
- **Core**: Contains the main application entry point (`main.c`), interrupt handlers (`stm32h7xx_it.c`), and system configuration.
- **Drivers**:
    - **BSP (Board Support Package)**: Custom drivers for board-specific hardware:
        - `esp_at`: ESP-AT command parser and controller.
        - `dht11`: Sensor driver.
        - `touch_GT9147`: Touch screen driver.
        - `rgbLCD`: LCD driver.
        - `sdram`: External memory controller configuration.
        - `key`, `led`: Basic IO control.
        - `malloc`: Custom memory management.
    - **STM32H7xx_HAL_Driver**: ST's Hardware Abstraction Layer.
    - **CMSIS**: Cortex-M standard interface.
- **Middlewares**:
    - **FatFs**: Generic FAT file system module.
- **LVGL**:
    - **lvgl**: The core LVGL 8.3 library source code.
    - **LVGL_APP**: Application-specific UI code, including `ui/Act_Manager.h` for screen management.
- **Application**:
    - Contains standalone test modules for verifying hardware:
        - `SDRAM_test`, `RAMspeed_test`
        - `SDCARD_test`, `FATFS_test`
        - `LVGL_test`

### 3.2. Key Features & Modules
- **Graphical User Interface**:
    - Uses LVGL 8.3.
    - Includes benchmark demos and a custom UI manager (`Act_Manager`).
    - Display driver implemented in `lv_port_disp.c`.
    - Touch input driver implemented in `lv_port_indev.c`.
- **Networking**:
    - WiFi connectivity via ESP-AT commands over UART.
    - MQTT support (implied by `dht11_MQTT.h`).
- **System**:
    - **FreeRTOS**: Not currently active in the main loop (Bare metal `while(1)` loop with `lv_timer_handler`), though `Middlewares` might support it.
    - **Memory Management**: Custom `my_mem_init` for managing multiple RAM regions (DTCM, ITCM, SDRAM, etc.).

## 4. Build & Toolchain
- **IDE**: STM32CubeIDE.
- **Configuration**: `.ioc` file available for CubeMX regeneration.
- **Build System**: Eclipse-based managed build (CDT) and CMake support (`CMakeLists.txt` present).

## 5. Entry Point Analysis (`main.c`)
The `main()` function performs the following initialization sequence:
1.  **HAL Init**: `HAL_Init()`.
2.  **System Clock**: `SystemClock_Config()` (PLL setup).
3.  **Peripherals**: GPIO, DMA, UART, FMC (SDRAM), LTDC, SDMMC, RTC, TIM.
4.  **Board Hardware**:
    - `Button_Init`, `SDRAM_InitSequence`.
    - `my_mem_init` for all memory regions.
5.  **Tests (Optional)**: Conditional execution of SDRAM, FatFS tests based on macros.
6.  **LVGL Init**: `lv_init()`, `lv_port_disp_init()`, `lv_port_indev_init()`.
7.  **App Init**: `act_manager_init()` (UI), `ESP_AT_sys_init()`.
8.  **Main Loop**:
    - `lv_timer_handler()`: Core LVGL task.
    - `ESP_AT_sys_handle()`: WiFi task.
    - `Button_UPDATE()`: Input polling.
    - `DHT11_ReadData()`: Sensor polling.
