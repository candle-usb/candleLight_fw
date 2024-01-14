/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Phil Greenland
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

#include "can.h"
#include "device.h"
#include "hal_include.h"

void device_sysclock_config(void) {
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();

	/*
	 * Configure the main internal regulator output voltage selecting boost mode as system clock
	 * >150Mhz
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/*
	 * Initializes the RCC Oscillators according to the specified parameters in the
	 * RCC_OscInitTypeDef structure.
	 */
	const RCC_OscInitTypeDef RCC_OscInitStruct = {
		.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48,
		.HSIState = RCC_HSI_ON,
		.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
		.HSI48State = RCC_HSI48_ON,
		.PLL.PLLState = RCC_PLL_ON,
		.PLL.PLLSource = RCC_PLLSOURCE_HSI,
		.PLL.PLLM = RCC_PLLM_DIV1,
		.PLL.PLLN = 20,
		.PLL.PLLP = RCC_PLLP_DIV2,
		.PLL.PLLQ = RCC_PLLQ_DIV4,
		.PLL.PLLR = RCC_PLLR_DIV2,
	};
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Initializes the CPU, AHB and APB buses clocks */
	const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
		.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
					 RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
		.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
		.AHBCLKDivider = RCC_SYSCLK_DIV1,
		.APB1CLKDivider = RCC_HCLK_DIV1,
		.APB2CLKDivider = RCC_HCLK_DIV1,
	};
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);

	const RCC_PeriphCLKInitTypeDef PeriphClkInit = {
		.PeriphClockSelection = RCC_PERIPHCLK_USB,
		.UsbClockSelection = RCC_USBCLKSOURCE_HSI48,
	};
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
