/*
 * elog_test.c
 *
 *  Created on: Dec 7, 2025
 *      Author: 12114
 */

#include "elog_test.h"
#include "all_tests_config.h"
#include "main.h"

#if _elog_test_

#include "elog.h"
#include <stdio.h>

// Helper to add some delay
static void delay_ms(volatile uint32_t ms) { HAL_Delay(ms); }

void elog_test_run(void) {
  // 1. Initialize EasyLogger
  /* initialize EasyLogger */
  if (elog_init() == ELOG_NO_ERR) {
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);

    // Start EasyLogger
    elog_start();

    // Allow some time for init message to go out
    delay_ms(100);

    log_i("EasyLogger Init Success!");
  } else {
    // Fallback if init fails
    return;
  }

  // 2. Test Basic Logging Levels
  log_a("This is an ASSERT message.");
  log_e("This is an ERROR message.");
  log_w("This is a WARNING message.");
  log_i("This is an INFO message.");
  log_d("This is a DEBUG message.");
  log_v("This is a VERBOSE message.");

  delay_ms(500);

  // 3. Test Raw Output
  elog_raw("This is a raw log output without any formatting.\r\n");
  elog_raw("Another raw line.\r\n");

  delay_ms(500);

  // 4. Test Dynamic Filter Changes
  // Only show warnings and above
  log_i("Setting filter to WARNING level. Info below should NOT appear.");
  elog_set_filter_lvl(ELOG_LVL_WARN);
  log_i("You should NOT see this info message.");
  log_d("You should NOT see this debug message.");
  log_w("You SHOULD see this warning message.");
  log_e("You SHOULD see this error message.");

  // Restore filter
  elog_set_filter_lvl(ELOG_LVL_VERBOSE);
  log_i("Filter restored to VERBOSE. You should see this.");

  delay_ms(500);

  // 5. Test Tag Filtering (Optional, if enabled in config)
#if ELOG_FILTER_TAG_ENABLE
  log_i("Testing Tag Filtering...");
  elog_set_filter_tag("TEST_TAG");
  log_i("This message has default tag (usually 'NO_TAG' or file-defined), "
        "might be hidden if filter is strict.");

  // We can't easily change the tag of the current file macro dynamically
  // without re-definition, but we can verify if the system is alive. Reset tag
  // filter
  elog_set_filter_tag("");
#endif

  log_i("EasyLogger Test Complete.");
}

#endif
