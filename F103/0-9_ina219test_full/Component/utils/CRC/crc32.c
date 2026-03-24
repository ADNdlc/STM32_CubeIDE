/*
 * crc32.c
 *
 *  Created on: Feb 19, 2026
 *      Author: Antigravity
 */

#include "crc32.h"
#include <stdbool.h>

static uint32_t crc32_table[256];
static bool is_initialized = false;

void utils_crc32_init(void) {
  if (is_initialized)
    return;

  uint32_t polynomial = 0xEDB88320;
  for (uint32_t i = 0; i < 256; i++) {
    uint32_t crc = i;
    for (uint32_t j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ polynomial;
      } else {
        crc >>= 1;
      }
    }
    crc32_table[i] = crc;
  }
  is_initialized = true;
}

uint32_t utils_crc32_calc(const uint8_t *data, size_t length,
                          uint32_t previous_crc) {
  if (!is_initialized) {
    utils_crc32_init();
  }

  uint32_t crc = previous_crc ^ 0xFFFFFFFF;
  for (size_t i = 0; i < length; i++) {
    crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
  }
  return crc ^ 0xFFFFFFFF;
}
