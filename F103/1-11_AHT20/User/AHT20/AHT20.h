/*
 * AHT20.h
 *
 *  Created on: May 4, 2025
 *      Author: 12114
 */

#ifndef AHT20_AHT20_H_
#define AHT20_AHT20_H_

#include "i2c.h"

void AHT20_Init(void);
void AHT20_Read(float *Temperature,float *Humidity);

#endif /* AHT20_AHT20_H_ */
