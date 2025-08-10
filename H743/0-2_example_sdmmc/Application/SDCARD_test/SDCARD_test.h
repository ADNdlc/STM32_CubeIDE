/*
 * sd_card.h
 *
 *  Created on: Jul 14, 2025
 *      Author: 12114
 */

#ifndef SD_CARD_SD_CARD_H_
#define SD_CARD_SD_CARD_H_

#include "sdmmc.h"

#define myCallback 1

void SDCard_ShowInfo();
void SDCard_EraseBlocks(uint32_t BlockStartAdd,uint32_t BlockEndAdd);
void SDCard_TestWrite();
void SDCard_TestRead();

void SDCard_TestWrite_DMA();
void SDCard_TestRead_DMA();

uint8_t get_busy();

#endif /* SD_CARD_SD_CARD_H_ */
