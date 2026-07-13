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

#include <string.h>

#include "can_common.h"
#include "can_drv.h"
#include "host_frame.h"
#include "led.h"
#include "timer.h"
#include "usbd_gs_can.h"

#define CAN_ERROR_WARNING_THRESHOLD 96
#define CAN_ERROR_PASSIVE_THRESHOLD 128
#define CAN_BUS_OFF_THRESHOLD		256

#ifndef CONFIG_CANFD
const struct gs_device_bt_const_extended CAN_btconst_ext;
#endif

#ifndef CONFIG_CAN_FILTER
const struct gs_device_filter_info CAN_filter_info;
#endif

bool can_check_bittiming_ok(const struct can_bittiming_const *btc,
							const struct gs_device_bittiming *timing)
{
	const uint32_t tseg1 = timing->prop_seg + timing->phase_seg1;

	if (tseg1 < btc->tseg1_min ||
		tseg1 > btc->tseg1_max ||
		timing->phase_seg2 < btc->tseg2_min ||
		timing->phase_seg2 > btc->tseg2_max ||
		timing->sjw > btc->sjw_max ||
		timing->brp < btc->brp_min ||
		timing->brp > btc->brp_max)
		return false;

	return true;
}

#ifdef CONFIG_CAN_FILTER
bool can_check_filter_ok(const struct gs_device_filter *filter)
{
	return filter->info.dev == CAN_filter_info.dev;
}
#endif

void can_enable(struct can_channel *channel, const uint32_t feature)
{
	channel->feature = feature;

	led_set_mode(&channel->leds, LED_MODE_NORMAL);
	channel->state = GS_CAN_STATE_ERROR_ACTIVE;
	can_drv_enable(channel);
}

void can_disable(USBD_GS_CAN_HandleTypeDef *hcan, struct can_channel *channel)
{
	can_drv_disable(channel);
	usbd_gs_can_purge_to_host_list_by_channel(hcan, channel);
	channel->state = GS_CAN_STATE_STOPPED;
	led_set_mode(&channel->leds, LED_MODE_OFF);
}

void CAN_SendFrame(USBD_GS_CAN_HandleTypeDef *hcan, can_data_t *channel)
{
	struct gs_host_frame_object *frame_object;

	bool was_irq_enabled = disable_irq();
	frame_object = list_first_entry_or_null(&channel->list_from_host,
											struct gs_host_frame_object,
											list);
	if (!frame_object) {
		restore_irq(was_irq_enabled);
		return;
	}

	list_del(&frame_object->list);
	restore_irq(was_irq_enabled);

	struct gs_host_frame *frame = &frame_object->frame;

	if (!can_send(channel, frame)) {
		list_add_locked(&frame_object->list, &channel->list_from_host);
		return;
	}

	// Echo sent frame back to host
	frame->reserved = 0x0;
	if (IS_ENABLED(CONFIG_CANFD) && frame->flags & GS_CAN_FLAG_FD)
		frame->canfd_ts->timestamp_us = timer_get();
	else
		frame->classic_can_ts->timestamp_us = timer_get();

	list_add_tail_locked(&frame_object->list, &hcan->list_to_host);

	led_indicate_trx(&channel->leds, LED_TX);
}

void CAN_ReceiveFrame(USBD_GS_CAN_HandleTypeDef *hcan, can_data_t *channel)
{
	struct gs_host_frame_object *frame_object;

	if (!can_is_rx_pending(channel)) {
		return;
	}

	frame_object = gs_host_frame_object_get_locked(hcan);
	if (!frame_object) {
		return;
	}

	struct gs_host_frame *frame = &frame_object->frame;

	if (!can_receive(channel, frame)) {
		list_add_tail_locked(&frame_object->list, &hcan->list_frame_pool);
		return;
	}

	frame->echo_id = GS_HOST_FRAME_ECHO_ID_RX; // not an echo frame
	frame->reserved = 0;

	list_add_tail_locked(&frame_object->list, &hcan->list_to_host);

	led_indicate_trx(&channel->leds, LED_RX);
}

void can_get_device_state(const struct can_channel *channel, struct gs_device_state *state)
{
	const uint32_t reg_status = can_drv_read_reg_status(channel);

	can_drv_get_device_state(channel, state, reg_status);
}

static void can_prepare_error_frame(const struct can_channel *channel,
									struct gs_host_frame *frame)

{
	frame->echo_id = GS_HOST_FRAME_ECHO_ID_RX;
	frame->can_id = CAN_ERR_FLAG;
	frame->can_dlc = CAN_ERR_DLC;
	frame->channel = can_channel_get_nr(channel);
	frame->flags = 0;
	frame->reserved = 0;
	memset(frame->classic_can->data, 0x0, sizeof(frame->classic_can->data));

	frame->classic_can_ts->timestamp_us = timer_get();
}

