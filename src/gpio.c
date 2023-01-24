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

#include "config.h"
#include "gpio.h"
#include "hal_include.h"

#ifdef TERM_Pin
static int term_state = 0;

enum gs_can_termination_state get_term(can_data_t * channel)
{
	const uint8_t nr = channel->nr;

	if (term_state & (1 << nr)) {
		return GS_CAN_TERMINATION_STATE_ON;
	} else {
		return GS_CAN_TERMINATION_STATE_OFF;
	}
}

enum gs_can_termination_state set_term(can_data_t *channel, enum gs_can_termination_state state)
{
	const uint8_t nr = channel->nr;

	if (state == GS_CAN_TERMINATION_STATE_ON) {
		term_state |= 1 << nr;
	} else {
		term_state &= ~(1 << nr);
	}

#if (TERM_Active_High == 1)
	#define TERM_ON	 GPIO_PIN_SET
	#define TERM_OFF GPIO_PIN_RESET
#else
	#define TERM_ON	 GPIO_PIN_RESET
	#define TERM_OFF GPIO_PIN_SET
#endif

	HAL_GPIO_WritePin(TERM_GPIO_Port, TERM_Pin, (state ? TERM_ON : TERM_OFF));

	return state;
}

#endif
