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

all: $(BUILD)/tuxberry.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/start.o: arch/arm/start.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/main.o: core/main.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/console.o: core/console.c include/tuxberry.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/tuxberry.elf: \
	$(BUILD)/start.o \
	$(BUILD)/main.o \
	$(BUILD)/console.o \
	linker.ld
	$(LD) -T linker.ld -o $@ \
		$(BUILD)/start.o \
		$(BUILD)/main.o \
		$(BUILD)/console.o

$(BUILD)/tuxberry.bin: $(BUILD)/tuxberry.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
