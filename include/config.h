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

*
*
* Some board-specific defines here, such as :
* - USB strings
* - LED pin assignments and polarity
* - other special pins to control CAN transceivers.
*
* CAN_S_PIN: Some CAN transceivers (e.g. TJA1050) have a "Silent mode in which the transmitter is disabled";
* enabled with this 'S' pin. If undefined, the corresponding code will be disabled.
*
* TERM_Pin: Add support for an externally controlled terminating resistor
*
*/

#pragma once

#include "version.h"

#define CAN_QUEUE_SIZE				 (64 * NUM_CAN_CHANNEL)

#define USBD_VID					 0x1d50
#define USBD_PID_FS					 0x606f
#define USBD_LANGID_STRING			 1033
#define USBD_CONFIGURATION_STRING_FS GIT_HASH
#define USBD_INTERFACE_STRING_FS	 "gs_usb interface"

/*************** STM32F042 ***************/

#if defined(BOARD_canable)
	#define USBD_PRODUCT_STRING_FS	 "canable gs_usb"
	#define USBD_MANUFACTURER_STRING "canable.io"
	#define DFU_INTERFACE_STRING_FS	 "canable firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOB
	#define LEDRX_Pin				 GPIO_PIN_0 /* green */
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 1

	#define LEDTX_GPIO_Port			 GPIOB
	#define LEDTX_Pin				 GPIO_PIN_1 /* blue */
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 0

#elif defined(BOARD_canalyze)
	#define USBD_PRODUCT_STRING_FS	 "CANAlyze gs_usb"
	#define USBD_MANUFACTURER_STRING "STMicroelectronics"
	#define DFU_INTERFACE_STRING_FS	 "CANAlyze firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOB
	#define LEDRX_Pin				 GPIO_PIN_0 /* green */
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 1

	#define LEDTX_GPIO_Port			 GPIOB
	#define LEDTX_Pin				 GPIO_PIN_1 /* red */
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 1

#elif defined(BOARD_cannette)
	#define USBD_PRODUCT_STRING_FS	 "cannette gs_usb"
	#define USBD_MANUFACTURER_STRING "chacaltech"
	#define DFU_INTERFACE_STRING_FS	 "cannette firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOA
	#define LEDRX_Pin				 GPIO_PIN_9 /* RX: green */
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDRX_Active_High		 0

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_8 /* TX: red */
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High		 0

	#define nCANSTBY_Port			 GPIOC
	#define nCANSTBY_Pin			 GPIO_PIN_14 /* control xceiver standby, active low */
	#define nCANSTBY_Active_High	 1

	#define nSI86EN_Port			 GPIOC
	#define nSI86EN_Pin				 GPIO_PIN_13 /* enable power to Si86xx isolater, active low */

	#define DCDCEN_Port				 GPIOC
	#define DCDCEN_Pin				 GPIO_PIN_15 /* activate DCDC converter, active high */

#elif defined(BOARD_cantact)
	#define USBD_PRODUCT_STRING_FS	 "cantact gs_usb"
	#define USBD_MANUFACTURER_STRING "cantact.io"
	#define DFU_INTERFACE_STRING_FS	 "cantact firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOB
	#define LEDRX_Pin				 GPIO_PIN_0 /* green */
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 1

	#define LEDTX_GPIO_Port			 GPIOB
	#define LEDTX_Pin				 GPIO_PIN_1 /* red */
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 1

#elif defined(BOARD_usb2can)
	#define USBD_PRODUCT_STRING_FS	 "USB2CAN RCA gs_usb"
	#define USBD_MANUFACTURER_STRING "Roboter Club Aachen"
	#define DFU_INTERFACE_STRING_FS	 "usb2can firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_1 /* blue */
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High		 0

	#define LEDRX_GPIO_Port			 GPIOB
	#define LEDRX_Pin				 GPIO_PIN_3 /* green */
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDRX_Active_High		 0

/*************** STM32F072 ***************/

#elif defined(BOARD_CANable_MKS)
	#define USBD_PRODUCT_STRING_FS	 "CANable-MKS gs_usb"
	#define USBD_MANUFACTURER_STRING "makerbase"
	#define DFU_INTERFACE_STRING_FS	 "CANable-MKS firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOA
	#define LEDRX_Pin				 GPIO_PIN_1
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDRX_Active_High		 0

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_0
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High		 0

#elif defined(BOARD_CONVERTDEVICE_xCAN)
	#define USBD_PRODUCT_STRING_FS	 "ConvertDevice xCAN"
	#define USBD_MANUFACTURER_STRING "ConvertDevice"
	#define DFU_INTERFACE_STRING_FS	 "ConvertDevice xCAN firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOA
	#define LEDRX_Pin				 GPIO_PIN_0
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 0

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_1
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 0

#elif defined(BOARD_DSD_TECH_SH_C30A)
	#define USBD_PRODUCT_STRING_FS	 "SH-C30A USB to CAN adapter"
	#define USBD_MANUFACTURER_STRING "DSD TECH"
	#define DFU_INTERFACE_STRING_FS	 "SH-C30A firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOB
	#define LEDRX_Pin				 GPIO_PIN_1
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 0

	#define LEDTX_GPIO_Port			 GPIOB
	#define LEDTX_Pin				 GPIO_PIN_0
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 0

