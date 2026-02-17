/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024,
 *               Romain Reignier <romain@reignier.sh>
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

#include "board.h"
#include "config.h"
#include "device.h"
#include "gpio.h"
#include "usbd_gs_can.h"

/*
 * LED_RX	 PA0
 * LED_TX	 PA1
 * LED_READY PA2
 * USB_P	 PA12
 * USB_N	 PA11
 * CAN1_RX	 PB8
 * CAN1_TX	 PB9
 */

#define LEDRX_GPIO_Port		 GPIOA
#define LEDRX_Pin			 GPIO_PIN_0
#define LEDRX_Mode			 GPIO_MODE_OUTPUT_PP
#define LEDRX_Active_High	 1

#define LEDTX_GPIO_Port		 GPIOA
#define LEDTX_Pin			 GPIO_PIN_1
#define LEDTX_Mode			 GPIO_MODE_OUTPUT_PP
#define LEDTX_Active_High	 1

#define LEDREADY_GPIO_Port	 GPIOA
#define LEDREADY_Pin		 GPIO_PIN_2
#define LEDREADY_Mode		 GPIO_MODE_OUTPUT_PP
#define LEDREADY_Active_High 1

static void candlelightfd_setup(USBD_GS_CAN_HandleTypeDef *hcan)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	UNUSED(hcan);

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* LEDs */
	HAL_GPIO_WritePin(LEDRX_GPIO_Port, LEDRX_Pin, GPIO_INIT_STATE(LEDRX_Active_High));
	GPIO_InitStruct.Pin = LEDRX_Pin;
	GPIO_InitStruct.Mode = LEDRX_Mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEDRX_GPIO_Port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(LEDTX_GPIO_Port, LEDTX_Pin, GPIO_INIT_STATE(LEDTX_Active_High));
	GPIO_InitStruct.Pin = LEDTX_Pin;
	GPIO_InitStruct.Mode = LEDTX_Mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEDTX_GPIO_Port, &GPIO_InitStruct);

	/* Switch ON the LED READY on start, not driven by the firmware */
	HAL_GPIO_WritePin(LEDREADY_GPIO_Port, LEDREADY_Pin, GPIO_INIT_STATE(LEDREADY_Active_High));
	GPIO_InitStruct.Pin = LEDREADY_Pin;
	GPIO_InitStruct.Mode = LEDREADY_Mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEDREADY_GPIO_Port, &GPIO_InitStruct);

	/* FDCAN */
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {
		.PeriphClockSelection = RCC_PERIPHCLK_FDCAN,
		.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL,
	};

	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	__HAL_RCC_FDCAN_CLK_ENABLE();

	/* FDCAN1_RX, FDCAN1_TX */
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF3_FDCAN1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

const struct board_config config = {
	.setup = candlelightfd_setup,
	.channel[0] = {
		.interface = FDCAN1,
		.leds = {
			[LED_RX] = {
				.port = LEDRX_GPIO_Port,
				.pin = LEDRX_Pin,
				.active_high = LEDRX_Active_High,
			},
			[LED_TX] = {
				.port = LEDTX_GPIO_Port,
				.pin = LEDTX_Pin,
				.active_high = LEDTX_Active_High,
			},
		},
	},
};
