/*

The MIT License (MIT)

Copyright (c) 2016 Hubert Denkmair

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/


#include "flash.h"
#include <string.h>
#include "stm32f0xx_hal_flash.h"

#define NUM_CHANNEL 1

typedef struct {
	uint32_t user_id[NUM_CHANNEL];
} flash_data_t;

static flash_data_t flash_data_ram;
__attribute__((__section__(".user_data"))) const flash_data_t flash_data_rom;


void flash_load()
{
	memcpy(&flash_data_ram, &flash_data_rom, sizeof(flash_data_t));
}

bool flash_set_user_id(uint8_t channel, uint32_t user_id)
{
	if (channel<NUM_CHANNEL) {

		if (flash_data_ram.user_id[channel] != user_id) {
			flash_data_ram.user_id[channel] = user_id;
			flash_flush();
		}

		return true;
	} else {
		return false;
	}
}

uint32_t flash_get_user_id(uint8_t channel)
{
	if (channel<NUM_CHANNEL) {
		return flash_data_ram.user_id[channel];
	} else {
		return 0;
	}
}

void flash_flush()
{

	FLASH_EraseInitTypeDef erase_pages;
	erase_pages.PageAddress = (uint32_t)&flash_data_rom;
	erase_pages.NbPages = 1;
	erase_pages.TypeErase = TYPEERASE_PAGES;

	uint32_t error;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_SR_PGERR);
	HAL_FLASHEx_Erase(&erase_pages, &error);
	if (error==0xFFFFFFFF) { // erase finished successfully
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&flash_data_rom.user_id[0], flash_data_ram.user_id[0]);
	}
	HAL_FLASH_Lock();
}
