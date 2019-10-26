project(STM32_HAL)

set(SOURCES
    config/stm32f0xx_hal_conf.h

    include/stm32f0xx/stm32f0xx_hal_def.h    
    include/stm32f0xx/stm32f0xx_hal.h
    src/stm32f0xx/stm32f0xx_hal.c
    
    include/stm32f0xx/stm32f0xx_hal_can.h
    src/stm32f0xx/stm32f0xx_hal_can.c
    
    include/stm32f0xx/stm32f0xx_hal_cortex.h
    src/stm32f0xx/stm32f0xx_hal_cortex.c

    include/stm32f0xx/stm32f0xx_hal_flash_ex.h
    src/stm32f0xx/stm32f0xx_hal_flash_ex.c
    include/stm32f0xx/stm32f0xx_hal_flash.h
    src/stm32f0xx/stm32f0xx_hal_flash.c

    include/stm32f0xx/stm32f0xx_hal_gpio_ex.h
    include/stm32f0xx/stm32f0xx_hal_gpio.h
    src/stm32f0xx/stm32f0xx_hal_gpio.c

    include/stm32f0xx/stm32f0xx_hal_pcd_ex.h
    src/stm32f0xx/stm32f0xx_hal_pcd_ex.c
    include/stm32f0xx/stm32f0xx_hal_pcd.h
    src/stm32f0xx/stm32f0xx_hal_pcd.c

    include/stm32f0xx/stm32f0xx_hal_rcc.h
    src/stm32f0xx/stm32f0xx_hal_rcc.c
    include/stm32f0xx/stm32f0xx_hal_rcc_ex.h
    src/stm32f0xx/stm32f0xx_hal_rcc_ex.c

    include/stm32f0xx/stm32f0xx_hal_tim_ex.h
    src/stm32f0xx/stm32f0xx_hal_tim_ex.c
    include/stm32f0xx/stm32f0xx_hal_tim.h
    src/stm32f0xx/stm32f0xx_hal_tim.c

    include/stm32f0xx/stm32f0xx_ll_usb.h
    src/stm32f0xx/stm32f0xx_ll_usb.c

    src/cmsis/system_stm32f0xx.c
    include/stm32f0xx/Legacy/stm32_hal_legacy.h

    include/cmsis/arm_common_tables.h
    include/cmsis/arm_const_structs.h
    include/cmsis/arm_math.h
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

set(INCLUDE_DIRS
    include/
    include/cmsis
    include/stm32f0xx
    include/cmsis/device
    config/
)

add_library(STM32_HAL_STM32F042x6 OBJECT ${SOURCES})
    target_include_directories(STM32_HAL_STM32F042x6 PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_HAL_STM32F042x6 PRIVATE -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32F042x6 PUBLIC STM32F042x6)

add_library(STM32_HAL_STM32F072xB OBJECT ${SOURCES})
    target_include_directories(STM32_HAL_STM32F072xB PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_HAL_STM32F072xB PRIVATE -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32F072xB PUBLIC STM32F072xB)
