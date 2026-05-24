cmake_minimum_required(VERSION 3.13)
project(candleLightFirmware C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

	# Set the possible values for cmake-gui drop-down
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

####### check for newlib and picolibc

include(CheckSymbolExists)

check_symbol_exists(__PICOLIBC__ "sys/features.h" HAVE_PICOLIBC)
check_symbol_exists(__NEWLIB__ "sys/features.h" HAVE_NEWLIB)

if(HAVE_PICOLIBC)
	message(FATAL_ERROR
		"Picolibc is not yet supported. Please install a newlib based toolchain, see: "
		"https://github.com/candle-usb/candleLight_fw/#building")
elseif(HAVE_NEWLIB)
	message(STATUS "Using newlib")
	set(LIBC_OPTIONS
		"-specs=nano.specs"
		"-specs=nosys.specs"
	)
else()
	message(FATAL_ERROR "Neither Picolibc nor Newlib detected.")
endif()

####### check linker not supporting "(READONLY)"

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS "11.0")
    set(HAVE_READONLY FALSE CACHE INTERNAL "Linker does not support (READONLY) keyword")
    message(STATUS "Disabling Linker (READONLY) flag")
else()
    set(HAVE_READONLY TRUE CACHE INTERNAL "Linker supports (READONLY) keyword")
    message(STATUS "Enabling linker (READONLY) flag")
endif()

set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_DEBUG "-O2")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)

add_compile_options(
	${LIBC_OPTIONS}
	-Wall
	-Werror
	-Wextra
	-Wstrict-prototypes
	-fdata-sections
	-ffreestanding
	-ffunction-sections
	-fmessage-length=0
	-fsigned-char
	-g3
	-mthumb
	-std=gnu11
)

# need these later, per-platform
set(CPUFLAGS_F0 -mcpu=cortex-m0)
set(CPUFLAGS_F4 -mcpu=cortex-m4)
set(CPUFLAGS_G0 -mcpu=cortex-m0)

add_link_options(
	${LIBC_OPTIONS}
	-Wall
	-Wextra
	-g3
	-mthumb
	LINKER:--gc-sections
	LINKER:--print-memory-usage
)

add_subdirectory(libs/STM32_HAL)
add_subdirectory(libs/STM32_USB_Device_Library)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# uncrustify - ignore HAL and third-party libs
file(GLOB_RECURSE CHECK_SOURCE_FILES
	"include/*.[ch]"
	"src/*.[ch]"
)

file(GLOB_RECURSE CHECK_EXTRA_FILES
	"70-candle-usb.rules"
	"LICENSE.md"
	"README.md"
	"cmake/*"
	"gdb_oocd.cmds"
	"ldscripts/*"
	"openocd.cfg"
)

add_custom_target(uncrustify
	COMMAND uncrustify -c "${CMAKE_CURRENT_SOURCE_DIR}/uncrustify.cfg" --replace --no-backup ${CHECK_SOURCE_FILES}
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Formatting source code with Uncrustify..."
)

add_custom_target(codespell
	COMMAND codespell -w ${CHECK_SOURCE_FILES} ${CHECK_EXTRA_FILES}
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Spellchecking source code with Codespell..."
)

# Add a custom target that produces version.h, plus
# a dummy output that's not actually produced, in order
# to force version.hmake to always be re-run before the build

add_custom_target(version_h
	BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/version.h"
	COMMAND ${CMAKE_COMMAND}
	-D SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
	-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gitversion.cmake
)

set(
	SOURCE_FILES
	include/config.h

	include/gs_usb.h
	include/usbd_desc.h src/usbd_desc.c
	include/usbd_gs_can.h src/usbd_gs_can.c
	src/usbd_conf.c

	include/board.h
	include/can.h
	include/can_common.h src/can_common.c
	include/device.h
	include/dfu.h src/dfu.c
	include/gpio.h src/gpio.c
	include/led.h src/led.c
	include/timer.h src/timer.c
	include/util.h src/util.c

	src/interrupts.c
	src/main.c
	src/no-op.c
	src/startup.c

	${CMAKE_CURRENT_BINARY_DIR}/version.h
)

####### some helpers to generate targets

## objcopy to produce .bin file
function(make_bin_file target)
	add_custom_command(
		TARGET ${target} POST_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		BYPRODUCTS ${target}.bin
		COMMAND ${CMAKE_OBJCOPY} -O binary ${target} ${target}.bin
	)
endfunction()

## report size
function(show_object_size target)
	string(REPLACE "objcopy" "size" CMAKE_OBJSIZE "${CMAKE_OBJCOPY}")
	add_custom_command(
		TARGET ${target} POST_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_OBJSIZE} ${target}
	)
endfunction()

find_package(DFUSuffix)
## run dfu-suffix to append DFU stuff and signature; generate relevant flash-* target
# TODO ? : run with execute_proces(... OUTPUT_QUIET ) instead of '... 1>/dev/null'

