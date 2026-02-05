#include "elog.h"
#include "flash_handler.h"
#include <string.h>


#define LOG_TAG "FLASH_FILE_TEST"

void test_flash_file_ops(const char *test_path) {
  log_i("Starting flash file operations test on: %s", test_path);

  char write_data[] = "Hello Antigravity Flash System!";
  char read_buf[64] = {0};

  // 1. Open and Write
  flash_file_t *f = flash_fopen(test_path, "w+");
  if (!f) {
    log_e("Failed to open file for writing: %s", test_path);
    return;
  }

  size_t written = flash_fwrite(write_data, 1, strlen(write_data), f);
  log_i("Wrote %d bytes", written);
  flash_fclose(f);

  // 2. Open and Read
  f = flash_fopen(test_path, "r");
  if (!f) {
    log_e("Failed to open file for reading: %s", test_path);
    return;
  }

  size_t read = flash_fread(read_buf, 1, sizeof(read_buf), f);
  log_i("Read %d bytes: %s", read, read_buf);

  if (strcmp(write_data, read_buf) == 0) {
    log_i("Data integrity check PASSED");
  } else {
    log_e("Data integrity check FAILED!");
  }

  // 3. Seek and Tell
  flash_fseek(f, 6, FLASH_SEEK_SET);
  long pos = flash_ftell(f);
  log_i("Position after seek: %ld", pos);

  memset(read_buf, 0, sizeof(read_buf));
  flash_fread(read_buf, 1, 11, f);
  log_i("Read from offset 6: %s", read_buf);

  flash_fclose(f);

  // 4. Stat
  flash_stat_t st;
  if (flash_stat(test_path, &st) == 0) {
    log_i("File size by stat: %d", st.size);
  }

  // 5. Unlink
  // flash_unlink(test_path);
  // log_i("File unlinked");
}
