/*
 * dht11.h
 *
 *  Created on: Oct 21, 2021
 *      Author: Administrator
 */

#ifndef DHT11_DHT11_H_
#define DHT11_DHT11_H_

#include "stm32H7xx_hal.h"
#include "../delay/delay.h"


extern uint8_t humiture[2];


#define DHT11_DA_Pin	GPIO_PIN_4
#define DHT11_DA_Port	GPIOA

void DHT11_IO_OUT (void);
void DHT11_IO_IN (void);
void DHT11_RST (void);
uint8_t Dht11_Check(void);
uint8_t Dht11_ReadBit(void);
uint8_t Dht11_ReadByte(void);

uint8_t DHT11_Init (void);
uint8_t DHT11_ReadData(uint8_t *h);


#endif /* DHT11_DHT11_H_ */
