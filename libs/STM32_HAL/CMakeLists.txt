project(STM32_HAL)

set(SOURCES
    config/stm32f0xx_hal_conf.h

    src/stm32f0xx/stm32f0xx_hal.c
    src/stm32f0xx/stm32f0xx_hal_can.c
    src/stm32f0xx/stm32f0xx_hal_cortex.c
    src/stm32f0xx/stm32f0xx_hal_dma.c
    src/stm32f0xx/stm32f0xx_hal_flash.c
    src/stm32f0xx/stm32f0xx_hal_flash_ex.c
    src/stm32f0xx/stm32f0xx_hal_gpio.c
    src/stm32f0xx/stm32f0xx_hal_i2c.c
    src/stm32f0xx/stm32f0xx_hal_i2c_ex.c
    src/stm32f0xx/stm32f0xx_hal_pcd.c
    src/stm32f0xx/stm32f0xx_hal_pcd_ex.c
    src/stm32f0xx/stm32f0xx_hal_pwr.c
    src/stm32f0xx/stm32f0xx_hal_pwr_ex.c
    src/stm32f0xx/stm32f0xx_hal_rcc.c
    src/stm32f0xx/stm32f0xx_hal_rcc_ex.c
    src/stm32f0xx/stm32f0xx_hal_tim.c
    src/stm32f0xx/stm32f0xx_hal_tim_ex.c

    src/newlib/assert.c
    src/newlib/_exit.c
    src/newlib/_sbrk.c
    src/newlib/_startup.c
    src/newlib/_syscalls.c

    src/cortexm/exception_handlers.c
    src/cortexm/_initialize_hardware.c
    src/cortexm/_reset_hardware.c

    src/cmsis/system_stm32f0xx.c

    include/stm32f0xx/stm32f0xx_hal_tim.h
    include/stm32f0xx/stm32f0xx_hal_pcd.h
    include/stm32f0xx/stm32f0xx_hal_gpio.h
    include/stm32f0xx/stm32f0xx_hal_i2c_ex.h
    include/stm32f0xx/stm32f0xx_hal_flash_ex.h
    include/stm32f0xx/stm32f0xx_hal_dma_ex.h
    include/stm32f0xx/stm32f0xx_hal_i2c.h
    include/stm32f0xx/stm32f0xx_hal_pwr.h
    include/stm32f0xx/stm32f0xx_hal_pwr_ex.h
    include/stm32f0xx/stm32f0xx_hal_dma.h
    include/stm32f0xx/stm32f0xx_hal_gpio_ex.h
    include/stm32f0xx/Legacy/stm32_hal_legacy.h
    include/stm32f0xx/stm32f0xx_hal_rcc_ex.h
    include/stm32f0xx/stm32f0xx_hal_def.h
    include/stm32f0xx/stm32f0xx_hal.h
    include/stm32f0xx/stm32f0xx_hal_rcc.h
    include/stm32f0xx/stm32f0xx_hal_cortex.h
    include/stm32f0xx/stm32f0xx_hal_flash.h
    include/stm32f0xx/stm32f0xx_hal_tim_ex.h
    include/stm32f0xx/stm32f0xx_hal_pcd_ex.h
    include/stm32f0xx/stm32f0xx_hal_can.h
    include/cmsis/cmsis_armcc.h
    include/cmsis/arm_math.h
    include/cmsis/cmsis_armcc_V6.h
    include/cmsis/arm_common_tables.h
    include/cmsis/core_cm3.h
    include/cmsis/core_cm0plus.h
    include/cmsis/core_cm7.h
    include/cmsis/cmsis_gcc.h
    include/cmsis/core_cmInstr.h
    include/cmsis/core_cmFunc.h
    include/cmsis/core_cm0.h
    include/cmsis/arm_const_structs.h
    include/cmsis/core_sc300.h
    include/cmsis/device/stm32f031x6.h
    include/cmsis/device/stm32f048xx.h
    include/cmsis/device/stm32f042x6.h
    include/cmsis/device/stm32f091xc.h
    include/cmsis/device/stm32f0xx.h
    include/cmsis/device/stm32f030x6.h
    include/cmsis/device/stm32f030x8.h
    include/cmsis/device/stm32f078xx.h
    include/cmsis/device/system_stm32f0xx.h
    include/cmsis/device/stm32f038xx.h
    include/cmsis/device/stm32f058xx.h
    include/cmsis/device/stm32f070xb.h
    include/cmsis/device/stm32f098xx.h
    include/cmsis/device/stm32f070x6.h
    include/cmsis/device/stm32f071xb.h
    include/cmsis/device/stm32f030xc.h
    include/cmsis/device/stm32f051x8.h
    include/cmsis/device/stm32f072xb.h
    include/cmsis/cmsis_device.h
    include/cmsis/core_cm4.h
    include/cmsis/core_sc000.h
    include/cmsis/core_cmSimd.h
    include/arm/semihosting.h
    include/cortexm/ExceptionHandlers.h
    include/diag/Trace.h
)

set(INCLUDE_DIRS
    include/
    include/cmsis
    include/stm32f0xx
    include/cmsis/device
    config/
)

add_library(STM32_HAL_STM32F042x6 OBJECT ${SOURCES} src/cmsis/startup_stm32f042x6.S)
    target_include_directories(STM32_HAL_STM32F042x6 PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_HAL_STM32F042x6 PRIVATE -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32F042x6 PUBLIC STM32F042x6)

add_library(STM32_HAL_STM32F072xB OBJECT ${SOURCES} src/cmsis/startup_stm32f072xb.S)
    target_include_directories(STM32_HAL_STM32F072xB PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_HAL_STM32F072xB PRIVATE -Wno-unused-parameter)
    target_compile_definitions(STM32_HAL_STM32F072xB PUBLIC STM32F072xB)
