# SPDX-FileCopyrightText: 2026 Artem Novak
# SPDX-License-Identifier: MIT
CROSS ?= arm-none-eabi-

CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

BUILD := build

CFLAGS := \
	-march=armv7-a \
	-marm \
	-ffreestanding \
	-fno-builtin \
	-fno-stack-protector \
	-fno-pic \
	-fno-pie \
	-nostdlib \
	-Wall \
	-Wextra \
	-Iinclude

OBJS := \
	$(BUILD)/start.o \
	$(BUILD)/main.o \
	$(BUILD)/console.o \
	$(BUILD)/fdt.o \
	$(BUILD)/menu.o \
	$(BUILD)/boot.o \
	$(BUILD)/gt58wifi-zImage.o \
	$(BUILD)/platform.o \
	$(BUILD)/sdhci.o \
	$(BUILD)/input.o

all: $(BUILD)/tuxberry.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/start.o: arch/arm/start.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/main.o: core/main.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/console.o: core/console.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/fdt.o: core/fdt.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/menu.o: core/menu.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/boot.o: core/boot.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/gt58wifi-zImage.o: payload/gt58wifi-zImage | $(BUILD)
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm \
		--rename-section .data=.rodata.payload,alloc,load,readonly,data,contents \
		$< $@

$(BUILD)/platform.o: platform/msm8916/platform.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/sdhci.o: platform/msm8916/sdhci.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/input.o: device/gt58wifi/input.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/tuxberry.elf: $(OBJS) linker.ld
	$(LD) -T linker.ld -o $@ $(OBJS)

$(BUILD)/tuxberry.bin: $(BUILD)/tuxberry.elf
	$(OBJCOPY) -O binary $< $@

package: all
	./scripts/package-gt58wifi.sh

clean:
	rm -rf $(BUILD)

.PHONY: all package clean
