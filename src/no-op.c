/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024, 2026 Marc Kleine-Budde <kernel@pengutronix.de>
 * Copyright (c) fenugrec 2022
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

#include <compiler.h>

void __libc_fini_array(void)
{
}

void _close(void)
{
}

void _lseek(void)
{
}

void _read(void)
{
}

void _write(void)
{
}

int atexit(void __maybe_unused (*fn)(void))
{
	return 0;
}

void exit(int __maybe_unused code)
{
	__asm__ ("BKPT");
	while (1);
}
