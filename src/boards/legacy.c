/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Pengutronix,
 *               Jonas Martin <kernel@pengutronix.de>
 * Copyright (c) 2026 Pengutronix,
 *               Marc Kleine-Budde <kernel@pengutronix.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "board.h"
#include "config.h"
#include "device.h"
#include "gpio.h"
#include "usbd_gs_can.h"

static void __maybe_unused legacy_phy_power_set(can_data_t *channel, bool enable)
{
	UNUSED(channel);

	if (enable) {
		if (IS_ENABLED(CONFIG_PHY_STANDBY)) {
			HAL_GPIO_WritePin(nCANSTBY_Port, nCANSTBY_Pin,
							  !GPIO_INIT_STATE(nCANSTBY_Active_High));
		}
	} else {
		if (IS_ENABLED(CONFIG_PHY_STANDBY)) {
			HAL_GPIO_WritePin(nCANSTBY_Port, nCANSTBY_Pin,
							  GPIO_INIT_STATE(nCANSTBY_Active_High));
		}
	}
}

const struct board_config config = {
	.channel[0] = {
		.interface = CAN_INTERFACE,
	},
	SET_PHY_POWER_FN(legacy_phy_power_set)
};
