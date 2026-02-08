#include "Sys.h"
#include "test_config.h"
#include "test_framework.h"
#include <string.h>

#if ENABLE_TEST_SDRAM

#define TEST_SDRAM_SIZE (1024 * 1024) // 1MB test size
#define TEST_NUMBER 27
static uint8_t current;
static uint32_t *pExternal[TEST_NUMBER];

static void test_sdram_setup(void) {
  current = 0;
  pExternal[0] = (uint32_t *)sys_malloc(SYS_MEM_EXTERNAL, TEST_SDRAM_SIZE);
  if (pExternal[0] == NULL) {
    log_e("SDRAM Malloc Failed! Size: %d", TEST_SDRAM_SIZE);
    return;
  }
  log_i("SDRAM Test Setup: Ready to verify external memory.");
}

static void test_sdram_run(void) {
  uint32_t write_time, read_time;
  log_i("");
  log_i("Starting SDRAM Data Integrity Test... Test number: %d", current);
  if (pExternal[current] == NULL) {
    return;
  }
  log_i("Allocated 1MB on SDRAM at: 0x%08X", (uint32_t)pExternal[current]);

  // 1. Write Pattern
  log_i("Writing pattern...");
  write_time = sys_get_systick_ms();
  for (uint32_t i = 0; i < TEST_SDRAM_SIZE / 4; i++) {
    pExternal[current][i] = i ^ 0xAAAAAAAA;
  }
  write_time = sys_get_systick_ms() - write_time;
  log_i("Write time: %d ms", write_time);

  // 2. Verify Pattern
  log_i("Verifying pattern...");
  int error_count = 0;
  read_time = sys_get_systick_ms();
  for (uint32_t i = 0; i < TEST_SDRAM_SIZE / 4; i++) {
    if (pExternal[current][i] != (i ^ 0xAAAAAAAA)) {
      if (error_count < 10) { // 防止出现太多错误时日志超载
        log_e("Data Error at index %d: Expected 0x%08X, got 0x%08X", i,
              (i ^ 0xAAAAAAAA), pExternal[current][i]);
      }
      error_count++;
    }
  }
  read_time = sys_get_systick_ms() - read_time;
  log_i("Read time: %d ms", read_time);

  if (error_count == 0) {
    log_i("SDRAM Data Integrity Test PASSED.");
  } else {
    log_e("SDRAM Data Integrity Test FAILED with %d errors.", error_count);
  }

  current++;
  if (current >= TEST_NUMBER) {
    // 3. Free Memory
    for (int i = 0; i < TEST_NUMBER; i++) {
      sys_free(SYS_MEM_EXTERNAL, pExternal[i]);
      pExternal[i] = NULL;
    }
    log_i("SDRAM Malloc/Free Test PASSED.");
    current = 0;
  }

  // 重新申请(还未释放)
  pExternal[current] =
      (uint32_t *)sys_malloc(SYS_MEM_EXTERNAL, TEST_SDRAM_SIZE);
  if (pExternal[current] == NULL) {
    log_e("SDRAM Malloc Failed! Size: %d", TEST_SDRAM_SIZE);
    return;
  }

  sys_delay_ms(200);
}

static void test_sdram_teardown(void) {
  for (int i = 0; i < TEST_NUMBER; i++) {
	if(pExternal[i] != NULL){
		sys_free(SYS_MEM_EXTERNAL, pExternal[i]);
		pExternal[i] = NULL;
	}
  }
  log_i("SDRAM Malloc/Free Test PASSED.");
  log_i("SDRAM Test Teardown: Completed.");
}

REGISTER_TEST(SDRAM, "Verify SDRAM data integrity and memory pool allocation",
              test_sdram_setup, test_sdram_run, test_sdram_teardown);

#endif /* ENABLE_TEST_SDRAM */