#elif defined(BOARD_FYSETC_UCAN)
	#define USBD_PRODUCT_STRING_FS	 "UCAN USB to CAN adapter"
	#define USBD_MANUFACTURER_STRING "FYSETC"
	#define DFU_INTERFACE_STRING_FS	 "UCAN firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define LEDRX_GPIO_Port			 GPIOA
	#define LEDRX_Pin				 GPIO_PIN_1
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 1

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_0
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 1

#elif defined(BOARD_candleLight)
	#define USBD_PRODUCT_STRING_FS	 "candleLight USB to CAN adapter"
	#define USBD_MANUFACTURER_STRING "bytewerk"
	#define DFU_INTERFACE_STRING_FS	 "candleLight firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CAN_FILTER		 1

	#define CAN_S_Pin				 GPIO_PIN_13
	#define CAN_S_GPIO_Port			 GPIOC

	#define LEDRX_Pin				 GPIO_PIN_0
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDRX_GPIO_Port			 GPIOA
	#define LEDRX_Active_High		 0

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_1
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High		 0

/*************** STM32F407 ***************/

#elif defined(BOARD_STM32F4_DevBoard)
	#define USBD_PRODUCT_STRING_FS	 "STM32F4VE Dev Board"
	#define USBD_MANUFACTURER_STRING "misc"
	#define DFU_INTERFACE_STRING_FS	 "STM32F4VE firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 96000000

	#define CAN_INTERFACE			 CAN1
	#define CAN_CLOCK_SPEED			 42000000
	#define NUM_CAN_CHANNEL			 1

	#define CAN_S_Pin				 GPIO_PIN_10
	#define CAN_S_GPIO_Port			 GPIOA

	#define LEDRX_GPIO_Port			 GPIOA
	#define LEDRX_Pin				 GPIO_PIN_6
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDRX_Active_High		 0

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_7
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High		 0

	#define USB_GPIO_Port			 GPIOA
	#define USB_Pin_DM				 GPIO_PIN_11
	#define USB_Pin_DP				 GPIO_PIN_12

	#define TERM_GPIO_Port			 GPIOB
	#define TERM_Pin				 GPIO_PIN_3
	#define TERM_Mode				 GPIO_MODE_OUTPUT_PP
	#define TERM_Active_High		 1

/*************** STM32G0B1 ***************/

#elif defined(BOARD_CONVERTDEVICE_xCANFD)
	#define USBD_PRODUCT_STRING_FS	 "ConvertDevice xCANFD"
	#define USBD_MANUFACTURER_STRING "ConvertDevice"
	#define DFU_INTERFACE_STRING_FS	 "ConvertDevice xCANFD firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 64000000

	#define CAN_INTERFACE			 FDCAN1
	#define CAN_CLOCK_SPEED			 64000000
	#define NUM_CAN_CHANNEL			 1
	#define CONFIG_CANFD			 1

	#define LEDRX_GPIO_Port			 GPIOA
	#define LEDRX_Pin				 GPIO_PIN_0
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 0

	#define LEDTX_GPIO_Port			 GPIOA
	#define LEDTX_Pin				 GPIO_PIN_1
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 0

	#define USB_GPIO_Port			 GPIOA
	#define USB_Pin_DM				 GPIO_PIN_11
	#define USB_Pin_DP				 GPIO_PIN_12

#elif defined(BOARD_budgetcan)
	#define USBD_PRODUCT_STRING_FS	 "budgetcan gs_usb"
	#define USBD_MANUFACTURER_STRING "budgetcan"
	#define DFU_INTERFACE_STRING_FS	 "budgetcan firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 64000000

	#define CAN_INTERFACE			 FDCAN1
	#define CAN_INTERFACE2			 FDCAN2
	#define CAN_CLOCK_SPEED			 64000000
	#define NUM_CAN_CHANNEL			 2
	#define CONFIG_CANFD			 1

	#define nCANSTBY_Port			 GPIOA
	#define nCANSTBY_Pin			 GPIO_PIN_0 /* control xceiver standby, active low */
	#define nCANSTBY_Active_High	 0

	#define LEDRX_GPIO_Port			 GPIOB
	#define LEDRX_Pin				 GPIO_PIN_4
	#define LEDRX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High		 1

	#define LEDTX_GPIO_Port			 GPIOB
	#define LEDTX_Pin				 GPIO_PIN_3
	#define LEDTX_Mode				 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High		 1

	#define USB_GPIO_Port			 GPIOA
	#define USB_Pin_DM				 GPIO_PIN_11
	#define USB_Pin_DP				 GPIO_PIN_12

	#define TERM_GPIO_Port			 GPIOA
	#define TERM_Pin				 GPIO_PIN_1
	#define TERM_Mode				 GPIO_MODE_OUTPUT_PP
	#define TERM_Active_High		 1

#else
	#error please define BOARD
#endif
