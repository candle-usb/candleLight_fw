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

#include "can_common.h"
#include "led.h"
#include "timer.h"
#include "usbd_gs_can.h"

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

	bool was_irq_enabled = disable_irq();
	frame_object = list_first_entry_or_null(&hcan->list_frame_pool,
											struct gs_host_frame_object,
											list);
	if (!frame_object) {
		restore_irq(was_irq_enabled);
		return;
	}

	list_del(&frame_object->list);
	restore_irq(was_irq_enabled);

	struct gs_host_frame *frame = &frame_object->frame;

	if (!can_receive(channel, frame)) {
		list_add_tail_locked(&frame_object->list, &hcan->list_frame_pool);
		return;
	}

	frame->classic_can_ts->timestamp_us = timer_get();
	frame->echo_id = 0xFFFFFFFF; // not an echo frame
	frame->reserved = 0;

	list_add_tail_locked(&frame_object->list, &hcan->list_to_host);

	led_indicate_trx(&channel->leds, LED_RX);
}

// If there are frames to receive, don't report any error frames. The
// best we can localize the errors to is "after the last successfully
// received frame", so wait until we get there. LEC will hold some error
// to report even if multiple pass by.
void CAN_HandleError(USBD_GS_CAN_HandleTypeDef *hcan, can_data_t *channel)
{
	struct gs_host_frame_object *frame_object;

	if (can_is_rx_pending(channel)) {
		return;
	}

	uint32_t can_err = can_get_error_status(channel);

	bool was_irq_enabled = disable_irq();
	frame_object = list_first_entry_or_null(&hcan->list_frame_pool,
											struct gs_host_frame_object,
											list);
	if (!frame_object) {
		restore_irq(was_irq_enabled);
		return;
	}

	list_del(&frame_object->list);
	restore_irq(was_irq_enabled);

	struct gs_host_frame *frame = &frame_object->frame;
	frame->classic_can_ts->timestamp_us = timer_get();
	frame->channel = channel->nr;

	if (can_parse_error_status(channel, frame, can_err)) {
		list_add_tail_locked(&frame_object->list, &hcan->list_to_host);
	} else {
		list_add_tail_locked(&frame_object->list, &hcan->list_frame_pool);
	}
}
