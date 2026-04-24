#include "user_task.h"
#include "usart.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

extern osMessageQueueId_t ButtonQueueHandle;
extern osMessageQueueId_t TimeQueueHandle;
extern osMessageQueueId_t printQueueHandle;
extern osSemaphoreId_t myBinarySem01Handle;
extern osSemaphoreId_t myCountingSem01Handle;
void User_Task_1(void)
{

    vTaskDelay(500);
}

void User_Task_2(void)
{
    switch (uxSemaphoreGetCount(myCountingSem01Handle))
    {
    case 0:
        break;
    case 1:
        printf("num = 1\r\n");
        break;
    case 2:
        printf("num = 2\r\n");
        break;
    case 3:
        printf("num = 3\r\n");
        break;

    default:
        break;
    }
    vTaskDelay(500);
}

void vButtonScan(void)
{
    uint8_t ButtonValueCurrent;
    static uint8_t ButtonValueLast = 0;
    const uint8_t * ButtonValueChar;

    ButtonValueCurrent = 0;
    if (HAL_GPIO_ReadPin(btn1_GPIO_Port, btn1_Pin) == GPIO_PIN_RESET)
    {
        ButtonValueCurrent |= 0x01;
        if((ButtonValueLast & 0x01) == 0){
            ButtonValueChar = "A";
            xQueueSend(printQueueHandle, &ButtonValueChar, 0);
        }
    }
    if (HAL_GPIO_ReadPin(btn2_GPIO_Port, btn2_Pin) == GPIO_PIN_RESET)
    {
        ButtonValueCurrent |= 0x02;
        if((ButtonValueLast & 0x02) == 0){
            ButtonValueChar = "B";
            xQueueSend(printQueueHandle, &ButtonValueChar, 0);
        }
    }
    if (HAL_GPIO_ReadPin(btn3_GPIO_Port, btn3_Pin) == GPIO_PIN_RESET)
    {
        ButtonValueCurrent |= 0x04;  // 修正：btn3应该使用0x04而不是0x03
        if((ButtonValueLast & 0x04) == 0){
            ButtonValueChar = "C";
            xQueueSend(printQueueHandle, &ButtonValueChar, 0);
        }
    }
    if (ButtonValueCurrent != ButtonValueLast)
    {
        xQueueSend(ButtonQueueHandle, &ButtonValueCurrent, 0);
    }

    uint8_t mask = 0x01;            // 按钮位掩码
    for (uint8_t i = 0; i < 3; i++) // 遍历按钮按下的情况
    {
        if (((ButtonValueCurrent & mask) != 0) && ((ButtonValueLast & mask) == 0)) // 按键按下
        {
            xSemaphoreGive(myCountingSem01Handle); // 按下时增加计数
        }
        else if (((ButtonValueCurrent & mask) == 0) && ((ButtonValueLast & mask) != 0)) // 按键释放
        {
            xSemaphoreTake(myCountingSem01Handle, 0); // 松开时减少计数
        }
        mask <<= 1; // 检查下一个
    }

    ButtonValueLast = ButtonValueCurrent;
    vTaskDelay(20);
}

void vLEDTask(void)
{
    uint8_t ButtonValue;
    uint16_t TimerValue;
    if (xQueueReceive(ButtonQueueHandle, &ButtonValue, 0)) // 检查按键队列，不阻塞
    {
        // 使用位检测而不是值比较
        if (ButtonValue & 0x01)  // btn1按下
        {
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
        }
        if (ButtonValue & 0x02)  // btn2按下
        {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
        }
        if (ButtonValue & 0x04)  // btn3按下
        {
            // HAL_GPIO_WritePin(led3_GPIO_Port, led3_Pin, GPIO_PIN_SET);
        }
        else
        {
            // HAL_GPIO_WritePin(led3_GPIO_Port, led3_Pin, GPIO_PIN_RESET);
        }
    }
    // if (xQueueReceive(TimeQueueHandle, &TimerValue, 0))
    if (xSemaphoreTake(myBinarySem01Handle, 0))
    {
        HAL_GPIO_TogglePin(led0_GPIO_Port, led0_Pin);
    }
    vTaskDelay(10);
}

int fputc(int ch, FILE *f)
{
    // HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    while (!(huart2.Instance->SR & USART_SR_TXE))
        ;
    huart2.Instance->DR = *(uint8_t *)&ch;
    return ch;
}
