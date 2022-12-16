/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023, 2026 Pengutronix,
 *               Marc Kleine-Budde <kernel@pengutronix.de>
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
#include "gpio.h"
#include "usbd_gs_can.h"

/*
 * See UM2324 rev5, page 34
 *
 * LED      PA5
 * USB_P    PA12	CN10/12 (green)
 * USB_N    PA11	CN10/14 (white)
 * CAN1_RX  PD0		CN7/9
 * CAN1_TX  PD1		CN7/10
 * CAN2_RX  PC2		CN7/35
 * CAN2_TX  PC3		CN7/37
 *
 * Boot0	PA14	CN7/7
 * 3V3		-		CN7/16
 * 5V0		-		CN7/18
 * GND		-		CN7/20
 */

#define LED_GPIO_Port	GPIOA
#define LED_Pin			GPIO_PIN_5
#define LED_Mode		GPIO_MODE_OUTPUT_PP
#define LED_Active_High 1

static void nucleo_g0b1re_setup(USBD_GS_CAN_HandleTypeDef *hcan)
{
	UNUSED(hcan);

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_INIT_STATE(LED_Active_High));
	GPIO_InitTypeDef gpio_led = {
		.Pin = LED_Pin,
		.Mode = LED_Mode,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW,
	};
	HAL_GPIO_Init(LED_GPIO_Port, &gpio_led);

	/* FDCAN */
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {
		.PeriphClockSelection = RCC_PERIPHCLK_FDCAN,
		.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL,
	};
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	__HAL_RCC_FDCAN_CLK_ENABLE();

	/* FDCAN1_RX, FDCAN1_TX */
	GPIO_InitTypeDef gpio_fdcan1 = {
		.Pin = GPIO_PIN_0 | GPIO_PIN_1,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW,
		.Alternate = GPIO_AF3_FDCAN1,
	};
	HAL_GPIO_Init(GPIOD, &gpio_fdcan1);

	if (NUM_CAN_CHANNEL == 1)
		return;

	/* FDCAN2_RX, FDCAN2_TX */
	GPIO_InitTypeDef gpio_fdcan2 = {
		.Pin = GPIO_PIN_2 | GPIO_PIN_3,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW,
		.Alternate = GPIO_AF3_FDCAN2,
	};
	HAL_GPIO_Init(GPIOC, &gpio_fdcan2);
}

const struct board_config config = {
	.setup = nucleo_g0b1re_setup,
	.channel[0] = {
		.interface = FDCAN1,
		.leds = {
			[LED_RX] = {
				.port = LED_GPIO_Port,
				.pin = LED_Pin,
				.active_high = LED_Active_High,
			},
		},
	},
#if NUM_CAN_CHANNEL == 2
	.channel[1] = {
		.interface = FDCAN2,
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
#endif
};
