#include "storage_interface.h"
#include "nor_flash_driver.h"
#include "elog.h"
#include <string.h>

#define LOG_TAG "NOR_STORAGE"

typedef struct {
    storage_device_t base;
    nor_flash_driver_t *drv;
} nor_flash_storage_t;

static int nor_storage_init(storage_device_t *self) {
    nor_flash_storage_t *st = (nor_flash_storage_t *)self;
    if (!st->drv) return -1;
    
    // NOR Flash 驱动通常在外部已经初始化
    nor_flash_info_t info;
    if (st->drv->ops->get_info(st->drv, &info) == 0) {
        self->info.type = STORAGE_TYPE_NOR_FLASH;
        self->info.total_size = info.total_size;
        self->info.erase_size = info.sector_size;
        self->info.write_size = info.page_size;
        self->info.read_size = 1;
        memcpy(self->info.device_id, info.device_id, 3);
        return 0;
    }
    return -2;
}

static storage_status_t nor_storage_get_status(storage_device_t *self) {
    // NOR Flash 只要能读出 ID 通常就认为 OK (其不带 CD 引脚)
    return STORAGE_STATUS_OK;
}

static int nor_storage_read(storage_device_t *self, uint32_t addr, uint8_t *buf, size_t len) {
    nor_flash_storage_t *st = (nor_flash_storage_t *)self;
    return st->drv->ops->read(st->drv, addr, buf, len);
}

static int nor_storage_write(storage_device_t *self, uint32_t addr, const uint8_t *data, size_t len) {
    nor_flash_storage_t *st = (nor_flash_storage_t *)self;
    return st->drv->ops->write(st->drv, addr, data, len);
}

static int nor_storage_erase(storage_device_t *self, uint32_t addr, size_t len) {
    nor_flash_storage_t *st = (nor_flash_storage_t *)self;
    uint32_t sector_size = self->info.erase_size;
    
    // 确保按扇区对齐
    if (addr % sector_size != 0 || len % sector_size != 0) return -1;
    
    uint32_t num_sectors = len / sector_size;
    for (uint32_t i = 0; i < num_sectors; i++) {
        if (st->drv->ops->erase_sector(st->drv, addr + i * sector_size) != 0) return -2;
    }
    return 0;
}

static int nor_storage_control(storage_device_t *self, int cmd, void *arg) {
    nor_flash_storage_t *st = (nor_flash_storage_t *)self;
    // 映射到驱动的 set_mode 等
    return 0;
}

static int nor_storage_get_info(storage_device_t *self, storage_info_t *info) {
    if (!info) return -1;
    *info = self->info;
    return 0;
}

static const storage_ops_t s_nor_storage_ops = {
    .init = nor_storage_init,
    .deinit = NULL,
    .get_status = nor_storage_get_status,
    .read = nor_storage_read,
    .write = nor_storage_write,
    .erase = nor_storage_erase,
    .control = nor_storage_control,
    .get_info = nor_storage_get_info
};

storage_device_t* nor_flash_storage_create(nor_flash_driver_t *drv) {
    static nor_flash_storage_t s_instance;
    s_instance.base.ops = &s_nor_storage_ops;
    s_instance.drv = drv;
    return &s_instance.base;
}
