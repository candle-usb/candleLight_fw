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

SRC  = $(wildcard src/*.c)
SRC += $(wildcard system/src/stm32f0xx/*.c)
SRC += $(wildcard system/src/newlib/*.c)
SRC += $(wildcard system/src/cortexm/*.c)
SRC += $(wildcard system/src/cmsis/*.c)
SRC += $(wildcard Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c)
SRC += system/src/cmsis/startup_stm32f072xb.S

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
