/*

The MIT License (MIT)

Copyright (c) 2022 Ricky Lopez

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
/* F4-specific code */

#include "can.h"
#include "device.h"
#include "hal_include.h"

void device_can_init(can_data_t *hcan, CAN_TypeDef *instance) {
	__HAL_RCC_CAN1_CLK_ENABLE();

	GPIO_InitTypeDef itd;

	itd.Pin = GPIO_PIN_0|GPIO_PIN_1;
	itd.Mode = GPIO_MODE_AF_PP;
	itd.Pull = GPIO_NOPULL;
	itd.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	itd.Alternate = GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOD, &itd);

	hcan->instance   = instance;
	hcan->brp        = 6;
	hcan->sjw        = 1;
	hcan->phase_seg1 = 12;
	hcan->phase_seg2 = 1;
	return;
}
