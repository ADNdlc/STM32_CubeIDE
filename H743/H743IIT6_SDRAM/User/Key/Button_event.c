/*
 * Button_event.c
 *
 *  Created on: Jun 14, 2025
 *      Author: 12114
 */


#include "Button_event.h"
#include"../inc/retarget.h"
#include "main.h"

/*实例化按钮*/
Button btn1;
Button btn2;
Button btn3;

extern uint16_t fps;

/*================btn1========================*/
void btn1_single_click(Button* btn) {
	printf("\r\nbtn1_single_click\r\n");
    // 单击处理
	printf("\r\nfps:%d",fps);

}

void btn1_double_click(Button* btn) {
	printf("\r\nbtn1_double_click\r\n");
    // 双击处理
	fps=0;
	printf("\r\nfps归零");
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
}

/*================btn3========================*/
void btn3_single_click(Button* btn) {
	printf("\r\nbtn3_single_click\r\n");
    // 单击处理

}

void btn3_double_click(Button* btn) {
	printf("\r\nbtn3_double_click\r\n");
    // 双击处理
}

void btn3_triple_click(Button* btn) {
	printf("\r\nbtn3_triple_click\r\n");
    // 三击处理

}
void btn3_long_click(Button* btn) {
	printf("\r\nbtn3_long_click\r\n");
    // 长按处理
}


void Button_Init(void){
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

	Button_Set(&btn3,btn3_GPIO_Port,btn3_Pin,GPIO_PIN_RESET);
	btn3.SinglePressHandler = btn3_single_click;
	btn3.DoublePressHandler = btn3_double_click;
	btn3.TriplePressHandler = btn3_triple_click;
	btn3.LongPressHandler = btn3_long_click;
}


void Button_UPDATE(void){
	Button_Update(&btn1);
	Button_Update(&btn2);
	Button_Update(&btn3);
}