function(dfu_flash target)
	if (DFU_SUFFIX_EXECUTABLE)
		add_custom_command(TARGET ${target}
			POST_BUILD
			BYPRODUCTS ${target}.dfu
			COMMAND ${CMAKE_OBJCOPY} -O binary ${target} ${target}.dfu
			COMMAND ${DFU_SUFFIX_EXECUTABLE} --add ${target}.dfu --vid 1d50 --pid 606f 1>/dev/null
			COMMENT "create and sign dfu bin file: ${TGTNAME}_fw"
		)

		add_custom_target(flash-${target}
			dfu-util -a 0 -s 0x08000000:leave -D ${target}.dfu
			DEPENDS ${target}.dfu
		)
	else()
		add_custom_target(flash-${target}
			dfu-util -d 1d50:606f -a 0 -s 0x08000000:leave -D ${target}.bin
			DEPENDS ${target}.bin
		)
	endif()
endfunction()

######### ldscripts are mostly identical.
# There's many ways of handling this :
# - external script (e.g. python), see e.g. libopencm3
# - preprocess .ldscript with gcc (some caveats, since .ld syntax != C)
# - concatenate files with file(APPEND...) : I wasn't able to make this work
# - configure_file().
#
# With configure_file(), the ldscripts are generated
# at configure time.

function(populate_ldscript)
	set(prefix LDV)
	set(options)
	set(oneValueArgs
		CPU_FAMILY
		FLASH_START
		FLASH_SIZE
		RAM_START
		RAM_SIZE
		STACK_SIZE
		HEAP_SIZE)
	set(multiValueArgs)
	cmake_parse_arguments("${prefix}" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if(HAVE_READONLY)
		set(LDV_READONLY "(READONLY)")
	else()
		set(LDV_READONLY "")
	endif()

	# now we should have function-scope vars like LDV_FLASH_SIZE etc.

	set(TEMPLATE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/ldscript_base.inc")
	set(OUTPUT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${${prefix}_CPU_FAMILY}_processed.ld")

	# Produce a new file in the current builddir

	configure_file("${TEMPLATE_SCRIPT}" "${OUTPUT_SCRIPT}" @ONLY)

	add_custom_target(${${prefix}_CPU_FAMILY}_linker_script DEPENDS "${OUTPUT_SCRIPT}")
	set(${${prefix}_CPU_FAMILY}_LINKER_SCRIPT "${OUTPUT_SCRIPT}" PARENT_SCOPE)
endfunction()

##################
# generate ldscripts for all supported targets.
# If this gets out of hand, an external solution may be better
# (e.g. like libopencm3 does with a python script)

populate_ldscript(CPU_FAMILY STM32F042X6
	FLASH_START 0x08000000
	FLASH_SIZE 32k
	RAM_START 0x20000000
	RAM_SIZE 6k
	STACK_SIZE 1k
	HEAP_SIZE 0k
)

populate_ldscript(CPU_FAMILY STM32F072XB
	FLASH_START 0x08000000
	FLASH_SIZE 128k
	RAM_START 0x20000000
	RAM_SIZE 16k
	STACK_SIZE 2k
	HEAP_SIZE 0k
)

populate_ldscript(CPU_FAMILY STM32F407XE
	FLASH_START 0x08000000
	FLASH_SIZE 512k
	RAM_START 0x20000000
	RAM_SIZE 128k
	STACK_SIZE 2k
	HEAP_SIZE 1k
)

populate_ldscript(CPU_FAMILY STM32G0B1xx
	FLASH_START 0x08000000
	FLASH_SIZE 128k
	RAM_START 0x20000000
	RAM_SIZE 144k
	STACK_SIZE 2k
	HEAP_SIZE 1k
)

######### commands for adding each target have a lot in common: make helper func.
# one helper func per STM32 CPU family

function(add_target_common TGTNAME CPU_FAMILY)
	add_executable(${TGTNAME}_fw ${SOURCE_FILES})
	add_dependencies(${TGTNAME}_fw version_h)
	target_include_directories(${TGTNAME}_fw PRIVATE include/ ${CMAKE_CURRENT_BINARY_DIR})
	add_dependencies(${TGTNAME}_fw ${CPU_FAMILY}_linker_script)
	set_target_properties(${TGTNAME}_fw PROPERTIES
		LINK_DEPENDS ${${CPU_FAMILY}_LINKER_SCRIPT}
	)
	target_link_options(${TGTNAME}_fw PRIVATE -T ${${CPU_FAMILY}_LINKER_SCRIPT} LINKER:-Map=${TGTNAME}_fw.map)
	set_target_properties(${TGTNAME}_fw PROPERTIES
		ADDITIONAL_CLEAN_FILES "${TGTNAME}_fw.map"
	)
	make_bin_file(${TGTNAME}_fw)
	dfu_flash(${TGTNAME}_fw)
	show_object_size(${TGTNAME}_fw)
endfunction()

function(add_f042_target TGTNAME)
	add_target_common(${TGTNAME} STM32F042X6)
	target_compile_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_F0})
	target_compile_definitions(${TGTNAME}_fw PRIVATE BOARD_${TGTNAME} STM32F0 CONFIG_BXCAN)
	target_sources(${TGTNAME}_fw PRIVATE "src/boards/legacy.c")
	target_sources(${TGTNAME}_fw PRIVATE "src/can/bxcan.c")
	target_sources(${TGTNAME}_fw PRIVATE "src/device/device_f0.c")
	target_link_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_F0})
	target_link_libraries(${TGTNAME}_fw PRIVATE STM32_HAL_STM32F042x6 STM32_USB_Device_Library_STM32F042x6)
