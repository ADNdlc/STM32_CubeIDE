/*
 * RTC_cal.c
 *
 *  Created on: Aug 14, 2025
 *      Author: 12114
 */

#include "../esp_wifi/esp_wifi.h"
#include "../../at_controller.h"
#include "RTC_cal.h"
#include "rtc.h"	// 此模块用于校准RTC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// =========== 私有状态 ===========
static bool g_NTP_ready = 	false; 	// ESP模块的NTP时间是否就绪
static bool g_RTC_ready =   false; 	// STM32的RTC是否已校准

// --- 私有命令对象和缓冲区
static char sntp_cfg_buf[128];
static AT_Cmd_t cmd_sntp_cfg;

static char parsed_time_string[64];//保存查询到的时间
static AT_Cmd_t cmd_sntp_sync;


/* ================== 内部辅助函数 ================== */
// 将月份字符串 ( "Jan") 转换为数字 (1-12)
static int _month_str_to_int(const char* month) {
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; ++i) {
        if (strncasecmp(month, months[i], 3) == 0) {
            return i + 1;
        }
    }
    return 0;
}


/* ================== AT命令回调函数 ================== */
// 配置命令的响应回调
static void _setcfg_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK) {
        printf("SNTP configured successfully. Waiting for time update...\r\n");
    } else {
        printf("Error: SNTP configuration failed.\r\n");
    }
}
// 时间查询步命令的数据解析回调
static void _sntp_time_parser(const char* data_line) {
    // data_line 格式: "+CIPSNTPTIME:Fri May 30 18:06:18 2025"
    // 我们需要将时间字符串部分拷贝出来
    const char* time_start = strchr(data_line, ':');
    if (time_start && strlen(time_start) > 2) {
        strncpy(parsed_time_string, time_start + 1, sizeof(parsed_time_string) - 1);
        parsed_time_string[sizeof(parsed_time_string) - 1] = '\0';
    }
}
// 时间查询命令的最终响应回调(解析数据校准RTC)
static void _sntp_sync_rsp_cb(AT_CmdResult_t result, const char* line) {
    if (result == AT_CMD_OK && strlen(parsed_time_string) > 0) {
        // 命令成功，并且我们已经解析到了时间字符串，现在设置STM32 RTC
        char day_of_week_str[4], month_str[4];
        int day, year, hour, min, sec;

        // 解析: " Fri May 30 18:06:18 2025"
        // 注意sscanf前面的空格可以匹配掉字符串前面的空格
        if (sscanf(parsed_time_string, " %s %s %d %d:%d:%d %d",
                   day_of_week_str, month_str, &day, &hour, &min, &sec, &year) == 7)
        {
        	if(year > 2000){ //确保模块返回的是校准后的时间
				RTC_TimeTypeDef sTime = {0};
				RTC_DateTypeDef sDate = {0};

				sTime.Hours = hour;
				sTime.Minutes = min;
				sTime.Seconds = sec;

				sDate.WeekDay = 0; // HAL库可以自动计算，或根据day_of_week_str设置
				sDate.Month = _month_str_to_int(month_str);
				sDate.Date = day;
				sDate.Year = year - 2000; // HAL库需要 0-99 的年份

				// 使用二进制格式设置，避免BCD转换的麻烦
				if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK &&
					HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) == HAL_OK)
				{
					g_RTC_ready = true;
					printf("RTC calibrated to: %s\r\n", parsed_time_string);

					// (可选) 写入一个备份寄存器，表明RTC已校准
					// HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0xCAFE);
				} else {
					printf("Error: Failed to set RTC!\r\n");
				}
        	}else{
        		g_NTP_ready = false;//模块返回了错误时间,需重新设置模块NTP
        		g_RTC_ready = false;//
        	}

        }
    } else {
        printf("Error: SNTP sync cmd failed or no time data parsed.\r\n");
    }
    // 清空缓冲区为下一次准备
    memset(parsed_time_string, 0, sizeof(parsed_time_string));
}

/* =================== URC处理函数 ===================== */
// +TIME_UPDATED
void SNTP_handle_time_update(const char* line) {
    g_NTP_ready = true;
#ifndef NDEBUG
    printf("Time updated by NTP server.\r\n");
#endif
    // 模块时间同步后，就可以查询并同步了
    RTC_calibrat_BY_NTP();
}

/* ================== 公开API ================== */
void RTC_sntp_init(void) {
    cmd_sntp_cfg = (AT_Cmd_t){
        .cmd_str = sntp_cfg_buf,
        .timeout_ms = 5000,
        .response_cb = _setcfg_rsp_cb,
    };
    cmd_sntp_sync = (AT_Cmd_t){
        .cmd_str = "AT+CIPSNTPTIME?\r\n",
        .timeout_ms = 5000,
        .parser_cb = _sntp_time_parser,
        .response_cb = _sntp_sync_rsp_cb,
    };

    // 检查备份寄存器来决定初始校准状态
    // if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0xCAFE) {
    //     g_RTC_ready = true;
    // }
}

/**
 * @brief 配置ESP模块的SNTP功能和服务器,最多三个,至少需要一个有效/可用服务器
 * @param timezone 时区, 例如中国的东八区为 8
 * @param server1 SNTP服务器地址1, 例如 "ntp.aliyun.com" "cn.pool.ntp.org"
 */
void RTC_sntp_configure_and_enable(int8_t timezone, const char* s1, const char* s2, const char* s3) {
	if(!s1){s1 = "NULL";}if(!s2){s2 = "NULL";}if(!s3){s3 = "NULL";}
    snprintf(sntp_cfg_buf, sizeof(sntp_cfg_buf),
             "AT+CIPSNTPCFG=1,%d,\"%s\",\"%s\",\"%s\"\r\n", timezone, s1, s2, s3);
    AT_controller_cmd_submit(&cmd_sntp_cfg);
}
//尝试校准RTC
void RTC_calibrat_BY_NTP(void) {
    if (!g_NTP_ready) {
        printf("Warning: NTP time not ready.\r\n");
        return;
    }
    AT_controller_cmd_submit(&cmd_sntp_sync);//提交查询NTP时间命令在回调中校准
}
//查询RTC是否校准
bool RTC_is_calibrated(void) {
    return g_RTC_ready;
}



