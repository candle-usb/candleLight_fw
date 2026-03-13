/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Hubert Denkmair
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
#include "can.h"
#include "device.h"
#include "hal_include.h"

void device_can_init(can_data_t *channel, const struct board_channel_config *channel_config)
{
	__HAL_RCC_CAN1_CLK_ENABLE();

	GPIO_InitTypeDef itd = {
		.Pin = GPIO_PIN_8 | GPIO_PIN_9,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH,
		.Alternate = GPIO_AF4_CAN,
	};
	HAL_GPIO_Init(GPIOB, &itd);

	channel->instance = channel_config->interface;
}

void device_sysclock_config(void) {
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();

	const RCC_OscInitTypeDef RCC_OscInitStruct = {
#if defined(CONFIG_HSE_OSC_SPEED)
		.OscillatorType = RCC_OSCILLATORTYPE_HSE,
		.HSEState = RCC_HSE_ON,
		.PLL.PLLState = RCC_PLL_ON,
		.PLL.PLLSource = RCC_PLLSOURCE_HSE,
		.PLL.PREDIV = RCC_PREDIV_DIV1,
		.PLL.PLLMUL = FIELD_PREP(RCC_CFGR_PLLMUL,
								 (48000000 / CONFIG_HSE_OSC_SPEED) - 2),
#else
		.OscillatorType = RCC_OSCILLATORTYPE_HSI48,
		.HSI48State = RCC_HSI48_ON,
		.PLL.PLLState = RCC_PLL_NONE,
#endif
	};
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
		.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1,
#if defined(CONFIG_HSE_OSC_SPEED)
		.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
#else
		.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48,
#endif
		.AHBCLKDivider = RCC_SYSCLK_DIV1,
		.APB1CLKDivider = RCC_HCLK_DIV1,
	};
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

	RCC_PeriphCLKInitTypeDef PeriphClkInit = {
		.PeriphClockSelection = RCC_PERIPHCLK_USB,
#if defined(CONFIG_HSE_OSC_SPEED)
		.UsbClockSelection = RCC_USBCLKSOURCE_PLLCLK,
#else
		.UsbClockSelection = RCC_USBCLKSOURCE_HSI48,
#endif
	};
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

#if !defined(CONFIG_HSE_OSC_SPEED)
	__HAL_RCC_CRS_CLK_ENABLE();
	RCC_CRSInitTypeDef RCC_CRSInitStruct = {
		.Prescaler = RCC_CRS_SYNC_DIV1,
		.Source = RCC_CRS_SYNC_SOURCE_USB,
		.Polarity = RCC_CRS_SYNC_POLARITY_RISING,
		.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000, 1000),
		.ErrorLimitValue = 34,
		.HSI48CalibrationValue = 32,
	};
	HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
#endif

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
