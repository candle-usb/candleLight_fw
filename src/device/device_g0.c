/*
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Ryan Edwards
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
#include "config.h"
#include "device.h"
#include "hal_include.h"

static void gpio_clk_enable(GPIO_TypeDef *port)
{
	if (port == GPIOA) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
	} else if (port == GPIOB) {
		__HAL_RCC_GPIOB_CLK_ENABLE();
	} else if (port == GPIOC) {
		__HAL_RCC_GPIOC_CLK_ENABLE();
#if defined(GPIOD)
	} else if (port == GPIOD) {
		__HAL_RCC_GPIOD_CLK_ENABLE();
#endif
	}
}

void device_can_init(can_data_t *channel, const struct board_channel_config *channel_config)
{
	GPIO_TypeDef *port = CAN_GPIO_Port;
	uint16_t pins = CAN_RX_Pin | CAN_TX_Pin;
	uint32_t alternate = CAN_GPIO_AF;

#if (NUM_CAN_CHANNEL > 1)
	if (channel_config->interface == CAN_INTERFACE2) {
		port = CAN2_GPIO_Port;
		pins = CAN2_RX_Pin | CAN2_TX_Pin;
		alternate = CAN2_GPIO_AF;
	}
#endif

	RCC_PeriphCLKInitTypeDef PeriphClkInit = {
		.PeriphClockSelection = RCC_PERIPHCLK_FDCAN,
		.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL,
	};
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	__HAL_RCC_FDCAN_CLK_ENABLE();
	gpio_clk_enable(port);

	GPIO_InitTypeDef itd = {
		.Pin = pins,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_VERY_HIGH,
		.Alternate = alternate,
	};
	HAL_GPIO_Init(port, &itd);

	channel->hfdcan.Instance = channel_config->interface;
}

void device_sysclock_config(void)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();

	/* Configure the main internal regulator output voltage	*/
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/*
	 * Initializes the RCC Oscillators according to the specified parameters in the
	 * RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitTypeDef RCC_OscInitStruct = {
#if defined(CONFIG_HSE_OSC_SPEED)
		.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48,
		.HSEState = RCC_HSE_ON,
		.HSI48State = RCC_HSI48_ON,
		.PLL.PLLSource = RCC_PLLSOURCE_HSE,
		.PLL.PLLN = 320000000 / CONFIG_HSE_OSC_SPEED,
#else
		.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48,
		.HSIState = RCC_HSI_ON,
		.HSI48State = RCC_HSI48_ON,
		.HSIDiv = RCC_HSI_DIV1,
		.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
		.PLL.PLLSource = RCC_PLLSOURCE_HSI,
		.PLL.PLLN = 20,
#endif
		.PLL.PLLState = RCC_PLL_ON,
		.PLL.PLLM = RCC_PLLM_DIV1,
		.PLL.PLLP = RCC_PLLP_DIV2,
		.PLL.PLLQ = RCC_PLLQ_DIV8,
		.PLL.PLLR = RCC_PLLR_DIV5,
	};
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Initializes the CPU, AHB and APB buses clocks */
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {
		.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1,
		.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
		.AHBCLKDivider = RCC_SYSCLK_DIV1,
		.APB1CLKDivider = RCC_HCLK_DIV1,
	};
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

	RCC_PeriphCLKInitTypeDef PeriphClkInit = {
		.PeriphClockSelection = RCC_PERIPHCLK_USB,
		.UsbClockSelection = RCC_USBCLKSOURCE_HSI48,
	};
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
