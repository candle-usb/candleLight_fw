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

#define NUM_CHANNEL 1

static struct {
	uint32_t user_id[NUM_CHANNEL];
} flash_data;

bool flash_set_user_id(uint8_t channel, uint32_t user_id)
{
	if (channel<NUM_CHANNEL) {
		flash_data.user_id[channel] = user_id;
		return true;
	} else {
		return false;
	}
}

uint32_t flash_get_user_id(uint8_t channel)
{
	if (channel<NUM_CHANNEL) {
		return flash_data.user_id[channel];
	} else {
		return 0;
	}
}

void flash_flush()
{

}
