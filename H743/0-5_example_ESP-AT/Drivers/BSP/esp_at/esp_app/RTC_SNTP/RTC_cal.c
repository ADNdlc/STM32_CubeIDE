/*
 * RTC_cal.c
 *
 *  Created on: Aug 14, 2025
 *      Author: 12114
 */
#include "RTC_cal.h"
#include "rtc.h"	// 此模块用于校准RTC
#include <string.h>
#include <stdio.h>
#include "../esp_wifi/esp_wifi.h"
#include "../../at_controller.h"


// =========== 私有状态 ===========
static bool g_NTP_ready = 	false; 	// ESP模块的NTP时间是否就绪
static bool g_RTC_ready =   false; 	// STM32的RTC是否已校准

// --- 私有命令对象和缓冲区
static char parsed_time_string[64];//保存查询到的时间

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
        // 命令成功，设置STM32
        char day_of_week_str[4], month_str[4];
        int day, year, hour, min, sec;
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
				// 使用二进制格式设置
				if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK &&
					HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) == HAL_OK)
				{
					g_RTC_ready = true;
#ifndef NDEBUG
					printf("_sntp_sync_rsp_cb: %s\r\n", parsed_time_string);
#endif
					// (可选) 写入一个备份寄存器，表明RTC已校准
					// HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0xCAFE);
				} else {
#ifndef NDEBUG
					printf("_sntp_sync_rsp_cb: RTC Set Fail!\r\n");
#endif
				}
        	}else{
        		g_NTP_ready = false;
        		g_RTC_ready = false;
        	}

        }
    } else {
#ifndef NDEBUG
        printf("_sntp_sync_rsp_cb: sync cmd fail or null data!\r\n");
#endif
    }
    memset(parsed_time_string, 0, sizeof(parsed_time_string));// 清空缓冲区为下一次准备
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



/* ================================ 公开API ================================  */
void RTC_sntp_init(void) {

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
void RTC_sntp_configure(int8_t timezone, const char* s1, const char* s2, const char* s3) {
	if(!s1){s1 = "NULL";}if(!s2){s2 = "NULL";}if(!s3){s3 = "NULL";}
	char sntp_cfg_buf[128];
	AT_Cmd_t cmd_sntp_cfg;
    cmd_sntp_cfg = (AT_Cmd_t){
        .cmd_str = sntp_cfg_buf,
        .timeout_ms = 5000,
        .response_cb = _setcfg_rsp_cb,
    };
    snprintf(sntp_cfg_buf, sizeof(sntp_cfg_buf),
             "AT+CIPSNTPCFG=1,%d,\"%s\",\"%s\",\"%s\"\r\n", timezone, s1, s2, s3);
    AT_controller_cmd_submit(&cmd_sntp_cfg);
}
//尝试校准RTC
void RTC_calibrat_BY_NTP(void) {
    if (!g_NTP_ready) {
#ifndef NDEBUG
        printf("RTC_calibrat_BY_NTP: NTP not-ready!\r\n");
#endif
        return;
    }
    AT_Cmd_t cmd_sntp_sync;
    cmd_sntp_sync = (AT_Cmd_t){
        .cmd_str = "AT+CIPSNTPTIME?\r\n",
        .timeout_ms = 5000,
        .parser_cb = _sntp_time_parser,
        .response_cb = _sntp_sync_rsp_cb,
    };
    AT_controller_cmd_submit(&cmd_sntp_sync);//提交查询NTP时间命令在回调中校准
}
//查询RTC是否校准
bool RTC_is_calibrated(void) {
    return g_RTC_ready;
}


time_t RTC_get_unix(int8_t timezone) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return -1; // 读取时间失败
    }
    if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return -1; // 读取日期失败
    }
    // 将HAL的结构体格式转换为标准C库的 `struct tm` 格式
    struct tm time_info = {0};
    // 年份: tm_year 是从1900年开始的年数
    // sDate.Year 的范围是 0-99，代表 2000-2099 年
    time_info.tm_year = sDate.Year + 2000 - 1900;
    // 月份: tm_mon 的范围是 0-11
    time_info.tm_mon = sDate.Month - 1;
    // 日: tm_mday 的范围是 1-31
    time_info.tm_mday = sDate.Date;
    // 时、分、秒
    time_info.tm_hour = sTime.Hours;
    time_info.tm_min = sTime.Minutes;
    time_info.tm_sec = sTime.Seconds;
    // 使用 mktime() 将本地时间结构体转换为本地时间戳
    // mktime() 会自动处理闰年等所有复杂的日历计算
    time_t local_timestamp = mktime(&time_info);
    if (local_timestamp == -1) {
        return -1; // 转换失败
    }
    // 根据时区偏移量，将本地时间戳校正为UTC时间戳
    time_t utc_timestamp = local_timestamp - (timezone * 3600L);
    return utc_timestamp;
}

