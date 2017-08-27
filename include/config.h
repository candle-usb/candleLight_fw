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

#define CAN_QUEUE_SIZE 64

#define USBD_VID                     0x1d50
#define USBD_PID_FS                  0x606f
#define USBD_LANGID_STRING           1033
#define USBD_MANUFACTURER_STRING     (uint8_t*) "bytewerk"
#define USBD_CONFIGURATION_STRING_FS (uint8_t*) "gs_usb config"
#define USBD_INTERFACE_STRING_FS     (uint8_t*) "gs_usb interface"
#define DFU_INTERFACE_STRING_FS      (uint8_t*) "candleLight firmware upgrade interface"

#define BOARD_candleLight 1
#define BOARD_cantact     2

#if BOARD == BOARD_candleLight
	#define USBD_PRODUCT_STRING_FS (uint8_t*) "candleLight USB to CAN adapter"

	#define CAN_S_Pin GPIO_PIN_13
	#define CAN_S_GPIO_Port GPIOC

	#define LED1_Pin GPIO_PIN_0
	#define LED1_Mode GPIO_MODE_OUTPUT_OD
	#define LED1_GPIO_Port GPIOA
	#define LED1_Active_Low

	#define LED2_GPIO_Port GPIOA
	#define LED2_Pin GPIO_PIN_1
	#define LED2_Mode GPIO_MODE_OUTPUT_OD
	#define LED2_Active_Low

#elif BOARD == BOARD_cantact
	#define USBD_PRODUCT_STRING_FS (uint8_t*) "cantact gs_usb"

	// SILENT pin not connected

	#define LED1_GPIO_Port GPIOB
	#define LED1_Pin GPIO_PIN_0
	#define LED1_Mode GPIO_MODE_OUTPUT_PP

	#define LED2_GPIO_Port GPIOB
	#define LED2_Pin GPIO_PIN_1
	#define LED2_Mode GPIO_MODE_OUTPUT_PP

#else
	#error please define BOARD
#endif
