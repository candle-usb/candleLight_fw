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

#include "gpio.h"
#include "hal_include.h"

#ifdef TERM_Pin
static int term_state = 0;

enum gs_can_termination_state get_term(unsigned int channel)
{
	if (term_state & (1 << channel)) {
		return GS_CAN_TERMINATION_STATE_ON;
	} else {
		return GS_CAN_TERMINATION_STATE_OFF;
	}
}

enum gs_can_termination_state set_term(unsigned int channel, enum gs_can_termination_state state)
{
	if (state == GS_CAN_TERMINATION_STATE_ON) {
		term_state |= 1 << channel;
	} else {
		term_state &= ~(1 << channel);
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

static inline void gpio_init_term(void)
{
	HAL_GPIO_WritePin(TERM_GPIO_Port, TERM_Pin, GPIO_INIT_STATE(TERM_Active_High));

	GPIO_InitTypeDef GPIO_InitStruct = {
		.Pin = TERM_Pin,
		.Mode = TERM_Mode,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW,
		.Alternate = 0
	};
	HAL_GPIO_Init(TERM_GPIO_Port, &GPIO_InitStruct);
}

#else

static inline void gpio_init_term(void)
{
}

#endif

// must run before can_init
void gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
#if defined(STM32F4)
	__HAL_RCC_GPIOD_CLK_ENABLE();
#endif

#ifdef CAN_S_Pin
	HAL_GPIO_WritePin(CAN_S_GPIO_Port, CAN_S_Pin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = CAN_S_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CAN_S_GPIO_Port, &GPIO_InitStruct);
#endif

#ifdef LEDRX_Pin
	HAL_GPIO_WritePin(LEDRX_GPIO_Port, LEDRX_Pin, GPIO_INIT_STATE(LEDRX_Active_High));
	GPIO_InitStruct.Pin = LEDRX_Pin;
	GPIO_InitStruct.Mode = LEDRX_Mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEDRX_GPIO_Port, &GPIO_InitStruct);
#endif

#ifdef LEDTX_Pin
	HAL_GPIO_WritePin(LEDTX_GPIO_Port, LEDTX_Pin, GPIO_INIT_STATE(LEDTX_Active_High));
	GPIO_InitStruct.Pin = LEDTX_Pin;
	GPIO_InitStruct.Mode = LEDTX_Mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEDTX_GPIO_Port, &GPIO_InitStruct);
#endif

#ifdef nCANSTBY_Pin
	HAL_GPIO_WritePin(nCANSTBY_Port, nCANSTBY_Pin, GPIO_INIT_STATE(nCANSTBY_Active_High));
	GPIO_InitStruct.Pin = nCANSTBY_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(nCANSTBY_Port, &GPIO_InitStruct); //xceiver standby.
#endif

#ifdef DCDCEN_Pin
	HAL_GPIO_WritePin(DCDCEN_Port, DCDCEN_Pin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = DCDCEN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DCDCEN_Port, &GPIO_InitStruct);   //start DCDC (TODO : wait until enumerated)
#endif

#ifdef nSI86EN_Pin
	HAL_GPIO_WritePin(nSI86EN_Port, nSI86EN_Pin, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = nSI86EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(nSI86EN_Port, &GPIO_InitStruct);  //enable si86
#endif


#if defined(BOARD_STM32F4_DevBoard)
	// initialize USB pins
	GPIO_InitStruct.Pin = USB_Pin_DM | USB_Pin_DP;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
	HAL_GPIO_Init(USB_GPIO_Port, &GPIO_InitStruct);
#endif

	gpio_init_term();
}
