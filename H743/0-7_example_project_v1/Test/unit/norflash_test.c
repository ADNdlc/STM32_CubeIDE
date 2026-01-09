#include "norflash_test.h"
#include "elog.h"
#include "old_driver/norflash/norflash.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "NORFlash Test"

void norflash_test(void)
{
    log_d("Starting NOR Flash test");
    
    // 1. 初始化NOR Flash
    log_d("Initializing NOR Flash");
    norflash_init();
    uint16_t flash_id = norflash_read_id();
    log_i("NOR Flash ID: 0x%04X", flash_id);
    
    if (flash_id == 0 || flash_id == 0xFFFF) {
        log_e("Failed to read valid Flash ID, got: 0x%04X", flash_id);
        return;
    }
    
    // 2. 准备测试数据
    uint8_t write_buf[256];
    uint8_t read_buf[256];
    uint32_t test_addr = 0;
    
    // 填充测试数据
    for (int i = 0; i < 256; i++) {
        write_buf[i] = i % 256;
    }
    
    log_d("Starting read-write test at address 0x%08X", test_addr);
    
    // 3. 先擦除扇区 (一个扇区是4KB)
    log_d("Erasing sector at address 0x%08X", test_addr);
    norflash_erase_sector(test_addr);
    
    // 4. 读取擦除后的数据（应该全是0xFF）
    log_d("Reading data after sector erase");
    norflash_read(read_buf, test_addr, 256);
    
    // 验证擦除是否成功
    int erased_correctly = 1;
    for (int i = 0; i < 256; i++) {
        if (read_buf[i] != 0xFF) {
            erased_correctly = 0;
            break;
        }
    }
    
    if (erased_correctly) {
        log_i("Sector erase successful - all bytes are 0xFF");
    } else {
        log_w("Sector erase may not be complete - some bytes are not 0xFF");
    }
    
    // 5. 写入测试数据
    log_d("Writing test data to address 0x%08X", test_addr);
    norflash_write(write_buf, test_addr, 256);
    
    // 6. 读取写入的数据
    log_d("Reading back written data");
    norflash_read(read_buf, test_addr, 256);
    
    // 7. 验证数据是否一致
    int data_match = 1;
    for (int i = 0; i < 256; i++) {
        if (write_buf[i] != read_buf[i]) {
            log_w("Data mismatch at index %d: expected 0x%02X, got 0x%02X", 
                  i, write_buf[i], read_buf[i]);
            data_match = 0;
            break;
        }
    }
    
    if (data_match) {
        log_i("Data write-read test passed - all bytes match");
    } else {
        log_e("Data write-read test failed");
        return;
    }
    
    // 8. 进行多组数据测试 - 在不同地址写入不同数据
    log_d("Starting multi-address test");
    
    for (int addr_idx = 1; addr_idx <= 3; addr_idx++) {
        uint32_t addr = addr_idx * 0x1000; // 每次跳一个扇区（4KB）
        
        // 准备不同的测试数据
        for (int i = 0; i < 64; i++) {
            write_buf[i] = (addr_idx * 100 + i) % 256;
        }
        
        log_d("Testing at address 0x%08X", addr);
        
        // 擦除扇区
        norflash_erase_sector(addr);
        
        // 写入数据
        norflash_write(write_buf, addr, 64);
        
        // 读取数据
        memset(read_buf, 0, 64);
        norflash_read(read_buf, addr, 64);
        
        // 验证数据
        int addr_data_match = 1;
        for (int i = 0; i < 64; i++) {
            if (write_buf[i] != read_buf[i]) {
                log_w("Data mismatch at address 0x%08X, index %d: expected 0x%02X, got 0x%02X", 
                      addr, i, write_buf[i], read_buf[i]);
                addr_data_match = 0;
                break;
            }
        }
        
        if (addr_data_match) {
            log_i("Address 0x%08X test passed", addr);
        } else {
            log_e("Address 0x%08X test failed", addr);
        }
    }
    
    log_i("NOR Flash test completed successfully");
}
