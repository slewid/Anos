[bits 16]
[org 0x7e00] 
mov ah, 0x0e
mov al, 'S'
int 0x10

KERNEL_LOCATION equ 0x1000

MODE_INFO_BLOCK equ 0x8000
BOOT_DISK: db 0

; --------------------
; setup real mode
; --------------------
xor ax, ax                          
mov ds, ax
mov ss, ax
mov sp, 0x9000

; --------------------
; save boot drive
; --------------------
mov [BOOT_DISK], dl

; --------------------
; disk read kernel (32 sectors)
; --------------------
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

cmp ah, 0
jne disk_error
; --------------------
; get VBE mode info
; --------------------
mov ax, 0x4F00
mov di, MODE_INFO_BLOCK

push es
xor bx, bx
mov es, bx

; prepare buffer
mov dword [MODE_INFO_BLOCK], 'VBE2'

xor ax, ax
mov es, ax
mov di, MODE_INFO_BLOCK

mov ax, 0x4F00
int 0x10

jc vbe_error_1
cmp ax, 0x004F
jne vbe_error_1

; now check BIOS-filled signature
cmp dword [MODE_INFO_BLOCK], 'VESA'
jne vbe_error_2

mov ax, 0x4F01
mov cx, 0x118
mov di, MODE_INFO_BLOCK

push es
xor bx, bx
mov es, bx

int 0x10

pop es

jc vbe_error_3
cmp ax, 0x004F
jne vbe_error_3

mov ax, [MODE_INFO_BLOCK + 0x00]
test ax, 0x0080
jz vbe_error_3

mov eax, [MODE_INFO_BLOCK + 0x28]
test eax, eax
jz vbe_error_3



; --------------------
; set vbe mode
; --------------------
mov ax, 0x4F02
mov bx, 0x4118

push es
xor dx, dx
mov es, dx

int 0x10

pop es
jc vbe_error_3
cmp ax, 0x004F
jne vbe_error_3


; framebuffer pointer
mov eax, [MODE_INFO_BLOCK + 0x28]
mov [FRAMEBUFFER_ADDR], eax

; --------------------
; protected mode setup
; --------------------
in al, 0x92
or al, 2
out 0x92, al

cli

lgdt [GDT_descriptor]

mov eax, cr0
or eax, 1
mov cr0, eax

jmp CODE_SEG:start_protected_mode

; --------------------
; errors
; --------------------
disk_error:
    mov ah, 0x0e
    mov al, 'e'
    int 0x10
    jmp $

vbe_error_1:
    mov ah, 0x0e
    mov al, '1'
    int 0x10
    jmp $

vbe_error_2:
    mov ah, 0x0e
    mov al, '2'
    int 0x10
    jmp $

vbe_error_3:
    mov ah, 0x0e
    mov al, '3'
    int 0x10
    jmp $

; --------------------
; GDT
; --------------------
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

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

; --------------------
; protected mode entry
; --------------------
[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	
	mov ebp, 0x90000		
	mov esp, ebp
    
    BOOT_INFO equ 0x9000

    ; framebuffer (32-bit pointer)
    mov eax, [  MODE_INFO_BLOCK + 0x28]
    mov [BOOT_INFO + 0], eax

    ; pitch (16-bit → zero extended)
    movzx eax, word [MODE_INFO_BLOCK + 0x10]
    mov [BOOT_INFO + 12], eax

    ; width
    movzx eax, word [MODE_INFO_BLOCK + 0x12]
    mov [BOOT_INFO + 4], eax

    ; height
    movzx eax, word [MODE_INFO_BLOCK + 0x14]
    mov [BOOT_INFO + 8], eax

    ; bpp
    xor eax, eax
    mov al, [MODE_INFO_BLOCK + 0x19]
    mov [BOOT_INFO + 16], eax

    mov eax, BOOT_INFO
    jmp CODE_SEG:KERNEL_LOCATION


times 512 db 0
FRAMEBUFFER_ADDR: dd 0