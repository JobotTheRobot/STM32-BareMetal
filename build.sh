#!/usr/bin/env sh

mkdir -p output

# Generate object file
arm-none-eabi-gcc -c -g -mcpu=cortex-m3 -mthumb main.c -o output/main.o
# Link the object file
arm-none-eabi-gcc -nostdlib -nostartfiles -Wl,-Tmemory.ld output/main.o -o output/application.elf
# Generate binary file
arm-none-eabi-objcopy -O binary output/application.elf output/application.bin