[org 0x7c00]

mov [BOOT_DRIVE], dl

; prints B to screen if correctly booted first stage
mov ah, 0x0e
mov al, 'B'
int 0x10

; newline
mov ah, 0x0e
mov al, 13
int 0x10
mov al, 10
int 0x10

; stack
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

; load sector from disk
mov ax, 0x0000
mov es, ax
mov bx, 0x7e00

mov ah, 0x02
mov al, 1 ; CHANGE TO 4
mov ch, 0
mov cl, 2
mov dh, 0

mov dl, [BOOT_DRIVE]
int 0x13

jc disk_error
mov dl, [BOOT_DRIVE]
; jump to loaded sector
jmp 0x0000:0x7e00

; print E if sector not loaded
disk_error:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $

BOOT_DRIVE: db 0

times 510-($-$$) db 0
dw 0xaa55