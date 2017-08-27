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

inline int disable_irq(void) {
    int primask;
    asm volatile("mrs %0, PRIMASK\n"
                 "cpsid i\n" : "=r"(primask));
    return primask & 1;
}

inline void enable_irq(int primask) {
    if (primask)
        asm volatile("cpsie i\n");
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
