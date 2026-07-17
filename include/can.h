/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Hubert Denkmair
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "gs_usb.h"
#include "hal_include.h"
#include "led.h"
#include "list.h"

struct can_drv_reg_status {
#if defined (CONFIG_BXCAN)
	uint32_t esr;
#endif
};

enum can_channel_flag {
	CAN_CHANNEL_FLAG_BITTIMING_SET = BIT(0),
	CAN_CHANNEL_FLAG_DATA_BITTIMING_SET = BIT(1),
	CAN_CHANNEL_FLAG_TDC_SET = BIT(2),
};

#define CAN_CHANNEL_BUS_OFF_RESTART_DISABLED 0

typedef struct can_channel {
#if defined (CONFIG_BXCAN)
	CAN_TypeDef *instance;
#endif
	struct can_drv_reg_status reg_status;
	struct list_head list_from_host;
	led_data_t leds;
	uint32_t feature;
	enum can_channel_flag flags;
	enum gs_can_state state;
	uint32_t bus_off_restart;
	struct gs_device_bittiming bittiming;
#ifdef CONFIG_CANFD
	struct gs_device_bittiming data_bittiming;
	struct gs_device_tdc tdc;
#endif
#if defined (CONFIG_BXCAN)
	struct gs_device_filter filter;
#endif
#if (NUM_CAN_CHANNEL > 1)
	uint8_t nr;
#endif
} can_data_t;

extern const struct gs_device_bt_const CAN_btconst;
extern const struct gs_device_bt_const_extended CAN_btconst_ext;
extern const struct gs_device_filter_info CAN_filter_info;
extern const struct gs_device_tdc_const CAN_tdc_const;

struct board_channel_config;

void can_init(can_data_t *channel, const struct board_channel_config *config);

#ifdef CONFIG_CAN_FILTER
void can_set_filter(can_data_t *channel, const struct gs_device_filter *filter);
#else
static inline void can_set_filter(can_data_t __maybe_unused *channel, const struct gs_device_filter  __maybe_unused *filter)
{
}
#endif

bool can_receive(can_data_t *channel, struct gs_host_frame *rx_frame);
bool can_is_rx_pending(can_data_t *channel);

bool can_send(can_data_t *channel, struct gs_host_frame *frame);
