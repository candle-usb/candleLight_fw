/*
 * Copyright (c) 2011-2014, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_SYS_UTIL_MACROS_H_
#define ZEPHYR_INCLUDE_SYS_UTIL_MACROS_H_

/*
 * Most of the eldritch implementation details for all the macrobatics
 * below (APIs like IS_ENABLED(), COND_CODE_1(), etc.) are hidden away
 * in this file.
 */
#include "util_internal.h"

/**
 * @brief Check for macro definition in compiler-visible expressions
 *
 * This trick was pioneered in Linux as the config_enabled() macro. It
 * has the effect of taking a macro value that may be defined to "1"
 * or may not be defined at all and turning it into a literal
 * expression that can be handled by the C compiler instead of just
 * the preprocessor. It is often used with a @p CONFIG_FOO macro which
 * may be defined to 1 via Kconfig, or left undefined.
 *
 * That is, it works similarly to <tt>\#if defined(CONFIG_FOO)</tt>
 * except that its expansion is a C expression. Thus, much <tt>\#ifdef</tt>
 * usage can be replaced with equivalents like:
 *
 *     if (IS_ENABLED(CONFIG_FOO)) {
 *             do_something_with_foo
 *     }
 *
 * This is cleaner since the compiler can generate errors and warnings
 * for @p do_something_with_foo even when @p CONFIG_FOO is undefined.
 *
 * @param config_macro Macro to check
 * @return 1 if @p config_macro is defined to 1, 0 otherwise (including
 *         if @p config_macro is not defined)
 */
#define IS_ENABLED(config_macro) Z_IS_ENABLED1(config_macro)
/* INTERNAL: the first pass above is just to expand any existing
 * macros, we need the macro value to be e.g. a literal "1" at
 * expansion time in the next macro, not "(1)", etc... Standard
 * recursive expansion does not work.
 */

#endif /* ZEPHYR_INCLUDE_SYS_UTIL_MACROS_H_ */