endfunction()

function(add_f072_target TGTNAME)
	add_target_common(${TGTNAME} STM32F072XB)
	target_compile_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_F0})
	target_compile_definitions(${TGTNAME}_fw PRIVATE BOARD_${TGTNAME} STM32F0 CONFIG_BXCAN)
	target_sources(${TGTNAME}_fw PRIVATE "src/boards/legacy.c")
	target_sources(${TGTNAME}_fw PRIVATE "src/can/bxcan.c")
	target_sources(${TGTNAME}_fw PRIVATE "src/device/device_f0.c")
	target_link_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_F0})
	target_link_libraries(${TGTNAME}_fw PRIVATE STM32_HAL_STM32F072xB STM32_USB_Device_Library_STM32F072xB)
endfunction()

function(add_f407_target TGTNAME)
	add_target_common(${TGTNAME} STM32F407XE)
	target_compile_definitions(${TGTNAME}_fw PRIVATE BOARD_${TGTNAME} STM32F4 CONFIG_BXCAN)
	target_compile_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_F4})
	target_sources(${TGTNAME}_fw PRIVATE "src/boards/legacy.c")
	target_sources(${TGTNAME}_fw PRIVATE "src/can/bxcan.c")
	target_sources(${TGTNAME}_fw PRIVATE "src/device/device_f4.c")
	target_link_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_F4})
	target_link_libraries(${TGTNAME}_fw PRIVATE STM32_HAL_STM32F407xE STM32_USB_Device_Library_STM32F407xE)
endfunction()

function(add_g0b1_target TGTNAME)
	add_target_common(${TGTNAME} STM32G0B1xx)
	target_compile_definitions(${TGTNAME}_fw PRIVATE BOARD_${TGTNAME} STM32G0 CONFIG_M_CAN)
	target_compile_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_G0})
	target_sources(${TGTNAME}_fw PRIVATE "src/device/device_g0.c")
	target_link_options(${TGTNAME}_fw BEFORE PRIVATE ${CPUFLAGS_G0})
	target_link_libraries(${TGTNAME}_fw PRIVATE STM32_HAL_STM32G0B1xx STM32_USB_Device_Library_STM32G0B1xx)
endfunction()

########## generate list of targets.
# the "_fw" part is appended automatically

set(TGTF042_LIST
	"canable"
	"canalyze"
	"cannette"
	"cantact"
	"falcan"
	"usb2can"
)
set(TGTF072_LIST
	"CANable_MKS"
	"CONVERTDEVICE_xCAN"
	"DSD_TECH_SH_C30A"
	"FYSETC_UCAN"
	"can_module_com_classic"
	"candleLight"
)
set(TGTF407_LIST
	"STM32F4_DevBoard"
)
set(TGTG0B1_LIST
	"CONVERTDEVICE_xCANFD"
	"budgetcan"
)

foreach (TGTNAME IN LISTS TGTF042_LIST)
	option(BUILD_${TGTNAME} "Build firmware for \"${TGTNAME}\" (default=yes)" ON)
	if (BUILD_${TGTNAME})
		add_f042_target(${TGTNAME})
	endif()
endforeach()

foreach (TGTNAME IN LISTS TGTF072_LIST)
	option(BUILD_${TGTNAME} "Build firmware for \"${TGTNAME}\" (default=yes)" ON)
	if (BUILD_${TGTNAME})
		add_f072_target(${TGTNAME})
	endif()
endforeach()

foreach (TGTNAME IN LISTS TGTF407_LIST)
	option(BUILD_${TGTNAME} "Build firmware for \"${TGTNAME}\" (default=yes)" ON)
	if (BUILD_${TGTNAME})
		add_f407_target(${TGTNAME})
	endif()
endforeach()

foreach (TGTNAME IN LISTS TGTG0B1_LIST)
	option(BUILD_${TGTNAME} "Build firmware for \"${TGTNAME}\" (default=yes)" OFF)
	if (BUILD_${TGTNAME})
		add_g0b1_target(${TGTNAME})
	endif()
endforeach()

message("*******************")
message("You may now:\n\t-compile all targets ('make')\n\t-compile a single target (e.g. 'make cantact_fw'")
message("\t-flash a device (e.g. 'make flash-cantact_fw'")
