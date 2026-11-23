/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Elliot Woods
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

#include "compiler.h"
#include "config.h"
#include "hal_include.h"
#include "util.h"
#include "watchdog.h"

#define IWDG_KEY_ENABLE		  0xCCCC
#define IWDG_KEY_WRITE_ACCESS 0x5555
#define IWDG_KEY_RELOAD		  0xAAAA

/* LSI is nominally 32-40 kHz depending on the family (30-50 kHz with tolerance):
 * /16 and 1250 counts give 0.5 s nominal at 40 kHz. */
#define IWDG_PRESCALER_DIV16 0x2
#define IWDG_RELOAD_COUNTS	 1250

void watchdog_init(void)
{
	if (!IS_ENABLED(CONFIG_WATCHDOG))
		return;

	IWDG->KR = IWDG_KEY_ENABLE;
	IWDG->KR = IWDG_KEY_WRITE_ACCESS;
	IWDG->PR = IWDG_PRESCALER_DIV16;
	IWDG->RLR = IWDG_RELOAD_COUNTS;
	while (IWDG->SR) {
	}
	IWDG->KR = IWDG_KEY_RELOAD;
}

void watchdog_feed(void)
{
	if (IS_ENABLED(CONFIG_WATCHDOG))
		IWDG->KR = IWDG_KEY_RELOAD;
}

bool debugger_attached(void)
{
#if defined(CoreDebug_DHCSR_C_DEBUGEN_Msk)
	return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0;
#else
	/* ARMv6-M (Cortex-M0): DHCSR is not accessible from software. */
	return false;
#endif
}

void fault_reset(void)
{
	if (debugger_attached())
		__BKPT(0);

	NVIC_SystemReset();
	while (1) {
	}
}
