##############################################################################
# STM32F769I-DISCOVERY + uT-Kernel 3.0 BSP2 (mtk3_bsp2)
# Copyright (c) 2026 Madhawa Polkotuwa
# SPDX-License-Identifier: MIT
#
# Equivalent of the STM32CubeIDE managed build described in
# mtk3_bsp2/doc/bsp2_stm32_cube_jp.md section 4.2.2:
#   - target symbol      _STM32CUBE_DISCOVERY_F769_
#   - include paths      mtk3_bsp2, mtk3_bsp2/config, mtk3_bsp2/include,
#                        mtk3_bsp2/mtkernel/kernel/knlinc
#   - every .c/.S under mtk3_bsp2 is compiled (as CubeIDE does); files for
#     other boards compile to nothing because of their #if MTKBSP_xxx guards.
#
# Usage:
#   make STAGE=3            milestone 3 (boot + tick + console)
#   make                    milestone 4 (task + semaphore + timeout), default
#   make STAGE=3 flash      build and program via ST-LINK (STM32_Programmer_CLI)
#   make clean
##############################################################################

TARGET   := mtk3_f769i_disco
STAGE    ?= 4
OPT      ?= -Og
BUILD_DIR := build/stage$(STAGE)

PREFIX   ?= arm-none-eabi-
CC       := $(PREFIX)gcc
AS       := $(PREFIX)gcc -x assembler-with-cpp
CP       := $(PREFIX)objcopy
SZ       := $(PREFIX)size

PROGRAMMER ?= STM32_Programmer_CLI

# Recursive wildcard: $(call rwildcard,dir,pattern)
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# ---- STM32Cube part --------------------------------------------------------
CUBE_C_SOURCES := \
  Core/Src/main.c \
  Core/Src/stm32f7xx_it.c \
  Core/Src/stm32f7xx_hal_msp.c \
  Core/Src/stm32f7xx_hal_timebase_tim.c \
  Core/Src/system_stm32f7xx.c \
  $(wildcard Drivers/STM32F7xx_HAL_Driver/Src/*.c)

CUBE_ASM_SOURCES := Core/Startup/startup_stm32f769xx.s

# ---- uT-Kernel 3.0 BSP2 part -----------------------------------------------
MTK_C_SOURCES   := $(call rwildcard,mtk3_bsp2,*.c)
MTK_ASM_SOURCES := $(call rwildcard,mtk3_bsp2,*.S)

# ---- User program ----------------------------------------------------------
APP_C_SOURCES := $(wildcard application/*.c)

C_SOURCES   := $(CUBE_C_SOURCES) $(MTK_C_SOURCES) $(APP_C_SOURCES)
ASM_SOURCES := $(CUBE_ASM_SOURCES)
ASMPP_SOURCES := $(MTK_ASM_SOURCES)

# ---- Flags -----------------------------------------------------------------
MCU := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

C_DEFS := \
  -DUSE_HAL_DRIVER \
  -DSTM32F769xx \
  -D_STM32CUBE_DISCOVERY_F769_ \
  -DAPP_STAGE=$(STAGE)

C_INCLUDES := \
  -ICore/Inc \
  -IDrivers/STM32F7xx_HAL_Driver/Inc \
  -IDrivers/STM32F7xx_HAL_Driver/Inc/Legacy \
  -IDrivers/CMSIS/Device/ST/STM32F7xx/Include \
  -IDrivers/CMSIS/Include \
  -Imtk3_bsp2 \
  -Imtk3_bsp2/config \
  -Imtk3_bsp2/include \
  -Imtk3_bsp2/mtkernel/kernel/knlinc

CFLAGS  := $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -g3 -std=gnu11 -Wall \
           -ffunction-sections -fdata-sections -MMD -MP
ASFLAGS := $(MCU) $(C_DEFS) $(C_INCLUDES) -g3 -MMD -MP

LDSCRIPT := STM32F769NIHX_FLASH.ld
LDFLAGS  := $(MCU) -T$(LDSCRIPT) --specs=nosys.specs --specs=nano.specs \
            -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections \
            -Wl,--print-memory-usage \
            -Wl,--start-group -lc -lm -Wl,--end-group

# ---- Objects (object tree mirrors the source tree: tm_com.c exists per board)
OBJECTS := $(addprefix $(BUILD_DIR)/obj/,$(C_SOURCES:.c=.o)) \
           $(addprefix $(BUILD_DIR)/obj/,$(ASM_SOURCES:.s=.o)) \
           $(addprefix $(BUILD_DIR)/obj/,$(ASMPP_SOURCES:.S=.o))

.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/obj/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	@echo CC $<
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/obj/%.o: %.s Makefile
	@mkdir -p $(dir $@)
	@echo AS $<
	@$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/obj/%.o: %.S Makefile
	@mkdir -p $(dir $@)
	@echo AS $<
	@$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	@echo LD $@
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	@$(CP) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@$(CP) -O binary -S $< $@

flash: $(BUILD_DIR)/$(TARGET).elf
	$(PROGRAMMER) -c port=SWD mode=UR -w $< -v -rst

clean:
	rm -rf build

-include $(OBJECTS:.o=.d)
