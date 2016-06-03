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

#include "led.h"
#include <string.h>
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"

void led_init(
	led_data_t *leds,
	void* led1_port, uint8_t led1_pin, bool led1_active_high,
	void* led2_port, uint8_t led2_pin, bool led2_active_high
) {
	memset(leds, 0, sizeof(led_data_t));
	leds->led_state[0].port = led1_port;
	leds->led_state[0].pin = led1_pin;
	leds->led_state[0].is_active_high = led1_active_high;
	leds->led_state[1].port = led2_port;
	leds->led_state[1].pin = led2_pin;
	leds->led_state[1].is_active_high = led2_active_high;
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
	led_seq_step_t *step = &leds->sequence[step_num];
	leds->sequence_step = step_num;
	led_set(&leds->led_state[0], step->state & 0x01);
	led_set(&leds->led_state[1], step->state & 0x02);
	leds->t_sequence_next = HAL_GetTick() + 10*step->time_in_10ms;
	return 10 * step->time_in_10ms;
}

void led_run_sequence(led_data_t *leds, led_seq_step_t *sequence, int32_t num_repeat)
{
	leds->last_mode = leds->mode;
	leds->mode = led_mode_sequence;
	leds->sequence = sequence;
	leds->seq_num_repeat = num_repeat;
	led_set_sequence_step(leds, 0);
	led_update(leds);
}

void led_indicate_trx(led_data_t *leds, led_num_t num)
{
	uint32_t now = HAL_GetTick();
	led_state_t *led = &leds->led_state[num];

	if ( (led->on_until < now) && (led->off_until < now) ) {
		led->off_until = now + 30;
		led->on_until = now + 45;
	}

	led_update(leds);
}

static void led_update_normal_mode(led_state_t *led)
{
	uint32_t now = HAL_GetTick();
	led_set(led, led->off_until < now);
}

static void led_update_sequence(led_data_t *leds)
{

	if (leds->sequence == NULL) {
		return;
	}

	uint32_t now = HAL_GetTick();
	if (now > leds->t_sequence_next) {

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
	switch (leds->mode) {

		case led_mode_off:
			led_set(&leds->led_state[0], false);
			led_set(&leds->led_state[1], false);
			break;

		case led_mode_normal:
			led_update_normal_mode(&leds->led_state[0]);
			led_update_normal_mode(&leds->led_state[1]);
			break;

		case led_mode_sequence:
			led_update_sequence(leds);
			break;

		default:
			led_set(&leds->led_state[0], false);
			led_set(&leds->led_state[1], true);
	}

}

