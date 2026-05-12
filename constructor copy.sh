#!/bin/bash
set -e

mkdir -p "temp"

#compile asm
nasm "code/kernel_entry.asm" -f elf -o "temp/kernel_entry.o"
nasm -f bin "code/boot.asm" -o "temp/boot.bin"
nasm -f bin "code/stage2.asm" -o "temp/stage2.bin"

#compile c
i386-elf-gcc -ffreestanding -m32 -g -c "code/kernel.c" -o "temp/kernel.o"

#link
i386-elf-ld -o "temp/full_kernel.bin" -Ttext 0x1000 "temp/kernel_entry.o" "temp/kernel.o" --oformat binary -nostdlib

#concatenate
cat "temp/boot.bin" "temp/full_kernel.bin" > "OS.bin"

truncate -s 1474560 OS.bin

#run
qemu-system-x86_64 -drive format=raw,file="OS.bin",index=0,if=floppy -m 128M
