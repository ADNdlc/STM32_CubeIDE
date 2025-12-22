#include "plat_osl.h"
#include "SYSTEM/sys.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void *osl_malloc(size_t size) {
  return sys_malloc(SYS_MEM_INTERNAL, (uint32_t)size);
}

void *osl_calloc(size_t num, size_t size) {
  void *ptr = sys_malloc(SYS_MEM_INTERNAL, (uint32_t)(num * size));
  if (ptr)
    memset(ptr, 0, num * size);
  return ptr;
}

void osl_free(void *ptr) { sys_free(SYS_MEM_INTERNAL, ptr); }

void *osl_memcpy(void *dst, const void *src, size_t n) {
  return memcpy(dst, src, n);
}

void *osl_memmove(void *dst, const void *src, size_t n) {
  return memmove(dst, src, n);
}

void *osl_memset(void *dst, int32_t val, size_t n) {
  return memset(dst, val, n);
}

uint8_t *osl_strdup(const uint8_t *s) {
  size_t len = strlen((const char *)s);
  uint8_t *d = (uint8_t *)osl_malloc(len + 1);
  if (d)
    strcpy((char *)d, (const char *)s);
  return d;
}

uint8_t *osl_strndup(const uint8_t *s, size_t n) {
  size_t len = strlen((const char *)s);
  if (len > n)
    len = n;
  uint8_t *d = (uint8_t *)osl_malloc(len + 1);
  if (d) {
    memcpy(d, s, len);
    d[len] = '\0';
  }
  return d;
}

uint8_t *osl_strcpy(uint8_t *s1, const uint8_t *s2) {
  return (uint8_t *)strcpy((char *)s1, (const char *)s2);
}

uint8_t *osl_strncpy(uint8_t *s1, const uint8_t *s2, size_t n) {
  return (uint8_t *)strncpy((char *)s1, (const char *)s2, n);
}

uint8_t *osl_strcat(uint8_t *dst, const uint8_t *src) {
  return (uint8_t *)strcat((char *)dst, (const char *)src);
}

uint8_t *osl_strstr(const uint8_t *s1, const uint8_t *s2) {
  return (uint8_t *)strstr((const char *)s1, (const char *)s2);
}

uint32_t osl_strlen(const uint8_t *s) {
  return (uint32_t)strlen((const char *)s);
}

int32_t osl_strcmp(const uint8_t *s1, const uint8_t *s2) {
  return strcmp((const char *)s1, (const char *)s2);
}

int32_t osl_strncmp(const uint8_t *s1, const uint8_t *s2, size_t n) {
  return strncmp((const char *)s1, (const char *)s2, n);
}

int32_t osl_sprintf(uint8_t *str, const uint8_t *format, ...) {
  va_list args;
  va_start(args, format);
  int32_t res = vsprintf((char *)str, (const char *)format, args);
  va_end(args);
  return res;
}

int32_t osl_sscanf(const uint8_t *str, const uint8_t *format, ...) {
  va_list args;
  va_start(args, format);
  int32_t res = vsscanf((const char *)str, (const char *)format, args);
  va_end(args);
  return res;
}

void osl_assert(boolean expression) {
  // 简略实现
}

int32_t osl_get_random(unsigned char *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    buf[i] = (unsigned char)rand();
  }
  return 0;
}

int32_t osl_atoi(const uint8_t *nptr) { return atoi((const char *)nptr); }

int32_t osl_rand(int32_t min, int32_t max) {
  return (rand() % (max - min + 1)) + min;
}

uint8_t *osl_random_string(uint8_t *buf, int len) {
  const char char_set[] =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for (int i = 0; i < len; i++) {
    buf[i] = char_set[rand() % (sizeof(char_set) - 1)];
  }
  buf[len] = '\0';
  return buf;
}

int32_t module_init(void *arg, void *callback) { return 0; }
int32_t module_deinit(void) { return 0; }
