/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Hubert Denkmair
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct _copy_table_t
{
	uint32_t const* src;
	uint32_t* dest;
	uint32_t wlen;
} copy_table_t;

extern const copy_table_t __copy_table_start__;
extern const copy_table_t __copy_table_end__;

void __initialize_hardware_early(void);
void _start(void) __attribute__((noreturn));

void Reset_Handler(void)
{
	__initialize_hardware_early();

	for (copy_table_t const* table = &__copy_table_start__; table < &__copy_table_end__; ++table) {
		memcpy(table->dest, table->src, table->wlen);
	}

	_start();
}
