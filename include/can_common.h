/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Marc Kleine-Budde <kernel@pengutronix.de>
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

#include "can.h"
#include "usbd_gs_can.h"

#define CAN_LEC_NO_ERROR	0
#define CAN_LEC_STUFF_ERROR 1
#define CAN_LEC_FORM_ERROR	2
#define CAN_LEC_ACK_ERROR	3
#define CAN_LEC_REC_ERROR	4
#define CAN_LEC_DOM_ERROR	5
#define CAN_LEC_CRC_ERROR	6
#define CAN_LEC_SOFTWARE	7

bool can_check_feature_ok(const can_data_t *channel, const uint32_t feature);
bool can_check_bittiming_ok(const struct can_bittiming_const *btc, const struct gs_device_bittiming *timing);
void can_set_bittiming(struct can_channel *channel, const struct gs_device_bittiming *bt);

#ifdef CONFIG_CANFD
void can_set_data_bittiming(struct can_channel *channel, const struct gs_device_bittiming *timing);
bool can_check_tdc_ok(const struct gs_device_tdc_const *tdc_const, const struct gs_device_tdc *tdc);
void can_set_tdc(struct can_channel *channel, const struct gs_device_tdc *tdc);
void can_get_device_tdc(const struct can_channel *channel, struct gs_device_tdc *tdc);
#else
static inline void can_set_data_bittiming(struct can_channel __maybe_unused *channel,
										  const struct gs_device_bittiming __maybe_unused *timing)
{
}

static inline bool can_check_tdc_ok(const struct gs_device_tdc_const __maybe_unused *tdc_const,
									const struct gs_device_tdc __maybe_unused *tdc)
{
	return false;
}

static inline void can_set_tdc(struct can_channel __maybe_unused *channel,
							   const struct gs_device_tdc __maybe_unused *tdc)
{
}

static inline void can_get_device_tdc(const struct can_channel __maybe_unused *channel,
									  struct gs_device_tdc __maybe_unused *tdc)
{
}
#endif

#ifdef CONFIG_CAN_FILTER
bool can_check_filter_ok(const struct gs_device_filter *filter);
#else
static inline bool can_check_filter_ok(const struct gs_device_filter __maybe_unused *filter)
{
	return false;
}
#endif

#if (NUM_CAN_CHANNEL > 1)
static inline void can_channel_set_nr(can_data_t *channel, const uint8_t nr)
{
	channel->nr = nr;
}

static inline uint8_t can_channel_get_nr(const can_data_t *channel)
{
	return channel->nr;
}
#else
static inline void can_channel_set_nr(can_data_t __maybe_unused *channel, const uint8_t __maybe_unused nr)
{
}

static inline uint8_t can_channel_get_nr(const can_data_t __maybe_unused *channel)
{
	return 0;
}
#endif

static inline bool can_is_lec_error(const uint8_t lec)
{
	if (lec == CAN_LEC_NO_ERROR || lec == CAN_LEC_SOFTWARE)
		return false;

	return true;
}

bool can_is_enabled(const struct can_channel *channel);
void can_enable(struct can_channel *channel, uint32_t mode);
void can_disable(USBD_GS_CAN_HandleTypeDef *hcan, struct can_channel *channel);
void can_get_device_state(const struct can_channel *channel, struct gs_device_state *state);

enum gs_can_state can_err_to_state(const uint16_t err);
uint8_t gs_can_tx_state_to_frame(const enum gs_can_state state);
uint8_t gs_can_rx_state_to_frame(const enum gs_can_state state);
void can_lec_error_to_frame(struct gs_host_frame *frame, const uint8_t lec);

bool can_check_bus_off_recovery_ok(const struct can_channel *channel);
void can_schedule_bus_off_recovery(struct can_channel *channel, uint32_t delay_ms);

void CAN_SendFrame(USBD_GS_CAN_HandleTypeDef *hcan, can_data_t *channel);
void CAN_ReceiveFrame(USBD_GS_CAN_HandleTypeDef *hcan, can_data_t *channel);
void CAN_HandleError(USBD_GS_CAN_HandleTypeDef *hcan, can_data_t *channel);
