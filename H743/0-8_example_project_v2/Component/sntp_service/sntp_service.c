#include "sntp_service/sntp_service.h"
#include "rtc_driver.h"
#include "rtc_factory.h"

#define LOG_TAG "SNTP_SVC"
#include "elog.h"

static void sntp_sync_callback(void *arg, int result, sntp_time_t *time) {
    sntp_service_t *self = (sntp_service_t *)arg;
    if (result == 0) {
        log_i("Time synced: 20%02d-%02d-%02d %02d:%02d:%02d",
              time->date.year, time->date.month, time->date.day,
              time->time.hour, time->time.minute, time->time.second);
        
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
    self->network_ready = false;
    self->last_sync_time = 0;
    self->sync_interval_ms = 0;
}

void sntp_svc_set_network_ready(sntp_service_t *self, bool ready) {
    self->network_ready = ready;
    if (ready && self->state == SNTP_SVC_STATE_ERROR) {
        self->state = SNTP_SVC_STATE_IDLE;
    }
}

void sntp_svc_start_sync(sntp_service_t *self, int timezone, uint32_t interval_ms) {
    if (!self->drv || !self->network_ready) {
        log_w("Sync failed: driver missing or network not ready");
        return;
    }

    self->sync_interval_ms = interval_ms;
    self->state = SNTP_SVC_STATE_SYNCING;
    SNTP_DRV_CONFIG(self->drv, timezone, "ntp.aliyun.com");
    SNTP_DRV_START_SYNC(self->drv, sntp_sync_callback, self);
}

void sntp_svc_process(sntp_service_t *self) {
    // 简单的自动重连或定时同步逻辑
    // ...
}

sntp_svc_state_t sntp_svc_get_state(sntp_service_t *self) {
    return self->state;
}
