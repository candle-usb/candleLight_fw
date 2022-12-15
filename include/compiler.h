/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2016 Mellanox Technologies, Ltd.
 * Copyright (c) 2015 François Tigeot
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#ifndef _LINUXKPI_LINUX_COMPILER_H_
#define _LINUXKPI_LINUX_COMPILER_H_

#include "util_macro.h"

#ifndef __aligned
#define __aligned(x) __attribute__((__aligned__(x)))
#endif

#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

#if __has_attribute(__fallthrough__)
#define fallthrough __attribute__((__fallthrough__))
#else
#define fallthrough do {} while (0) /* fallthrough */
#endif

#define barrier()	   __asm__ __volatile__ ("" : : : "memory")

#define ACCESS_ONCE(x) (*(volatile __typeof(x) *)&(x))

#define WRITE_ONCE(x,v) \
	do { \
		barrier(); \
		ACCESS_ONCE(x) = (v); \
		barrier(); \
	} while (0)

#define READ_ONCE(x) \
	({ \
		__typeof(x) __var = ({ \
			barrier(); \
			ACCESS_ONCE(x); \
		}); \
		barrier(); \
		__var; \
	})

#define lockless_dereference(p) READ_ONCE(p)

#define __same_type(a, b)		__builtin_types_compatible_p(typeof(a), typeof(b))
#define __must_be_array(a)		__same_type(a, &(a)[0])

#define sizeof_field(_s, _m)	sizeof(((_s *)0)->_m)

#define container_of(ptr, type, member) \
	({ \
		__typeof(((type *)0)->member) *_p = (ptr); \
		(type *)((char *)_p - offsetof(type, member)); \
	})

#define struct_size(ptr, field, num) \
	(offsetof(__typeof(*(ptr)), field) + sizeof((ptr)->field[0]) * (num))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define DECLARE_FLEX_ARRAY(_t, _n) \
	struct { \
		struct { } __dummy_ ## _n; \
		_t _n[0]; \
	}

#define min(x, y)			((x) < (y) ? (x) : (y))
#define max(x, y)			((x) > (y) ? (x) : (y))

#define min3(a, b, c)		min(a, min(b, c))
#define max3(a, b, c)		max(a, max(b, c))

#define min4(a, b, c, d)	min(min(a, b), min(c, d))
#define max4(a, b, c, d)	max(max(a, b), max(b, d))

#define min5(a, b, c, d, e) min3(min(a, b), min(c, d), e)
#define max5(a, b, c, d, e) max3(max(a, b), max(b, d), e)

#endif /* _LINUXKPI_LINUX_COMPILER_H_ */
