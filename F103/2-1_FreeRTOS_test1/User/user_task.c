#include "user_task.h"
#include "usart.h"
#include "stdio.h"

int fputc(int ch, FILE *f)
{
    // HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    while (huart1.Instance->SR & USART_SR_TXE)
    {
        huart1.Instance->DR = *(uint8_t *)&ch;
    }
    return ch;
}