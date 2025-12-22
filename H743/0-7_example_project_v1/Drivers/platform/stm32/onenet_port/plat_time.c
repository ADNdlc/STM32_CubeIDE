#include "plat_time.h"
#include "SYSTEM/sys.h"

uint64_t time_count_ms(void) { return sys_get_systick_ms(); }

uint64_t time_count(void) {
  // 暂时没有 RTC 秒级计数的可以直接转毫秒
  return sys_get_systick_ms() / 1000;
}

void time_delay_ms(uint32_t ms) { sys_delay_ms(ms); }

void time_delay(uint32_t sec) { sys_delay_ms(sec * 1000); }

// 模拟 countdown
struct countdown_tmr_t {
  uint64_t end_time;
};

handle_t countdown_start(uint32_t ms) {
  struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)sys_malloc(
      SYS_MEM_INTERNAL, sizeof(struct countdown_tmr_t));
  if (tmr) {
    tmr->end_time = sys_get_systick_ms() + ms;
  }
  return (handle_t)tmr;
}

void countdown_set(handle_t handle, uint32_t ms) {
  struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;
  if (tmr) {
    tmr->end_time = sys_get_systick_ms() + ms;
  }
}

uint32_t countdown_left(handle_t handle) {
  struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;
  if (!tmr)
    return 0;
  uint64_t now = sys_get_systick_ms();
  if (now >= tmr->end_time)
    return 0;
  return (uint32_t)(tmr->end_time - now);
}

uint32_t countdown_is_expired(handle_t handle) {
  return countdown_left(handle) == 0;
}

void countdown_stop(handle_t handle) {
  if (handle) {
    sys_free(SYS_MEM_INTERNAL, handle);
  }
}

// 暂时不实现获取日期，OneNET 基础功能不需要
uint64_t time_get_date(int *year, int *month, int *day, int *hour, int *min,
                       int *sec, int *ms) {
  return 0;
}
