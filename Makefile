CHIP  ?= STM32F072xB
BOARD ?= candleLight

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size 

CFLAGS  = -c -std=gnu11 -mcpu=cortex-m0 -mthumb -Os 
CFLAGS += -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants 
CFLAGS += -Wall -Wextra -g3 -D $(CHIP) -D BOARD=BOARD_$(BOARD)

INCLUDES  = -I"include" 
INCLUDES += -I"system/include" -I"system/include/cmsis" -I"system/include/stm32f0xx" -I"system/include/cmsis/device" 
INCLUDES += -I"Middlewares/ST/STM32_USB_Device_Library/Core/Inc"

LDFLAGS   = -T ldscripts/mem.ld -T ldscripts/libs.ld -T ldscripts/sections.ld -nostartfiles -Xlinker --gc-sections --specs=nano.specs

SRC  = src/main.c 
SRC += src/can.c 
SRC += src/gpio.c
SRC += src/interrupts.c
SRC += src/led.c
SRC += src/queue.c
SRC += src/stm32f0xx_hal_msp.c
SRC += src/usbd_conf.c
SRC += src/usbd_desc.c
SRC += src/usbd_gs_can.c
SRC += src/util.c

SRC += system/src/stm32f0xx/stm32f0xx_hal.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_can.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_cortex.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_dma.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_flash.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_flash_ex.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_gpio.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_pcd.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_pcd_ex.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_pwr.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_pwr_ex.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_rcc.c
SRC += system/src/stm32f0xx/stm32f0xx_hal_rcc_ex.c
SRC += system/src/newlib/_exit.c
SRC += system/src/newlib/_sbrk.c
SRC += system/src/newlib/_startup.c
SRC += system/src/newlib/_syscalls.c
SRC += system/src/newlib/assert.c
SRC += system/src/cortexm/_initialize_hardware.c
SRC += system/src/cortexm/_reset_hardware.c
SRC += system/src/cortexm/exception_handlers.c
SRC += system/src/cmsis/startup_stm32f072xb.S
SRC += system/src/cmsis/system_stm32f0xx.c
SRC += Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
SRC += Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
SRC += Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c

OBJECTS = $(patsubst %.c,build/$(BOARD)/%.o,$(SRC))
ELF = build/$(BOARD)/gsusb_$(BOARD).elf
BIN = bin/gsusb_$(BOARD).bin

all: candleLight cantact

candleLight:
	$(MAKE) CHIP=STM32F072xB BOARD=candleLight bin

cantact:
	$(MAKE) CHIP=STM32F072xB BOARD=cantact bin

bin: $(BIN)

$(BIN): $(ELF)
	@mkdir -p $(dir $@)	
	$(OBJCOPY) -O binary $(ELF) $(BIN)
	$(SIZE) --format=berkeley $(ELF) 
	
$(ELF): $(OBJECTS)
	@mkdir -p $(dir $@)	
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

build/$(BOARD)/%.o : %.c
	@echo $<
	@mkdir -p $(dir $@)	
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
