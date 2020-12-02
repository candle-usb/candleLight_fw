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

#include <util.h>

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

int disable_irq(void) {
	int primask;
	asm volatile("dsb sy\nisb sy\nmrs %0, PRIMASK\n"
				"cpsid i\ndsb sy\nisb sy\n" : "=r"(primask) :: "memory", "cc");
	return primask & 1;
}

void enable_irq(int primask) {
	if (!primask)
		asm volatile("dsb sy\nisb sy\ncpsie i\n"
				"dsb sy\nisb sy\n" ::: "memory", "cc");
}

void hex32(char *out, uint32_t val)
{
	char *p = out + 8;

	*p-- = 0;

	while (p >= out) {
		uint8_t nybble = val & 0x0F;

		if (nybble < 10)
			*p = '0' + nybble;
		else
			*p = 'A' + nybble - 10;

		val >>= 4;
		p--;
	}
}
