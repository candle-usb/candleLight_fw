#pragma once

#ifndef BOARD
#define BOARD candleLight
#endif

#if BOARD == candleLight

	#define CAN_S_Pin GPIO_PIN_13
	#define CAN_S_GPIO_Port GPIOC

	#define LED1_GPIO_Port GPIOA
	#define LED1_Pin GPIO_PIN_0
	#define LED1_Mode GPIO_MODE_OUTPUT_OD
	#define LED1_Active_Low

	#define LED2_GPIO_Port GPIOA
	#define LED2_Pin GPIO_PIN_1
	#define LED2_Mode GPIO_MODE_OUTPUT_OD
	#define LED2_Active_Low

#elif BOARD == cantact

	// SILENT pin not connected

	#define LED1_GPIO_Port GPIOB
	#define LED1_Pin GPIO_PIN_0
	#define LED1_Mode GPIO_MODE_OUTPUT_PP

	#define LED2_GPIO_Port GPIOB
	#define LED2_Pin GPIO_PIN_1
	#define LED2_Mode GPIO_MODE_OUTPUT_PP

#endif

