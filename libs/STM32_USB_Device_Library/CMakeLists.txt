#
# The MIT License (MIT)
#
# Copyright (c) 2019 Hubert Denkmair
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

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
