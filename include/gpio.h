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

#pragma once

#include "config.h"
#include "gs_usb.h"

void gpio_init(void);

/* macro to define init (i.e. reset) state of a pin */
#define GPIO_INIT_STATE(active_high) (((active_high) == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET)

#ifdef TERM_Pin
enum gs_can_termination_state set_term(unsigned int channel, enum gs_can_termination_state state);
enum gs_can_termination_state get_term(unsigned int channel);

#else
static inline enum gs_can_termination_state set_term(unsigned int channel, enum gs_can_termination_state state)
{
	(void)channel;
	(void)state;
	return GS_CAN_TERMINATION_UNSUPPORTED;
}

static inline enum gs_can_termination_state get_term(unsigned int channel)
{
	(void)channel;
	return GS_CAN_TERMINATION_UNSUPPORTED;
}

#endif
