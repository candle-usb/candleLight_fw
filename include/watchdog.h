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

#pragma once

#include <stdbool.h>

/*
 * The independent watchdog (CONFIG_WATCHDOG) and what happens on a fault.
 *
 * Without it, a fault or a failed assertion halts the core for good: the
 * adapter no longer answers even USB control requests and needs a power
 * cycle. With it, the device resets, drops off the bus and re-enumerates.
 */

/* Starts the watchdog (about 500 ms) if CONFIG_WATCHDOG is set. Once started
 * it can only be stopped by a reset. */
void watchdog_init(void);

/* Feeds the watchdog; call once per main-loop pass. */
void watchdog_feed(void);

/* True if a debugger has halting debug enabled (only detectable on cores with
 * software access to DHCSR; false otherwise). */
bool debugger_attached(void);

/* Stops in the debugger if one is attached, else resets the device. */
void fault_reset(void) __attribute__((noreturn));
