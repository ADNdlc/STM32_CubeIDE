/*
 * Button_event.c
 *
 *  Created on: Jun 14, 2025
 *      Author: 12114
 */


#include "Button_event.h"
#include "retarget.h"
#include "main.h"
#include "esp_at/esp_app/esp_sys/esp_sys.h"

static void test1_parser_cb(const char* data_line){
	printf("test1_parser_cb data: %s\r\n",data_line+11);
}
static void test1_response_cb(AT_CmdResult_t result, const char* line){
	printf("test1_response_cb result: %d line:%s\r\n", result, line);
}

static AT_Cmd_t test1CMD = {
	.cmd_str = "test1CMD\r\n",
	.data_to_send = "data123",
	.timeout_ms = 0xFFFF,
	.parser_cb = test1_parser_cb,
	.response_cb = test1_response_cb,
	.next = NULL
};


extern wifi_info_t wifi_info1;
extern wifi_info_t wifi_info2;

/*实例化按钮*/
Button btn0;
Button btn1;
Button btn2;

/*================btn0========================*/
void btn0_single_click(Button* btn) {
	printf("\r\nbtn0_single_click\r\n");
    // 单击处理
	AT_controller_cmd_submit(&test1CMD);//测试命令
}

void btn0_double_click(Button* btn) {
	printf("\r\nbtn0_double_click\r\n");
    // 双击处理
	WiFi_connect(&wifi_info2);		//连接wifi
}

void btn0_triple_click(Button* btn) {
	printf("\r\nbtn0_triple_click\r\n");
    // 三击处理

}
void btn0_long_click(Button* btn) {
	printf("\r\nbtn0_long_click\r\n");
    // 长按处理
	WiFi_disconnect();				//断开wifi

}

/*================btn1========================*/
void btn1_single_click(Button* btn) {
	printf("\r\nbtn1_single_click\r\n");
    // 单击处理
	//WiFi_connect(&wifi_info1);		//测试错误Wifi

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	if(RTC_is_calibrated()){
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		printf("日期为:20%d-%d-%d\r\n",sDate.Year,sDate.Month,sDate.Date);
		printf("时间为:%dh-%dmin-%dsec\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
	}else{
		printf("RTC not ready.\r\n");
	}
}

void btn1_double_click(Button* btn) {
	printf("\r\nbtn1_double_click\r\n");
    // 双击处理

}

void btn1_triple_click(Button* btn) {
	printf("\r\nbtn1_triple_click\r\n");
    // 三击处理

}
void btn1_long_click(Button* btn) {
	printf("\r\nbtn1_long_click\r\n");
    // 长按处理
}

/*================btn2========================*/
void btn2_single_click(Button* btn) {
	printf("\r\nbtn2_single_click\r\n");
    // 单击处理
	MQTT_connect(MQTT_Get_DeviceID(),Product_ID,
			"version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2Ftest2&et=1855499668539&method=md5&sign=%2FHVmg4Xz2RfTRWEu44mApQ%3D%3D");
}

void btn2_double_click(Button* btn) {
	printf("\r\nbtn2_double_click\r\n");
    // 双击处理
}

void btn2_triple_click(Button* btn) {
	printf("\r\nbtn2_triple_click\r\n");
    // 三击处理

}
void btn2_long_click(Button* btn) {
	printf("\r\nbtn2_long_click\r\n");
    // 长按处理
	MQTT_disconnect();
}


void Button_Init(void){
	Button_Set(&btn0,btn0_GPIO_Port,btn0_Pin,GPIO_PIN_RESET);
	btn0.SinglePressHandler = btn0_single_click;
	btn0.DoublePressHandler = btn0_double_click;
	btn0.TriplePressHandler = btn0_triple_click;
	btn0.LongPressHandler = btn0_long_click;

	Button_Set(&btn1,btn1_GPIO_Port,btn1_Pin,GPIO_PIN_RESET);
	btn1.SinglePressHandler = btn1_single_click;
	btn1.DoublePressHandler = btn1_double_click;
	btn1.TriplePressHandler = btn1_triple_click;
	btn1.LongPressHandler = btn1_long_click;

	Button_Set(&btn2,btn2_GPIO_Port,btn2_Pin,GPIO_PIN_RESET);
	btn2.SinglePressHandler = btn2_single_click;
	btn2.DoublePressHandler = btn2_double_click;
	btn2.TriplePressHandler = btn2_triple_click;
	btn2.LongPressHandler = btn2_long_click;
}


void Button_UPDATE(void){
	Button_Update(&btn0);
	Button_Update(&btn1);
	Button_Update(&btn2);
}
