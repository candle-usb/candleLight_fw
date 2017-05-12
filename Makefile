TOOLCHAIN ?= arm-none-eabi-

CC = $(TOOLCHAIN)gcc
OBJCOPY = $(TOOLCHAIN)objcopy
SIZE = $(TOOLCHAIN)size 

CFLAGS  = -c -std=gnu11 -mcpu=cortex-m0 -mthumb -Os
CFLAGS += -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants 
CFLAGS += -Wall -Wextra -g3 -D$(CHIP) -D BOARD=BOARD_$(BOARD)

INCLUDES  = -I"include" 
INCLUDES += -I"system/include" -I"system/include/cmsis" -I"system/include/stm32f0xx" -I"system/include/cmsis/device" 
INCLUDES += -I"Middlewares/ST/STM32_USB_Device_Library/Core/Inc"

LDFLAGS  = -mcpu=cortex-m0 -mthumb -O
LDFLAGS += -Wall -Wextra -g3
LDFLAGS += -T ldscripts/mem.ld -T ldscripts/libs.ld -T ldscripts/sections.ld 
LDFLAGS += -nostartfiles -Xlinker --gc-sections --specs=nano.specs

SRC  = $(wildcard src/*.c)
SRC += $(wildcard system/src/stm32f0xx/*.c)
SRC += $(wildcard system/src/newlib/*.c)
SRC += $(wildcard system/src/cortexm/*.c)
SRC += $(wildcard system/src/cmsis/*.c)
SRC += $(wildcard Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c)
OBJ = $(patsubst %.c,build/$(BOARD)/%.o,$(SRC))
DEP = $(OBJ:%.o=%.d)

ASM_SRC  = system/src/cmsis/startup_stm32f042x6.S
ASM_OBJ += $(patsubst %.S,build/$(BOARD)/%.asmo,$(ASM_SRC))
DEP     += $(ASM_OBJ:%.asmo=%.d)

ELF = build/$(BOARD)/gsusb_$(BOARD).elf
BIN = bin/gsusb_$(BOARD).bin

all: candleLight cantact

.PHONY : clean all

clean:
	$(MAKE) BOARD=candleLight board-clean
	$(MAKE) BOARD=cantact board-clean

candleLight:
	$(MAKE) CHIP=STM32F042x6 BOARD=candleLight bin

flash-candleLight:
	$(MAKE) CHIP=STM32F042x6 BOARD=candleLight board-flash

cantact:
	$(MAKE) CHIP=STM32F072xB BOARD=cantact bin

flash-cantact:
	$(MAKE) CHIP=STM32F072xB BOARD=cantact board-flash

board-flash: bin
	sudo dfu-util -d 1d50:606f -a 0 -s 0x08000000 -D $(BIN)

bin: $(BIN)

$(BIN): $(ELF)
	@mkdir -p $(dir $@)	
	$(OBJCOPY) -O binary $(ELF) $(BIN)
	$(SIZE) --format=berkeley $(ELF) 
	
$(ELF): $(OBJ) $(ASM_OBJ)
	@mkdir -p $(dir $@)	
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(ASM_OBJ) 

-include $(DEP)

build/$(BOARD)/%.o : %.c
	@echo $<
	@mkdir -p $(dir $@)	
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

build/$(BOARD)/%.asmo : %.S
	@echo $<
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

.PHONY : board-clean
board-clean :
	-rm -f $(BIN) $(OBJ) $(ASM_OBJ) $(DEP)
