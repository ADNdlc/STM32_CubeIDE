/*
 * esp_sys.c
 *
 *  Created on: Aug 13, 2025
 *      Author: 12114
 */
#include "esp_sys.h"
#if USE_MY_MALLOC
#include "malloc/malloc.h"
#endif

#define READ_BUFFER_SIZE 256
#if !USE_MY_MALLOC
static uint8_t temp_read_buffer[READ_BUFFER_SIZE]; // 从驱动读取的临时缓冲区
#else
static uint8_t* temp_read_buffer = NULL;
#endif


// Wi-Fi 事件回调函数
static void wifi_event_handler(wifi_state_typedef new_state) {
    if (new_state != WIFI_STATE_GOT_IP) {

    }
    else if(new_state == WIFI_STATE_DISCONNECTED){
    	MQTT_disconnect();
    }
}

static void MQTT_event_handler(mqtt_state_typedef new_state) {
    if (new_state != MQTT_STATE_CONNECTED) {

    }
}

/*	初始化所有底层模块
 */
void ESP_AT_sys_init(UART_HandleTypeDef* uart_port){
	ATuart_driver_init(uart_port);//绑定AT串口
#if	USE_MY_MALLOC
	if(!temp_read_buffer){
		temp_read_buffer = mymalloc(SRAMDTCM, LINE_BUF_SIZE);
	}
#endif
	AT_parser_init();		// 初始化行解析
	AT_controller_init(); 	// 初始化控制器

	WiFi_init(wifi_event_handler);
	WiFi_set_mode(Station);

	RTC_sntp_init();
	RTC_sntp_configure(8,"cn.pool.ntp.org", "ntp.aliyun.com", 0);
	//设置NTP服务器,模块联网后会自动同步,需要等待一段时间

	MQTT_init(MQTT_event_handler);
}



/*	读取底层接收内容,行解析,维护命令状态机
 * 	放在主函数中,循环调用
 */
void ESP_AT_sys_handle(void){
    if (ATuart_get_readable_bytes() > 0){
        size_t len = ATuart_read(temp_read_buffer, READ_BUFFER_SIZE);//读取到缓冲区中
        if (len > 0)
        {
            // 数据流向: uart_driver(接收) -> parser(分行) -> dispatcher(调用) -> controller(处理)
            AT_parser_input(temp_read_buffer, len);
        }
    }
    AT_controller_process();
}


