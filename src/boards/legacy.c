/*

The MIT License (MIT)

Copyright (c) 2023 Pengutronix,
              Jonas Martin <kernel@pengutronix.de>

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

#include "board.h"
#include "config.h"
#include "device.h"
#include "gpio.h"
#include "usbd_gs_can.h"

static void legacy_phy_power_set(can_data_t *channel, bool enable) {
	UNUSED(channel);

	if (enable) {
#ifdef CAN_S_GPIO_Port
		HAL_GPIO_WritePin(CAN_S_GPIO_Port, CAN_S_Pin, GPIO_PIN_RESET);
#endif
#ifdef nCANSTBY_Pin
		HAL_GPIO_WritePin(nCANSTBY_Port, nCANSTBY_Pin,
						  !GPIO_INIT_STATE(nCANSTBY_Active_High));
#endif
	} else {
#ifdef nCANSTBY_Pin
		HAL_GPIO_WritePin(nCANSTBY_Port, nCANSTBY_Pin,
						  GPIO_INIT_STATE(nCANSTBY_Active_High));
#endif
	}
}

const struct BoardConfig config = {
	.phy_power_set = legacy_phy_power_set,
	.channels[0].interface = CAN_INTERFACE,
};
