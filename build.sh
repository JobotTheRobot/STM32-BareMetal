#!/usr/bin/env sh

mkdir -p output

# ---- Build bootloader ----
# Generate object file
arm-none-eabi-gcc -c -g -Og -Wall -Wextra -mcpu=cortex-m3 -mthumb bootloader.c -o output/bootloader.o
# Link the object file
arm-none-eabi-gcc -nostdlib -nostartfiles -Wl,-Tbootloader_memory.ld output/bootloader.o -o output/bootloader.elf
# Generate binary file
arm-none-eabi-objcopy -O binary output/bootloader.elf output/bootloader.bin

# ---- Build main application ----
# Generate object file
arm-none-eabi-gcc -c -g -Og -Wall -Wextra -mcpu=cortex-m3 -mthumb main.c -o output/main.o
# Link the object file
arm-none-eabi-gcc -nostdlib -nostartfiles -Wl,-Tmain_memory.ld output/main.o -o output/main.elf
# Generate binary file
arm-none-eabi-objcopy -O binary output/main.elf output/main.bin