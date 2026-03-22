#include "sntp_service/sntp_service.h"
#include "rtc_driver.h"
#include "rtc_factory.h"

#define LOG_TAG "SNTP_SVC"
#include "elog.h"

static void sntp_sync_callback(void *arg, int result, sntp_time_t *time) {
  sntp_service_t *self = (sntp_service_t *)arg;
  if (result == 0) {
    log_i("Time synced: 20%02d-%02d-%02d %02d:%02d:%02d", time->date.year,
          time->date.month, time->date.day, time->time.hour, time->time.minute,
          time->time.second);
    // 更新本地 RTC
    rtc_driver_t *rtc = rtc_driver_get(RTC_ID_INTERNAL);
    if (rtc) {
      RTC_SET_TIME(rtc, &time->time);
      RTC_SET_DATE(rtc, &time->date);
      log_d("RTC updated.");
    }
    self->state = SNTP_SVC_STATE_SYNCED;
  } else {
    log_e("Sync failed: %d", result);
    self->state = SNTP_SVC_STATE_ERROR;
  }
}

void sntp_svc_init(sntp_service_t *self, sntp_driver_t *drv) {
  self->drv = drv;
  self->state = SNTP_SVC_STATE_IDLE;
  self->last_sync_time = 0;
}

void sntp_svc_set_config(sntp_service_t *self, int timezone,
                         char *domain_name) {
  self->timezone = timezone;
  self->domain_name = domain_name;
  self->state = SNTP_SVC_STATE_SYNCING;
  SNTP_DRV_CONFIG(self->drv, self->timezone, self->domain_name);
}

int sntp_svc_start_sync(sntp_service_t *self) {
  if (SNTP_DRV_START_SYNC(self->drv, sntp_sync_callback, self) != 0) {
    log_e("driver not ready");
    return -1;
  }
  self->state = SNTP_SVC_STATE_FETCHING;
  return 0;
}

void sntp_svc_process(sntp_service_t *self) {
  if (self->state == SNTP_SVC_STATE_SYNCING) {
    // If the driver has successfully synced time behind the scenes, fetch it
    if (SNTP_DRV_GET_STATUS(self->drv) == SNTP_DRV_STATUS_SUCCESS) {
      sntp_svc_start_sync(self);
    }
  }
}

sntp_svc_state_t sntp_svc_get_state(sntp_service_t *self) {
  return self->state;
}
