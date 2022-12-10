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

#include <string.h>

#include "led.h"
#include "hal_include.h"
#include "util.h"

#define SEQ_ISPASSED(now, target) ((int32_t) ((now) - (target)) >= 0)

void led_init(
	led_data_t *leds,
	void* led_rx_port, uint16_t led_rx_pin, bool led_rx_active_high,
	void* led_tx_port, uint16_t led_tx_pin, bool led_tx_active_high
	) {
	memset(leds, 0, sizeof(led_data_t));
	leds->led_state[led_rx].port = led_rx_port;
	leds->led_state[led_rx].pin = led_rx_pin;
	leds->led_state[led_rx].is_active_high = led_rx_active_high;
	leds->led_state[led_tx].port = led_tx_port;
	leds->led_state[led_tx].pin = led_tx_pin;
	leds->led_state[led_tx].is_active_high = led_tx_active_high;
}

void led_set_mode(led_data_t *leds,led_mode_t mode)
{
	leds->mode = mode;
	led_update(leds);
}

static void led_set(led_state_t *led, bool state)
{
	if (!led->is_active_high) {
		state = !state;
	}

	HAL_GPIO_WritePin(led->port, led->pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint32_t led_set_sequence_step(led_data_t *leds, uint32_t step_num)
{
	const led_seq_step_t *step = &leds->sequence[step_num];
	leds->sequence_step = step_num;
	led_set(&leds->led_state[led_rx], step->state & 0x01);
	led_set(&leds->led_state[led_tx], step->state & 0x02);
	uint32_t delta = 10 * step->time_in_10ms;
	if (delta > INT32_MAX) {
		delta = INT32_MAX;  //clamp
	}
	leds->t_sequence_next = HAL_GetTick() + delta;
	return delta;
}

void led_run_sequence(led_data_t *leds, const led_seq_step_t *sequence, int32_t num_repeat)
{
	leds->mode = led_mode_sequence;
	leds->sequence = sequence;
	leds->seq_num_repeat = num_repeat;
	led_set_sequence_step(leds, 0);
	led_update(leds);
}

void led_indicate_trx(led_data_t *leds, led_num_t num) {
	leds->led_state[num].blink_request = 1;
}

static void led_trx_blinker(led_state_t *ledstate, uint32_t now) {
	if ( SEQ_ISPASSED(now, ledstate->on_until) &&
		 SEQ_ISPASSED(now, ledstate->off_until) ) {
		ledstate->off_until = now + 30;
		ledstate->on_until = now + 45;
	}
}

static void led_update_normal_mode(led_state_t *led, uint32_t now)
{
	if (led->blink_request) {
		led->blink_request = 0;
		led_trx_blinker(led, now);
	}

	led_set(led, SEQ_ISPASSED(now, led->off_until));
}

static void led_update_sequence(led_data_t *leds)
{
	if (leds->sequence == NULL) {
		return;
	}

	uint32_t now = HAL_GetTick();
	if (SEQ_ISPASSED(now, leds->t_sequence_next)) {

		uint32_t t = led_set_sequence_step(leds, ++leds->sequence_step);

		if (t > 0) { // the saga continues

			leds->t_sequence_next = now + t;

		} else { // end of sequence

			if (leds->seq_num_repeat != 0) {

				if (leds->seq_num_repeat > 0) {
					leds->seq_num_repeat--;
				}

				led_set_sequence_step(leds, 0);

			} else {
				leds->sequence = NULL;
			}
		}
	}
}

void led_update(led_data_t *leds)
{
	static uint32_t next_update = 0;
	uint32_t now = HAL_GetTick();
	if (!SEQ_ISPASSED(now, next_update)) {
		return;
	}
	next_update = now + LED_UPDATE_INTERVAL;

	switch (leds->mode) {

		case led_mode_off:
			led_set(&leds->led_state[led_rx], false);
			led_set(&leds->led_state[led_tx], false);
			break;

		case led_mode_normal:
			led_update_normal_mode(&leds->led_state[led_rx], now);
			led_update_normal_mode(&leds->led_state[led_tx], now);
			break;

		case led_mode_sequence:
			led_update_sequence(leds);
			break;

		default:
			assert_failed();
	}
}
