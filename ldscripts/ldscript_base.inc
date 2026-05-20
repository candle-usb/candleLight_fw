/* -*-ld-script-*-
 *
 * The MIT License (MIT)
 *
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

/*
 * The following is a template linker script since most of this is identical across targets.
 *
 * the cmake build sys will process this with 'configure_file()' into a cpu-specific header.
 *
 * It would be possible to use gcc to preprocess a common .ld file (and therefore leverage
 * #ifdef and other mechanisms), but has certain limitations and challenges that I decided to avoid.
 *
 *
 * fenugrec 2022
 */

__STACK_SIZE = @LDV_STACK_SIZE@;
__HEAP_SIZE  = @LDV_HEAP_SIZE@;

MEMORY
{
	FLASH (rx) : ORIGIN = @LDV_FLASH_START@, LENGTH = @LDV_FLASH_SIZE@
	RAM (rw) : ORIGIN = @LDV_RAM_START@, LENGTH = @LDV_RAM_SIZE@
}

ENTRY(Reset_Handler)

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))
		*(.text*)

		KEEP(*(.init))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		*(.rodata*)

		KEEP(*(.eh_frame*))
	} > FLASH

	.copy.table @LDV_READONLY@ :
	{
		. = ALIGN(4);
		__copy_table_start__ = .;
		LONG (__data_source)
		LONG (__data_start)
		LONG (__data_size)
		__copy_table_end__ = .;
	} > FLASH

	.preinit_array @LDV_READONLY@ :
	{
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
		. = ALIGN(4);
	} >FLASH

	.init_array @LDV_READONLY@ :
	{
		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
		. = ALIGN(4);
	} >FLASH

	__etext = ALIGN (4);

	.data : AT (__etext)
	{
		*(.data)
		*(.data.*)

		. = ALIGN(4);
		/* All data end */
		PROVIDE( __data_end = . );
	} > RAM

	PROVIDE( __data_start = ADDR(.data));
	PROVIDE( __data_size = __data_end - __data_start );
	PROVIDE( __data_source = LOADADDR(.data));
	PROVIDE( __data_source_end = LOADADDR(.data) + SIZEOF(.data) );
	PROVIDE( __data_source_size = __data_source_end - __data_source );

	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM AT > RAM

	.heap (COPY) :
	{
		. = ALIGN(8);
		__end__ = .;
		PROVIDE(end = .);
		. = . + __HEAP_SIZE;
		. = ALIGN(8);
		__HeapLimit = .;
	} > RAM

	.stack (ORIGIN(RAM) + LENGTH(RAM) - __STACK_SIZE) (COPY) :
	{
		. = ALIGN(8);
		__StackLimit = .;
		. = . + __STACK_SIZE;
		. = ALIGN(8);
		__StackTop = .;
	} > RAM
	PROVIDE(__stack = __StackTop);

	/DISCARD/ : {
		/* Throw away C++ exception handling information */
		*(.ARM.exidx*)
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		*(.note .note.*)
		/* Throw away dtors and fini_array handling */
		*(.dtors .dtors.*)
		*(.fini .fini.*)
		*(.fini_array .fini_array.*)
	}

	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
