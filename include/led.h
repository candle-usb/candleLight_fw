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

#include <stdint.h>
#include <stdbool.h>


#define LED_UPDATE_INTERVAL 10  // number of ticks from HAL_GetTick

typedef enum {
	led_mode_off,
	led_mode_normal,
	led_mode_warn,
	led_mode_error,
	led_mode_sequence
} led_mode_t;

typedef enum {
	led_rx = 0, //will also index into array led_state[]
	led_tx
} led_num_t;

typedef struct {
	uint8_t state;
	uint8_t time_in_10ms;
} led_seq_step_t;

typedef struct {
	void* port;
	uint16_t pin;
	bool is_active_high;

	bool blink_request;
	uint32_t on_until;
	uint32_t off_until;
} led_state_t;

typedef struct {
	led_mode_t mode;

	const led_seq_step_t *sequence;
	uint32_t sequence_step;
	uint32_t t_sequence_next;
	int32_t seq_num_repeat;

	led_state_t led_state[2];
} led_data_t;


void led_init(
	led_data_t *leds,
	void* led_rx_port, uint16_t led_rx_pin, bool led_rx_active_high,
	void* led_tx_port, uint16_t led_tx_pin, bool led_tx_active_high
	);
void led_set_mode(led_data_t *leds,led_mode_t mode);
void led_run_sequence(led_data_t *leds, const led_seq_step_t *sequence, int32_t num_repeat);
void led_indicate_trx(led_data_t *leds, led_num_t num);
void led_update(led_data_t *leds);
