#!/bin/bash
set -e
clear

echo "== Cleaning/build folder =="
rm -rf "build os.img"
mkdir -p build

echo "== Assembling bootloader (sector 0) =="
nasm -f bin "code/boot.asm" -o "build/boot.bin"

echo "== Assembling stage2 (sector 1) =="
nasm -f bin "code/stage2.asm" -o "build/stage2.bin"

echo "== Assembling kernel entry =="
nasm -f elf32 "code/kernel_entry.asm" -o "build/kernel_entry.o"

echo "== Compiling C code =="
i386-elf-gcc -ffreestanding -m32 -g -c "code/kernel.c" -o "build/kernel.o"

echo "== Linking kernel (entry + C kernel) =="
i386-elf-gcc -m elf_i386 \
    -Ttext 0x1000 \
    "build/kernel_entry.o" "build/kernel.o" \
    -o "build/kernel.bin" \
    --oformat binary \
    --nostdlib

echo "== Creating raw disk image (64MB) =="
dd if=/dev/zero of=disk.img bs=1M count=64 status=none

echo "== Writing bootloader (sector 0) =="
dd if="build/boot.bin" of="disk.img" conv=notrunc status=none

echo "== Writing stage2 (sector 1) ==""
dd if="build/stage2.bin" of="disk.img" bs-1M seek=1 conv=notrunc status=none

echo "== Writing kernel (sector 10) ==''
dd if="build/stage2.bin" of="disk.img" bs-1M seek=10 conv=notrunc status=none

qemu-system-i386 -drive format=raw,file=disk.img -m 128M