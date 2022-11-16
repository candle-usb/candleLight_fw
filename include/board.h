#pragma once

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

#include "config.h"
#include "usbd_gs_can.h"

struct board_channel_config {
#if defined(CONFIG_BXCAN)
	CAN_TypeDef *interface;
#elif defined(CONFIG_M_CAN)
	FDCAN_GlobalTypeDef *interface;
#endif
};

struct board_config {
	struct board_channel_config channel[NUM_CAN_CHANNEL];
#ifdef CONFIG_PHY
	void (*phy_power_set)(can_data_t *channel, bool enable);
#endif
#ifdef CONFIG_TERMINATION
	void (*termination_set)(can_data_t *channel, enum gs_can_termination_state state);
#endif
};

extern const struct board_config config;

#ifdef CONFIG_PHY
#define SET_PHY_POWER_FN(set_fn) \
		.phy_power_set = (set_fn),
static inline void board_phy_power_set(can_data_t *channel, bool enable)
{
	config.phy_power_set(channel, enable);
}
#else
#define SET_PHY_POWER_FN(set_fn)
static inline void board_phy_power_set(can_data_t *channel, bool enable)
{
	UNUSED(channel);
	UNUSED(enable);
}
#endif

#ifdef CONFIG_TERMINATION
#define SET_TERMINATION_FN(set_fn) \
		.termination_set = (set_fn),
static inline void board_termination_set(can_data_t *channel, enum gs_can_termination_state state)
{
	config.termination_set(channel, state);
}
#else
#define SET_TERMINATION_FN(set_fn)
static inline void board_termination_set(can_data_t *channel, enum gs_can_termination_state state)
{
	UNUSED(channel);
	UNUSED(state);
}
#endif
