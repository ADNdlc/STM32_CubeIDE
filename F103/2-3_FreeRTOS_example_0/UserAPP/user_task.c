#include "user_task.h"
#include "usart.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

void User_Task_1(void){

    vTaskDelay(500);
}

void User_Task_2(void){
    vTaskDelay(500);
}

void vLEDTask(void){
    vTaskDelay(10);
}

int fputc(int ch, FILE *f){
    // HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    while (!(huart1.Instance->SR & USART_SR_TXE))
        ;
    huart1.Instance->DR = *(uint8_t *)&ch;
    return ch;
}
