/*
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

#pragma once

#include <stdbool.h>
#include <stdint.h>

struct can_channel;
struct gs_device_state;
struct gs_host_frame;

void can_drv_enable(struct can_channel *channel);
void can_drv_disable(struct can_channel *channel);

uint32_t can_drv_read_reg_status(const struct can_channel *channel);
enum gs_can_state can_drv_get_state(const struct can_channel *channel, const uint32_t reg);
void can_drv_handle_state_change(const struct can_channel *channel, struct gs_host_frame *frame, const uint32_t reg);
void can_drv_get_device_state(const struct can_channel *channel, struct gs_device_state *state, const uint32_t reg);

bool can_drv_bus_error_pending(const uint32_t reg);
void can_drv_handle_bus_error(const struct can_channel *channel, struct gs_host_frame *frame, const uint32_t reg);
