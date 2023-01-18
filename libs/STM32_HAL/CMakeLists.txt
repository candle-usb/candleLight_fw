project(STM32_HAL)

set(STM32F0_HALDIR stm32f0xx_hal_driver)
set(STM32F4_HALDIR stm32f4xx_hal_driver)
set(STM32G0_HALDIR stm32g0xx_hal_driver)

set(STM32F0_SOURCES
    config/stm32f0xx_hal_conf.h

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_def.h
    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_can.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_can.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_cortex.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_cortex.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_flash_ex.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_flash_ex.c
    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_flash.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_flash.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_gpio_ex.h
    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_gpio.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_gpio.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_pcd_ex.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_pcd_ex.c
    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_pcd.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_pcd.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_rcc.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_rcc.c
    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_rcc_ex.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_rcc_ex.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_tim_ex.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_tim_ex.c
    ${STM32F0_HALDIR}/Inc/stm32f0xx_hal_tim.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_hal_tim.c

    ${STM32F0_HALDIR}/Inc/stm32f0xx_ll_usb.h
    ${STM32F0_HALDIR}/Src/stm32f0xx_ll_usb.c

    src/cmsis/system_stm32f0xx.c
    ${STM32F0_HALDIR}/Inc/Legacy/stm32_hal_legacy.h

    include/cmsis/cmsis_compiler.h
    include/cmsis/cmsis_device.h
    include/cmsis/cmsis_gcc.h
    include/cmsis/cmsis_version.h
    include/cmsis/core_cm0.h
    include/cmsis/device/stm32f0xx.h
    include/cmsis/device/stm32f042x6.h
    include/cmsis/device/stm32f072xb.h
    include/cmsis/device/system_stm32f0xx.h
)


set(STM32F4_SOURCES
    config/stm32f4xx_hal_conf.h

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_def.h
    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_can.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_can.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_cortex.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_cortex.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_flash_ex.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_flash_ex.c
    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_flash.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_flash.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_gpio_ex.h
    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_gpio.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_gpio.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_pcd_ex.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_pcd_ex.c
    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_pcd.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_pcd.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_rcc.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_rcc.c
    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_rcc_ex.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_rcc_ex.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_pwr.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_pwr.c
    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_pwr_ex.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_pwr_ex.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_tim_ex.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_tim_ex.c
    ${STM32F4_HALDIR}/Inc/stm32f4xx_hal_tim.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_hal_tim.c

    ${STM32F4_HALDIR}/Inc/stm32f4xx_ll_usb.h
    ${STM32F4_HALDIR}/Src/stm32f4xx_ll_usb.c

    src/cmsis/system_stm32f4xx.c
    ${STM32F4_HALDIR}/Inc/Legacy/stm32_hal_legacy.h

    include/cmsis/cmsis_compiler.h
    include/cmsis/cmsis_device.h
    include/cmsis/cmsis_gcc.h
    include/cmsis/cmsis_version.h
    include/cmsis/core_cm4.h
    include/cmsis/device/stm32f4xx.h
    include/cmsis/device/stm32f407xx.h
    include/cmsis/device/system_stm32f4xx.h
)

set(STM32G0_SOURCES
    config/stm32g0xx_hal_conf.h

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_def.h
    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_fdcan.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_fdcan.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_cortex.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_cortex.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_flash_ex.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_flash_ex.c
    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_flash.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_flash.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_gpio_ex.h
    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_gpio.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_gpio.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_pcd_ex.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_pcd_ex.c
    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_pcd.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_pcd.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_rcc.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_rcc.c
    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_rcc_ex.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_rcc_ex.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_pwr.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_pwr.c
    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_pwr_ex.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_pwr_ex.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_tim_ex.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_tim_ex.c
    ${STM32G0_HALDIR}/Inc/stm32g0xx_hal_tim.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_hal_tim.c

    ${STM32G0_HALDIR}/Inc/stm32g0xx_ll_usb.h
    ${STM32G0_HALDIR}/Src/stm32g0xx_ll_usb.c

    src/cmsis/system_stm32g0xx.c
    ${STM32G0_HALDIR}/Inc/Legacy/stm32_hal_legacy.h

    include/cmsis/cmsis_compiler.h
    include/cmsis/cmsis_device.h
    include/cmsis/cmsis_gcc.h
    include/cmsis/cmsis_version.h
    include/cmsis/core_cm0plus.h
    include/cmsis/device/stm32g0xx.h
    include/cmsis/device/stm32g0b1xx.h
    include/cmsis/device/system_stm32g0xx.h
)

set(INCLUDE_DIRS
    include/
    include/cmsis
    include/cmsis/device
    config/
)

add_library(STM32_HAL_STM32F042x6 STATIC ${STM32F0_SOURCES})
    target_include_directories(STM32_HAL_STM32F042x6 PUBLIC ${INCLUDE_DIRS} ${STM32F0_HALDIR}/Inc)
    target_compile_options(STM32_HAL_STM32F042x6 PRIVATE ${CPUFLAGS_F0} -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32F042x6 PUBLIC STM32F042x6 HAL_TARGET_PREFIX=stm32f0xx)

add_library(STM32_HAL_STM32F072xB STATIC ${STM32F0_SOURCES})
    target_include_directories(STM32_HAL_STM32F072xB PUBLIC ${INCLUDE_DIRS} ${STM32F0_HALDIR}/Inc)
    target_compile_options(STM32_HAL_STM32F072xB PRIVATE ${CPUFLAGS_F0} -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32F072xB PUBLIC STM32F072xB HAL_TARGET_PREFIX=stm32f0xx)

add_library(STM32_HAL_STM32F407xE STATIC ${STM32F4_SOURCES})
    target_include_directories(STM32_HAL_STM32F407xE PUBLIC ${INCLUDE_DIRS} ${STM32F4_HALDIR}/Inc)
    target_compile_options(STM32_HAL_STM32F407xE PRIVATE ${CPUFLAGS_F4} -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32F407xE PUBLIC STM32F407xx HAL_TARGET_PREFIX=stm32f4xx)

add_library(STM32_HAL_STM32G0B1xK STATIC ${STM32G0_SOURCES})
    target_include_directories(STM32_HAL_STM32G0B1xK PUBLIC ${INCLUDE_DIRS} ${STM32G0_HALDIR}/Inc)
    target_compile_options(STM32_HAL_STM32G0B1xK PRIVATE ${CPUFLAGS_G0} -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32G0B1xK PUBLIC STM32G0B1xx HAL_TARGET_PREFIX=stm32g0xx)
