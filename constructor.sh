#!/bin/bash
set -e

#compile asm
nasm "code/kernel_entry.asm" -f elf -o "obj/kernel_entry.o"
nasm -f bin "code/boot.asm" -o "bin/boot.bin"
nasm -f bin "code/zeroes.asm" -o "bin/zeroes.bin"

#compile c
i386-elf-gcc -ffreestanding -m32 -g -c "code/kernel.c" -o "obj/kernel.o"

#link
i386-elf-ld -o "bin/full_kernel.bin" -Ttext 0x7e00 "obj/kernel_entry.o" "obj/kernel.o" --oformat binary

#concatenate
cat "bin/boot.bin" "bin/full_kernel.bin" > "bin/everything.bin"
cat "bin/everything.bin" "bin/zeroes.bin" > "bin/OS.bin"

#run
qemu-system-x86_64 -drive format=raw,file="OS.bin",index=0,if=floppy -m 128M
