[org 0x7e00]                        
KERNEL_LOCATION equ 0x1000

MODE_INFO_BLOCK equ 0x8000
BOOT_DISK: db0

; save boot drive
mov [BOOT_DISK], dl

; setup real mode
xor ax, ax                          
mov ds, ax
mov ss, ax
mov sp, 0x7000


; disk read kernel (32 sectors)
mov ax, 0x0000
mov es, ax
mov bx, KERNEL_LOCATION

mov ah, 0x02
mov al, 32 ; sectors 
mov ch, 0
mov dh, 0x00
mov cl, 10 ; start at sector 10
mov dh, 0
mov dl, [BOOT_DISK]

int 0x13                
jc disk_error



push ds
pop es

mov ax, 0x4F01
mov cx, 0x118
mov di, mode_info_block
int 0x10

cmp ax, 0x004F
jne vbe_error

mov ax, 0x4F02
mov bx, 0x4118
int 0x10

cmp ax, 0x004F
jne vbe_error

mov eax, [mode_info_block + 0x28]
mov [FRAMEBUFFER_ADDR], eax

mov eax, [0x8000 + 0x28]
mov [FRAMEBUFFER_PTR], eax



CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

cli
lgdt [GDT_descriptor]
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE_SEG:start_protected_mode

jmp $
mode_info_block:
    times 256 db 0

disk_error:
    mov ah, 0x0e
    mov al, 'e'
    int 0x10
    jmp $
BOOT_DISK: db 0

vbe_error:
    mov ah, 0x0e
    mov al, 'v'
    int 0x10
    jmp $

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0

    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start


[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov ebp, 0x90000		
	mov esp, ebp
    
    mov eax, [FRAMEBUFFER_PTR]
    jmp KERNEL_LOCATION
