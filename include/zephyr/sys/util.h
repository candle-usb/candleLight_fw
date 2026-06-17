/*
 * Copyright (c) 2011-2014, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Misc utilities
 *
 * Misc utilities usable by the kernel and application code.
 */

#ifndef ZEPHYR_INCLUDE_SYS_UTIL_H_
#define ZEPHYR_INCLUDE_SYS_UTIL_H_

#include <zephyr/sys/util_macro.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a contiguous bitmask starting at bit position @p l
 *        and ending at position @p h.
 */
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

/**
 * @brief Create a contiguous 64-bit bitmask starting at bit position @p l
 *        and ending at position @p h.
 */
#define GENMASK64(h, l) (((~0ULL) - (1ULL << (l)) + 1) & (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

/** @brief 0 if @p cond is true-ish; causes a compile error otherwise. */
#define ZERO_OR_COMPILE_ERROR(cond) ((int)sizeof(char[1 - (2 * !(cond))]) - 1)

#if defined(__cplusplus)

/* The built-in function used below for type checking in C is not
 * supported by GNU C++.
 */
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#else /* __cplusplus */

/**
 * @brief Zero if @p array has an array type, a compile error otherwise
 *
 * This macro is available only from C, not C++.
 */
#define IS_ARRAY(array)                                                                            \
		ZERO_OR_COMPILE_ERROR(                                                                     \
			!__builtin_types_compatible_p(__typeof__(array), __typeof__(&(array)[0])))

/**
 * @brief Number of elements in the given @p array
 *
 * In C++, due to language limitations, this will accept as @p array
 * any type that implements <tt>operator[]</tt>. The results may not be
 * particularly meaningful in this case.
 *
 * In C, passing a pointer as @p array causes a compile error.
 */
#define ARRAY_SIZE(array) ((size_t)(IS_ARRAY(array) + (sizeof(array) / sizeof((array)[0]))))

#endif /* __cplusplus */

#endif /* ZEPHYR_INCLUDE_SYS_UTIL_H_ */
