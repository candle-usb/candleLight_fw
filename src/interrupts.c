/*

The MIT License (MIT)

Copyright (c) 2019 Hubert Denkmair

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

#include <stdint.h>
#include <stddef.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx.h>

void NMI_Handler(void)
{
    __asm__("BKPT");
    while (1);
}

void HardFault_Handler(void)
{
    __asm__("BKPT");
    while (1);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

extern PCD_HandleTypeDef hpcd_USB_FS;
void USB_Handler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

void Default_Handler()
{
    __asm__("BKPT");
    while (1);
}

extern void Reset_Handler();

typedef void(*pFunc)();
extern uint32_t __StackTop;

__attribute__((used, section(".vectors"))) 
const pFunc InterruptVectorTable[48] =
{
    (pFunc)(&__StackTop), // initial stack pointer
    Reset_Handler, // reset handler
    NMI_Handler, // -14: NMI
    HardFault_Handler, // -13: HardFault
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // -5: SVCall
    0,
    0,
    0, // -2: PendSV
    SysTick_Handler, // -1: SysTick

    0, // int 0: WWDG
    0, // int 1: PVD
    0, // int 2: RTC
    0, // int 3: FLASH
    0, // int 4: RCC_CRS
    0, // int 5: EXTI0_1
    0, // int 6: EXTI2_3
    0, // int 7: EXTI4_15
    0, // int 8: TSC
    0, // int 9: DMA_CH1
    0, // int 10: DMA_CH2_3
    0, // int 11: DMA_CH4_5_6_7
    0, // int 12: ADC_COMP
    0, // int 13: TIM1_BRK_UP_TRG_COM
    0, // int 14: TIM1_CC
    0, // int 15: TIM2
    0, // int 16: TIM3
    0, // int 17: TIM6_DAC
    0, // int 18: TIM7
    0, // int 19: TIM14
    0, // int 20: TIM15
    0, // int 21: TIM16
    0, // int 22: TIM17
    0, // int 23: I2C1
    0, // int 24: I2C2
    0, // int 25: SPI1
    0, // int 26: SPI2
    0, // int 27: USART1
    0, // int 28: USART2
    0, // int 29: USART3_4
    0, // int 30: CEC_CAN
    USB_Handler, // int 31: USB
};
