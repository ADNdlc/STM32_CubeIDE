#include "raw_strategy.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

#define RAW_STRATEGY_MEMSOURCE SYS_MEM_INTERNAL

typedef struct {
  flash_strategy_t parent;
  // Fine-grained mutex here if needed
} raw_strategy_impl_t;

static int _raw_mount(flash_strategy_t *self, block_device_t *dev) {
  self->dev = dev;
  return BLOCK_DEV_INIT(dev);
}

static int _raw_unmount(flash_strategy_t *self) {
  if (self->dev) {
    BLOCK_DEV_DEINIT(self->dev);
    self->dev = NULL;
  }
  return 0;
}

// Raw strategy treats path as offset if possible, or ignores path
// For simplicity, let's assume usage is generic read/write and path is ignored
// or parsed as offset But user requirement says:
// "根据路径的前缀，决定调用哪个文件系统实例". So Handler decides strategy.
// Strategy handles the rest. For Raw, maybe we just use the offset passed in
// arg.

static int _raw_read(flash_strategy_t *self, const char *path, uint32_t offset,
                     uint8_t *buf, size_t size) {
  // Lock fine-grained mutex
  if (!self->dev)
    return -1;
  // Interpret offset as absolute address
  return BLOCK_DEV_READ(self->dev, offset, buf, size);
  // Unlock
}

static int _raw_write(flash_strategy_t *self, const char *path, uint32_t offset,
                      const uint8_t *buf, size_t size) {
  // Lock fine-grained mutex
  if (!self->dev)
    return -1;
  return BLOCK_DEV_PROGRAM(self->dev, offset, buf, size);
  // Unlock
}

static const flash_strategy_ops_t raw_ops = {
    .mount = _raw_mount,
    .unmount = _raw_unmount,
    .read = _raw_read,
    .write = _raw_write,
};

flash_strategy_t *raw_strategy_create(void) {
  raw_strategy_impl_t *impl = (raw_strategy_impl_t *)sys_malloc(
      RAW_STRATEGY_MEMSOURCE, sizeof(raw_strategy_impl_t));
  if (impl) {
    impl->parent.ops = &raw_ops;
    impl->parent.dev = NULL;
    return &impl->parent;
  }
  return NULL;
}

void raw_strategy_destroy(flash_strategy_t *strategy) {
  if (strategy) {
    sys_free(RAW_STRATEGY_MEMSOURCE, strategy);
  }
}
