/*
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Marc Kleine-Budde <kernel@pengutronix.de>
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

#include "can.h"
#include "compiler.h"
#include "usbd_gs_can.h"

#if NUM_CAN_CHANNEL == 1

static inline can_data_t
*gs_host_frame_object_get_channel(USBD_GS_CAN_HandleTypeDef *hcan,
								  struct gs_host_frame_object __maybe_unused *frame_object)
{
	return hcan->channels;
}

static inline void
gs_host_frame_object_set_channel(struct gs_host_frame_object __maybe_unused *frame_object,
								 can_data_t __maybe_unused *channel)
{
}

#else

static inline can_data_t
*gs_host_frame_object_get_channel(USBD_GS_CAN_HandleTypeDef __maybe_unused *hcan,
								  struct gs_host_frame_object *frame_object)
{
	return frame_object->channel;
}

static inline void
gs_host_frame_object_set_channel(struct gs_host_frame_object *frame_object,
								 can_data_t *channel)
{
	frame_object->channel = channel;
}

#endif
