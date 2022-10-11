/*

The MIT License (MIT)

Copyright (c) 2016, 2019 Hubert Denkmair

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

#include <stdint.h>
#include <stdbool.h>
#include <cmsis_device.h>

void hex32(char *out, uint32_t val);

// ARM's
// "Application Note 321 ARM Cortex-M Programming Guide to Memory Barrier Instructions"
// (from https://developer.arm.com/documentation/dai0321/latest) says that
// the ISBs are actually necessary on Cortex-M0 to avoid a 2-instruction
// delay in the effect of enabling and disabling interrupts.
// That probably doesn't matter here, but it's hard to say what the compiler
// will put in those 2 instructions so it's safer to leave it. The DSB isn't
// necessary on Cortex-M0, but it's architecturally required so we'll
// include it to be safe.
//
// The "memory" and "cc" clobbers tell GCC to avoid moving memory loads or
// stores across the instructions. This is important when an interrupt and the
// code calling disable_irq/enable_irq share memory. The fact that these are
// non-inlined functions probably forces GCC to flush everything to memory
// anyways, but trying to outsmart the compiler is a bad strategy (you never
// know when somebody will turn on LTO or something).

static inline bool is_irq_enabled(void)
{
	return (__get_PRIMASK() & 1u) == 0u; // interrupts not prevented
}

static inline bool disable_irq(void)
{
	bool was_enabled = is_irq_enabled();
	__disable_irq();
	__ISB();
	return was_enabled;
}

static inline void enable_irq(void)
{
	__enable_irq();
	__ISB();
}

static inline void restore_irq(bool was_enabled)
{
	if (was_enabled) {
		enable_irq();
	}
}

/* Lightweight assert macro to replace standard assert()
 *
 * Standard assert() needs fprint, __FILE__ , __LINE__ string consts etc.
 *
 * stm32's library has "USE_FULL_ASSERT" to enable some checks, but they have
 * their own assert_param() macro that also passes in __FILE__, __LINE__ so
 * I'm not sure if (and how) it'll be possible to combine both.
 *
 * Interesting notes on
 * https://barrgroup.com/embedded-systems/how-to/define-assert-macro
 *
 */
#define assert_basic(exp)   \
	if (exp) {          \
	} else              \
	assert_failed()

/** halt / set core to debug state with BKPT.
 *
 * TODO : save extra info somewhere in RAM, and let WDT auto-reset ?
 */
void assert_failed(void);
