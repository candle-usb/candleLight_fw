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
#define USBD_CONFIGURATION_STRING_FS (uint8_t*) GIT_HASH
#define USBD_INTERFACE_STRING_FS	 (uint8_t*) "gs_usb interface"

#if defined(BOARD_candleLight)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "candleLight USB to CAN adapter"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "bytewerk"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "candleLight firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

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

#elif defined(BOARD_CANable_MKS)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "CANable-MKS gs_usb"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "makerbase"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "CANable-MKS firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port	  GPIOA
	#define LEDRX_Pin		  GPIO_PIN_1
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_OD
	#define LEDRX_Active_High 0

	#define LEDTX_GPIO_Port	  GPIOA
	#define LEDTX_Pin		  GPIO_PIN_0
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High 0

#elif defined(BOARD_CONVERTDEVICE_xCAN)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "ConvertDevice xCAN"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "ConvertDevice"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "ConvertDevice xCAN firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port	  GPIOA
	#define LEDRX_Pin		  GPIO_PIN_0
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High 0

	#define LEDTX_GPIO_Port	  GPIOA
	#define LEDTX_Pin		  GPIO_PIN_1
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High 0

#elif defined(BOARD_CONVERTDEVICE_xCANFD)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "ConvertDevice xCANFD"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "ConvertDevice"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "ConvertDevice xCANFD firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 64000000

	#define CAN_INTERFACE			 FDCAN1
	#define CAN_CLOCK_SPEED			 64000000
	#define NUM_CAN_CHANNEL			 1
	#define CANFD_SUPPORT

	#define LEDRX_GPIO_Port	  GPIOA
	#define LEDRX_Pin		  GPIO_PIN_0
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High 0

	#define LEDTX_GPIO_Port	  GPIOA
	#define LEDTX_Pin		  GPIO_PIN_1
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High 0

	#define USB_GPIO_Port	  GPIOA
	#define USB_Pin_DM		  GPIO_PIN_11
	#define USB_Pin_DP		  GPIO_PIN_12

#elif defined(BOARD_DSD_TECH_SH_C30A)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "SH-C30A USB to CAN adapter"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "DSD TECH"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "SH-C30A firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port	  GPIOB
	#define LEDRX_Pin		  GPIO_PIN_1
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High 0

	#define LEDTX_GPIO_Port	  GPIOB
	#define LEDTX_Pin		  GPIO_PIN_0
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High 0

#elif defined(BOARD_FYSETC_UCAN)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "UCAN USB to CAN adapter"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "FYSETC"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "UCAN firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port	  GPIOA
	#define LEDRX_Pin		  GPIO_PIN_1
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High 1

	#define LEDTX_GPIO_Port	  GPIOA
	#define LEDTX_Pin		  GPIO_PIN_0
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High 1

#elif defined(BOARD_cantact)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "cantact gs_usb"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "cantact.io"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "cantact firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port	  GPIOB
	#define LEDRX_Pin		  GPIO_PIN_0 /* green */
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High 1

	#define LEDTX_GPIO_Port	  GPIOB
	#define LEDTX_Pin		  GPIO_PIN_1 /* red */
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High 1

#elif defined(BOARD_canable)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "canable gs_usb"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "canable.io"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "canable firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port	  GPIOB
	#define LEDRX_Pin		  GPIO_PIN_0 /* green */
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High 1

	#define LEDTX_GPIO_Port	  GPIOB
	#define LEDTX_Pin		  GPIO_PIN_1 /* blue */
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High 0

#elif defined(BOARD_usb2can)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "USB2CAN RCA gs_usb"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "Roboter Club Aachen"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "usb2can firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LED4_GPIO_Port GPIOA
	#define LED4_Pin	   GPIO_PIN_0 /* white */
	#define LED4_Mode	   GPIO_MODE_OUTPUT_OD
	#define LED4_Active_Low

	#define LEDTX_GPIO_Port	  GPIOA
	#define LEDTX_Pin		  GPIO_PIN_1 /* blue */
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High 0

	#define LED3_GPIO_Port	  GPIOA
	#define LED3_Pin		  GPIO_PIN_2 /* red */
	#define LED3_Mode		  GPIO_MODE_OUTPUT_OD
	#define LED3_Active_Low

	#define LEDRX_GPIO_Port	  GPIOB
	#define LEDRX_Pin		  GPIO_PIN_3 /* green */
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_OD
	#define LEDRX_Active_High 0

