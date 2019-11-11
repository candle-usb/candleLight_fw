cmake_minimum_required(VERSION 3.10)
project(candleLightFirmware C ASM)

add_compile_options(
    -std=gnu11
    -mcpu=cortex-m0 -mthumb
    -Wall -Wextra -Werror -Wno-deprecated
    -fmessage-length=0
    -fsigned-char
    -ffunction-sections -fdata-sections
    -ffreestanding
    -fno-move-loop-invariants
    -Os -g3
)

add_link_options(
    -mcpu=cortex-m0 -mthumb -O
    -Wall -Wextra -g3
    -nostartfiles -Xlinker --gc-sections --specs=nano.specs
    -T ${CMAKE_SOURCE_DIR}/ldscripts/ldscript.ld
)


add_subdirectory(libs/STM32_HAL)
add_subdirectory(libs/STM32_USB_Device_Library)

set(
    SOURCE_FILES
        include/config.h

        include/gs_usb.h
        include/usbd_desc.h src/usbd_desc.c
        include/usbd_gs_can.h src/usbd_gs_can.c
        src/usbd_conf.c

        include/can.h src/can.c
        include/dfu.h src/dfu.c
        include/flash.h src/flash.c
        include/gpio.h src/gpio.c
        include/led.h src/led.c
        include/queue.h src/queue.c
        include/timer.h src/timer.c
        include/util.h src/util.c

        src/main.c
        src/interrupts.c
)


####### some helpers to generate targets
function(make_bin_file target)
    add_custom_command(
        TARGET ${target} POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_OBJCOPY} -O binary ${target} ${target}.bin
    )
endfunction()

function(show_object_size target)
    string(REPLACE "objcopy" "size" CMAKE_OBJSIZE "${CMAKE_OBJCOPY}")
    add_custom_command(
        TARGET ${target} POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_OBJSIZE} ${target}
    )
endfunction()

function(add_flash_target target)
	add_custom_target(
		flash-${target} dfu-util -d 1d50:606f -a 0 -R -s 0x08000000 -D ${target}.bin
	)
endfunction()

######### commands for adding each target have a lot in common: make helper func.
# Split into two categories, F042-based and F072-based.

function(add_f042_target TGTNAME)
	add_executable(${TGTNAME}_fw ${SOURCE_FILES})
    target_include_directories(${TGTNAME}_fw PRIVATE include/)
    target_compile_definitions(${TGTNAME}_fw PRIVATE BOARD=BOARD_${TGTNAME})
    target_link_libraries(${TGTNAME}_fw PRIVATE STM32_HAL_STM32F042x6 STM32_USB_Device_Library_STM32F042x6)
    make_bin_file(${TGTNAME}_fw)
    show_object_size(${TGTNAME}_fw)
    add_flash_target(${TGTNAME}_fw)
endfunction()

function(add_f072_target TGTNAME)
	add_executable(${TGTNAME}_fw ${SOURCE_FILES})
    target_include_directories(${TGTNAME}_fw PRIVATE include/)
    target_compile_definitions(${TGTNAME}_fw PRIVATE BOARD=BOARD_${TGTNAME})
    target_link_libraries(${TGTNAME}_fw PRIVATE STM32_HAL_STM32F072xB STM32_USB_Device_Library_STM32F072xB)
    make_bin_file(${TGTNAME}_fw)
    show_object_size(${TGTNAME}_fw)
    add_flash_target(${TGTNAME}_fw)
endfunction()

########## generate list of targets.
# the "_fw" part is appended automatically
set(TGT042_LIST "cantact" "canalyze" "canable" "usb2can")
set(TGT072_LIST "candleLight")


foreach (TGTNAME IN LISTS TGT042_LIST)
	option(BUILD_${TGTNAME} "Build firmware for \"${TGTNAME}\" (default=yes)" ON)
	if (BUILD_${TGTNAME})
		add_f042_target(${TGTNAME})
	endif()
endforeach()

foreach (TGTNAME IN LISTS TGT072_LIST)
	option(BUILD_${TGTNAME} "Build firmware for \"${TGTNAME}\" (default=yes)" ON)
	if (BUILD_${TGTNAME})
		add_f072_target(${TGTNAME})
	endif()
endforeach()

message("*******************")
message("You may now:\n\t-compile all targets ('make')\n\t-compile a single target (e.g. 'make cantact_fw'")
message("\t-flash a device (e.g. 'make flash-cantact_fw'")