enum gs_can_state can_err_to_state(const uint16_t err)
{
	if (err < CAN_ERROR_WARNING_THRESHOLD)
		return GS_CAN_STATE_ERROR_ACTIVE;
	if (err < CAN_ERROR_PASSIVE_THRESHOLD)
		return GS_CAN_STATE_ERROR_WARNING;
	if (err < CAN_BUS_OFF_THRESHOLD)
		return GS_CAN_STATE_ERROR_PASSIVE;

	return GS_CAN_STATE_BUS_OFF;
}

uint8_t gs_can_tx_state_to_frame(const enum gs_can_state state)
{
	switch (state) {
		case GS_CAN_STATE_ERROR_ACTIVE:
			return CAN_ERR_CRTL_ACTIVE;
		case GS_CAN_STATE_ERROR_WARNING:
			return CAN_ERR_CRTL_TX_WARNING;
		case GS_CAN_STATE_ERROR_PASSIVE:
			return CAN_ERR_CRTL_TX_PASSIVE;
		default:
			return 0;
	}
}

uint8_t gs_can_rx_state_to_frame(const enum gs_can_state state)
{
	switch (state) {
		case GS_CAN_STATE_ERROR_ACTIVE:
			return CAN_ERR_CRTL_ACTIVE;
		case GS_CAN_STATE_ERROR_WARNING:
			return CAN_ERR_CRTL_RX_WARNING;
		case GS_CAN_STATE_ERROR_PASSIVE:
			return CAN_ERR_CRTL_RX_PASSIVE;
		default:
			return 0;
	}
}

void can_lec_error_to_frame(struct gs_host_frame *frame, const uint8_t lec)
{
	switch (lec) {
		case CAN_LEC_STUFF_ERROR:
			frame->classic_can->data[2] |= CAN_ERR_PROT_STUFF;
			break;
		case CAN_LEC_FORM_ERROR:
			frame->classic_can->data[2] |= CAN_ERR_PROT_FORM;
			break;
		case CAN_LEC_ACK_ERROR:
			frame->can_id |= CAN_ERR_ACK;
			break;
		case CAN_LEC_REC_ERROR:
			frame->classic_can->data[2] |= CAN_ERR_PROT_BIT1;
			break;
		case CAN_LEC_DOM_ERROR:
			frame->classic_can->data[2] |= CAN_ERR_PROT_BIT0;
			break;
		case CAN_LEC_CRC_ERROR:
			frame->classic_can->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
			break;
		default:
			break;
	}
}

static void can_handle_bus_error(USBD_GS_CAN_HandleTypeDef *hcan, const struct can_channel *channel,
								 const uint32_t reg_status)
{
	struct gs_host_frame_object *frame_object = gs_host_frame_object_get_locked(hcan);
	if (!frame_object)
		return;

	struct gs_host_frame *frame = &frame_object->frame;

	can_prepare_error_frame(channel, frame);
	bool handled = can_drv_handle_bus_error(channel, frame, reg_status);
	if (handled) {
		list_add_tail_locked(&frame_object->list, &hcan->list_to_host);
	} else {
		list_add_locked(&frame_object->list, &hcan->list_frame_pool);
	}
}

static bool can_bus_error_pending(const struct can_channel *channel, const uint32_t reg_status)
{
	if (!(channel->feature & GS_CAN_FEATURE_BERR_REPORTING)) {
		return false;
	}

	return can_drv_bus_error_pending(reg_status);
}

static void can_handle_state_change(USBD_GS_CAN_HandleTypeDef *hcan, struct can_channel *channel,
									const uint32_t reg_status)
{
	struct gs_host_frame_object *frame_object = gs_host_frame_object_get_locked(hcan);
	if (!frame_object)
		return;

	struct gs_host_frame *frame = &frame_object->frame;
	can_prepare_error_frame(channel, frame);

	if (channel->state == GS_CAN_STATE_BUS_OFF) {
		frame->can_id |= CAN_ERR_BUSOFF;
	} else {
		frame->can_id |= CAN_ERR_CRTL | CAN_ERR_CNT;
		can_drv_handle_state_change(channel, frame, reg_status);
	}

	list_add_tail_locked(&frame_object->list, &hcan->list_to_host);
}

static bool can_state_change_pending(struct can_channel *channel, const uint32_t reg_status)
{
	if (channel->state >= GS_CAN_STATE_STOPPED)
		return false;

	const enum gs_can_state new_state = can_drv_get_state(reg_status);
	if (channel->state == new_state)
		return false;

	channel->state = new_state;

	return true;
}

// If there are frames to receive, don't report any error frames. The
// best we can localize the errors to is "after the last successfully
// received frame", so wait until we get there. LEC will hold some error
// to report even if multiple pass by.
void CAN_HandleError(USBD_GS_CAN_HandleTypeDef *hcan, can_data_t *channel)
{
	if (can_is_rx_pending(channel)) {
		return;
	}

	const uint32_t reg_status = can_drv_read_reg_status(channel);

	if (can_state_change_pending(channel, reg_status)) {
		can_handle_state_change(hcan, channel, reg_status);
	} else if (can_bus_error_pending(channel, reg_status)) {
		can_handle_bus_error(hcan, channel, reg_status);
	}
}
