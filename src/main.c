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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "can.h"
#include "config.h"
#include "device.h"
#include "dfu.h"
#include "gpio.h"
#include "gs_usb.h"
#include "hal_include.h"
#include "led.h"
#include "timer.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_gs_can.h"
#include "util.h"

void HAL_MspInit(void);
static void SystemClock_Config(void);
static void send_to_host(void);

static USBD_GS_CAN_HandleTypeDef hGS_CAN;
static USBD_HandleTypeDef hUSB = {0};
static led_data_t hLED = {0};

int main(void)
{
	can_data_t *channel = &hGS_CAN.channels[0];

	HAL_Init();
	SystemClock_Config();

	gpio_init();

	led_init(&hLED, LEDRX_GPIO_Port, LEDRX_Pin, LEDRX_Active_High, LEDTX_GPIO_Port, LEDTX_Pin, LEDTX_Active_High);

	/* nice wake-up pattern */
	for (uint8_t i=0; i<10; i++)
	{
		HAL_GPIO_TogglePin(LEDRX_GPIO_Port, LEDRX_Pin);
		HAL_Delay(50);
		HAL_GPIO_TogglePin(LEDTX_GPIO_Port, LEDTX_Pin);
	}

	led_set_mode(&hLED, led_mode_off);
	timer_init();

	can_init(channel, CAN_INTERFACE);
	can_disable(channel);

	INIT_LIST_HEAD(&hGS_CAN.list_frame_pool);
	INIT_LIST_HEAD(&hGS_CAN.list_from_host);
	INIT_LIST_HEAD(&hGS_CAN.list_to_host);

	for (unsigned i = 0; i < ARRAY_SIZE(hGS_CAN.msgbuf); i++) {
		list_add_tail(&hGS_CAN.msgbuf[i].list, &hGS_CAN.list_frame_pool);
	}

	USBD_Init(&hUSB, (USBD_DescriptorsTypeDef*)&FS_Desc, DEVICE_FS);
	USBD_RegisterClass(&hUSB, &USBD_GS_CAN);
	USBD_GS_CAN_Init(&hGS_CAN, &hUSB, &hLED);
	USBD_Start(&hUSB);

#ifdef CAN_S_GPIO_Port
	HAL_GPIO_WritePin(CAN_S_GPIO_Port, CAN_S_Pin, GPIO_PIN_RESET);
#endif

	while (1) {
		struct gs_host_frame_object *frame_object;

		bool was_irq_enabled = disable_irq();
		frame_object = list_first_entry_or_null(&hGS_CAN.list_from_host,
												struct gs_host_frame_object,
												list);
		if (frame_object) { // send CAN message from host
			struct gs_host_frame *frame = &frame_object->frame;

			list_del(&frame_object->list);
			restore_irq(was_irq_enabled);

			if (can_send(channel, frame)) {
				// Echo sent frame back to host
				frame->flags = 0x0;
				frame->reserved = 0x0;
				frame->timestamp_us = timer_get();

				list_add_tail_locked(&frame_object->list, &hGS_CAN.list_to_host);

				led_indicate_trx(&hLED, led_tx);
			} else {
				list_add_locked(&frame_object->list, &hGS_CAN.list_from_host);
			}
		} else {
			restore_irq(was_irq_enabled);
		}

		if (USBD_GS_CAN_TxReady(&hUSB)) {
			send_to_host();
		}

		if (can_is_rx_pending(channel)) {
			bool was_irq_enabled = disable_irq();
			frame_object = list_first_entry_or_null(&hGS_CAN.list_frame_pool,
													struct gs_host_frame_object,
													list);
			if (frame_object) {
				struct gs_host_frame *frame = &frame_object->frame;

				list_del(&frame_object->list);
				restore_irq(was_irq_enabled);

				if (can_receive(channel, frame)) {

					frame->timestamp_us = timer_get();
					frame->echo_id = 0xFFFFFFFF; // not a echo frame
					frame->channel = 0;
					frame->flags = 0;
					frame->reserved = 0;

					list_add_tail_locked(&frame_object->list, &hGS_CAN.list_to_host);

					led_indicate_trx(&hLED, led_rx);
				} else {
					list_add_tail_locked(&frame_object->list, &hGS_CAN.list_frame_pool);
				}
			} else {
				restore_irq(was_irq_enabled);
			}
			// If there are frames to receive, don't report any error frames. The
			// best we can localize the errors to is "after the last successfully
			// received frame", so wait until we get there. LEC will hold some error
			// to report even if multiple pass by.
		} else {
			uint32_t can_err = can_get_error_status(channel);

			bool was_irq_enabled = disable_irq();
			frame_object = list_first_entry_or_null(&hGS_CAN.list_frame_pool,
													struct gs_host_frame_object,
													list);
			if (frame_object) {
				struct gs_host_frame *frame = &frame_object->frame;

				list_del(&frame_object->list);
				restore_irq(was_irq_enabled);

				frame->timestamp_us = timer_get();
				if (can_parse_error_status(channel, frame, can_err)) {
					list_add_tail_locked(&frame_object->list, &hGS_CAN.list_to_host);
				} else {
					list_add_tail_locked(&frame_object->list, &hGS_CAN.list_frame_pool);
				}
			} else {
				restore_irq(was_irq_enabled);
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
#elif defined(STM32G0)
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void SystemClock_Config(void)
{
	device_sysclock_config();
}

void send_to_host(void)
{
	struct gs_host_frame_object *frame_object;

	bool was_irq_enabled = disable_irq();
	frame_object = list_first_entry_or_null(&hGS_CAN.list_to_host,
											struct gs_host_frame_object,
											list);
	if (!frame_object) {
		restore_irq(was_irq_enabled);
		return;
	}
	list_del(&frame_object->list);
	restore_irq(was_irq_enabled);

	if (USBD_GS_CAN_SendFrame(&hUSB, &frame_object->frame) == USBD_OK) {
		list_add_tail_locked(&frame_object->list, &hGS_CAN.list_frame_pool);
	} else {
		list_add_locked(&frame_object->list, &hGS_CAN.list_to_host);
	}
}
