cmake_minimum_required(VERSION 3.10)
project(candleLightFirmware)

add_compile_options(
    -std=gnu11
    -mcpu=cortex-m0 -mthumb 
    -Wall -Wextra -Werror
    -fmessage-length=0 
    -fsigned-char 
    -ffunction-sections -fdata-sections 
    -ffreestanding 
    -fno-move-loop-invariants
    -Os -g3
    -flto -ffat-lto-objects
    --specs=nano.specs
    --specs=nosys.specs
)

add_link_options(
    -mcpu=cortex-m0 -mthumb
    -Wall -Wextra -g3
    -Xlinker --gc-sections 
    --specs=nano.specs
    --specs=nosys.specs
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

        src/startup.c
        src/main.c
        src/interrupts.c
)

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

add_executable(candleLight_fw ${SOURCE_FILES})
    target_include_directories(candleLight_fw PRIVATE include/)
    target_compile_definitions(candleLight_fw PRIVATE BOARD=BOARD_candleLight)
    target_link_libraries(candleLight_fw PRIVATE STM32_HAL_STM32F042x6 STM32_USB_Device_Library_STM32F042x6)
    target_link_options(candleLight_fw PRIVATE -T ${CMAKE_SOURCE_DIR}/ldscripts/STM32F042X6.ld)
    make_bin_file(candleLight_fw)
    show_object_size(candleLight_fw)
    add_flash_target(candleLight_fw)

add_executable(cantact_fw ${SOURCE_FILES})
    target_include_directories(cantact_fw PRIVATE include/)
    target_compile_definitions(cantact_fw PRIVATE BOARD=BOARD_cantact)
    target_link_libraries(cantact_fw PRIVATE STM32_HAL_STM32F072xB STM32_USB_Device_Library_STM32F072xB)
    target_link_options(cantact_fw PRIVATE -T ${CMAKE_SOURCE_DIR}/ldscripts/STM32F072XB.ld)
    make_bin_file(cantact_fw)
    show_object_size(cantact_fw)
    add_flash_target(cantact_fw)
