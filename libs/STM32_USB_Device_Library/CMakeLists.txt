project(STM32_USB_Device_Library)

set(SOURCES
    Core/Inc/usbd_def.h
    Core/Inc/usbd_ctlreq.h
    Core/Src/usbd_ctlreq.c
    Core/Inc/usbd_ioreq.h
    Core/Src/usbd_ioreq.c
    Core/Inc/usbd_core.h
    Core/Src/usbd_core.c
)

set(INCLUDE_DIRS
    Core/Inc/
    config/
)

add_library(STM32_USB_Device_Library_STM32F042x6 STATIC ${SOURCES})
    target_include_directories(STM32_USB_Device_Library_STM32F042x6 PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_USB_Device_Library_STM32F042x6 PRIVATE ${CPUFLAGS_F0} -Wno-unused-parameter -DSTM32F0)
    target_link_libraries(STM32_USB_Device_Library_STM32F042x6 PRIVATE STM32_HAL_STM32F042x6)

add_library(STM32_USB_Device_Library_STM32F072xB STATIC ${SOURCES})
    target_include_directories(STM32_USB_Device_Library_STM32F072xB PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_USB_Device_Library_STM32F072xB PRIVATE ${CPUFLAGS_F0} -Wno-unused-parameter -DSTM32F0)
    target_link_libraries(STM32_USB_Device_Library_STM32F072xB PRIVATE STM32_HAL_STM32F072xB)

add_library(STM32_USB_Device_Library_STM32F407xE STATIC ${SOURCES})
    target_include_directories(STM32_USB_Device_Library_STM32F407xE PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_USB_Device_Library_STM32F407xE PRIVATE ${CPUFLAGS_F4} -Wno-unused-parameter -DSTM32F4)
    target_link_libraries(STM32_USB_Device_Library_STM32F407xE PRIVATE STM32_HAL_STM32F407xE)

add_library(STM32_USB_Device_Library_STM32G0B1xx STATIC ${SOURCES})
    target_include_directories(STM32_USB_Device_Library_STM32G0B1xx PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_USB_Device_Library_STM32G0B1xx PRIVATE ${CPUFLAGS_G0} -Wno-unused-parameter -DSTM32G0)
    target_link_libraries(STM32_USB_Device_Library_STM32G0B1xx PRIVATE STM32_HAL_STM32G0B1xx)

add_library(STM32_USB_Device_Library_STM32G431xx STATIC ${SOURCES})
    target_include_directories(STM32_USB_Device_Library_STM32G431xx PUBLIC ${INCLUDE_DIRS})
    target_compile_options(STM32_USB_Device_Library_STM32G431xx PRIVATE ${CPUFLAGS_G4} -Wno-unused-parameter -DSTM32G4)
    target_link_libraries(STM32_USB_Device_Library_STM32G431xx PRIVATE STM32_HAL_STM32G431xx)
