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

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "hal_include.h"
#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_core.h"
#include "usbd_gs_can.h"
#include "gpio.h"
#include "queue.h"
#include "gs_usb.h"
#include "can.h"
#include "led.h"
#include "dfu.h"
#include "timer.h"
#include "flash.h"
#include "util.h"

void HAL_MspInit(void);
void SystemClock_Config(void);
static bool send_to_host_or_enqueue(struct gs_host_frame *frame);
static void send_to_host();

can_data_t hCAN;
USBD_HandleTypeDef hUSB;
led_data_t hLED;

queue_t *q_frame_pool;
queue_t *q_from_host;
queue_t *q_to_host;

uint32_t received_count=0;


int main(void)
{
	uint32_t last_can_error_status = 0;

	HAL_Init();
	SystemClock_Config();

	flash_load();

	gpio_init();

	led_init(&hLED, LED1_GPIO_Port, LED1_Pin, LED1_Active_High, LED2_GPIO_Port, LED2_Pin, LED2_Active_High);

	/* nice wake-up pattern */
	for(uint8_t i=0; i<10; i++)
	{
		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		HAL_Delay(50);
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	}

	led_set_mode(&hLED, led_mode_off);
	timer_init();

	can_init(&hCAN, CAN_INTERFACE);
	can_disable(&hCAN);


	q_frame_pool = queue_create(CAN_QUEUE_SIZE);
	q_from_host  = queue_create(CAN_QUEUE_SIZE);
	q_to_host	 = queue_create(CAN_QUEUE_SIZE);
	assert_basic(q_frame_pool && q_from_host && q_to_host);

	struct gs_host_frame *msgbuf = calloc(CAN_QUEUE_SIZE, sizeof(struct gs_host_frame));
	assert_basic(msgbuf);

	for (unsigned i=0; i<CAN_QUEUE_SIZE; i++) {
		queue_push_back(q_frame_pool, &msgbuf[i]);
	}

	USBD_Init(&hUSB, (USBD_DescriptorsTypeDef*)&FS_Desc, DEVICE_FS);
	USBD_RegisterClass(&hUSB, &USBD_GS_CAN);
	USBD_GS_CAN_Init(&hUSB, q_frame_pool, q_from_host, &hLED);
	USBD_GS_CAN_SetChannel(&hUSB, 0, &hCAN);
	USBD_Start(&hUSB);

#ifdef CAN_S_GPIO_Port
	HAL_GPIO_WritePin(CAN_S_GPIO_Port, CAN_S_Pin, GPIO_PIN_RESET);
#endif

	while (1) {
		struct gs_host_frame *frame = queue_pop_front(q_from_host);
		if (frame != 0) { // send can message from host
			if (can_send(&hCAN, frame)) {
				// Echo sent frame back to host
				frame->flags = 0x0;
				frame->reserved = 0x0;
				frame->timestamp_us = timer_get();
				send_to_host_or_enqueue(frame);

				led_indicate_trx(&hLED, led_2);
			} else {
				queue_push_front(q_from_host, frame); // retry later
			}
		}

		if (USBD_GS_CAN_TxReady(&hUSB)) {
			send_to_host();
		}

		if (can_is_rx_pending(&hCAN)) {
			struct gs_host_frame *frame = queue_pop_front(q_frame_pool);
			if (frame != 0)
			{
				if (can_receive(&hCAN, frame)) {
					received_count++;

					frame->timestamp_us = timer_get();
					frame->echo_id = 0xFFFFFFFF; // not a echo frame
					frame->channel = 0;
					frame->flags = 0;
					frame->reserved = 0;

					send_to_host_or_enqueue(frame);

					led_indicate_trx(&hLED, led_1);
				}
				else
				{
					queue_push_back(q_frame_pool, frame);
				}
			}
			// If there are frames to receive, don't report any error frames. The
			// best we can localize the errors to is "after the last successfully
			// received frame", so wait until we get there. LEC will hold some error
			// to report even if multiple pass by.
		} else {
			uint32_t can_err = can_get_error_status(&hCAN);
			struct gs_host_frame *frame = queue_pop_front(q_frame_pool);
			if (frame != 0) {
				frame->timestamp_us = timer_get();
				if (can_parse_error_status(can_err, last_can_error_status, &hCAN, frame)) {
					send_to_host_or_enqueue(frame);
					last_can_error_status = can_err;
				} else {
					queue_push_back(q_frame_pool, frame);
				}
			}
		}

		led_update(&hLED);

		if (USBD_GS_CAN_DfuDetachRequested(&hUSB)) {
			dfu_run_bootloader();
		}

	}
}

void HAL_MspInit(void)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
#if defined(STM32F4)
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

#if defined(STM32F0)
	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	RCC_CRSInitTypeDef       RCC_CRSInitStruct;

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
								  RCC_CLOCKTYPE_SYSCLK |
								  RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	__HAL_RCC_CRS_CLK_ENABLE();
	RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
	RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
	RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
	RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
	RCC_CRSInitStruct.ErrorLimitValue = 34;
	RCC_CRSInitStruct.HSI48CalibrationValue = 32;
	HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
#elif defined(STM32F4)
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
								  RCC_CLOCKTYPE_SYSCLK |
								  RCC_CLOCKTYPE_PCLK1 |
								  RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
#endif

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

bool send_to_host_or_enqueue(struct gs_host_frame *frame)
{
	queue_push_back(q_to_host, frame);
	return true;
}

void send_to_host()
{
	struct gs_host_frame *frame = queue_pop_front(q_to_host);

	if(!frame)
	  return;

	if (USBD_GS_CAN_SendFrame(&hUSB, frame) == USBD_OK) {
		queue_push_back(q_frame_pool, frame);
	} else {
		queue_push_front(q_to_host, frame);
	}
}