#elif defined(BOARD_canalyze)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "CANAlyze gs_usb"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "STMicroelectronics"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "CANAlyze firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port	  GPIOB
	#define LEDRX_Pin		  GPIO_PIN_0 /* green */
	#define LEDRX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High 1

	#define LEDTX_GPIO_Port	  GPIOB
	#define LEDTX_Pin		  GPIO_PIN_1 /* red */
	#define LEDTX_Mode		  GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High 1

#elif defined(BOARD_cannette)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "cannette gs_usb"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "chacaltech"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "cannette firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 48000000

	#define CAN_INTERFACE			 CAN
	#define CAN_CLOCK_SPEED			 48000000
	#define NUM_CAN_CHANNEL			 1

// SILENT pin not connected

	#define LEDRX_GPIO_Port		 GPIOA
	#define LEDRX_Pin			 GPIO_PIN_9 /* RX: green */
	#define LEDRX_Mode			 GPIO_MODE_OUTPUT_OD
	#define LEDRX_Active_High	 0

	#define LEDTX_GPIO_Port		 GPIOA
	#define LEDTX_Pin			 GPIO_PIN_8 /* TX: red */
	#define LEDTX_Mode			 GPIO_MODE_OUTPUT_OD
	#define LEDTX_Active_High	 0

	#define nCANSTBY_Port		 GPIOC
	#define nCANSTBY_Pin		 GPIO_PIN_14 /* control xceiver standby, active low */
	#define nCANSTBY_Active_High 1

	#define nSI86EN_Port		 GPIOC
	#define nSI86EN_Pin			 GPIO_PIN_13 /* enable power to Si86xx isolater, active low */

	#define DCDCEN_Port			 GPIOC
	#define DCDCEN_Pin			 GPIO_PIN_15 /* activate DCDC converter, active high */

#elif defined(BOARD_budgetcan)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "budgetcan gs_usb"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "budgetcan"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "budgetcan firmware upgrade interface"

	#define TIM2_CLOCK_SPEED		 64000000

	#define CAN_INTERFACE			 FDCAN1
	#define CAN_INTERFACE2			 FDCAN2
	#define CAN_CLOCK_SPEED			 64000000
	#define NUM_CAN_CHANNEL			 2
	#define CANFD_SUPPORT

	#define nCANSTBY_Port		 GPIOA
	#define nCANSTBY_Pin		 GPIO_PIN_0    /* control xceiver standby, active low */
	#define nCANSTBY_Active_High 0

	#define LEDRX_GPIO_Port		 GPIOB
	#define LEDRX_Pin			 GPIO_PIN_4
	#define LEDRX_Mode			 GPIO_MODE_OUTPUT_PP
	#define LEDRX_Active_High	 1

	#define LEDTX_GPIO_Port		 GPIOB
	#define LEDTX_Pin			 GPIO_PIN_3
	#define LEDTX_Mode			 GPIO_MODE_OUTPUT_PP
	#define LEDTX_Active_High	 1

	#define USB_GPIO_Port		 GPIOA
	#define USB_Pin_DM			 GPIO_PIN_11
	#define USB_Pin_DP			 GPIO_PIN_12

	#define TERM_GPIO_Port		 GPIOA
	#define TERM_Pin			 GPIO_PIN_1
	#define TERM_Mode			 GPIO_MODE_OUTPUT_PP
	#define TERM_Active_High	 1

#elif defined(BOARD_STM32F4_DevBoard)
	#define USBD_PRODUCT_STRING_FS	 (uint8_t*) "STM32F4VE Dev Board"
	#define USBD_MANUFACTURER_STRING (uint8_t*) "misc"
	#define DFU_INTERFACE_STRING_FS	 (uint8_t*) "STM32F4VE firmware upgrade interface"

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

#else
	#error please define BOARD
#endif
