#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
	while(42) {}
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void USB_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
