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

#include "config.h"
#include "hal_include.h"
#include "timer.h"

void timer_init(void)
{
	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM2->CR1 = 0;
	TIM2->CR2 = 0;
	TIM2->SMCR = 0;
	TIM2->DIER = 0;
	TIM2->CCMR1 = 0;
	TIM2->CCMR2 = 0;
	TIM2->CCER = 0;
	TIM2->PSC = (TIM2_CLOCK_SPEED / 1000000) - 1;   // run @1MHz = 1us
	TIM2->ARR = 0xFFFFFFFF;
	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2->EGR = TIM_EGR_UG;
}

uint32_t timer_get(void)
{
	return TIM2->CNT;
}
