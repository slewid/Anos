[org 0x7e00]
[BITS 16]

mov ah, 0x0e
mov al, 'a'

loop:

    int 0x10

    add al, -31
    int 0x10
    add al, 32

    inc al
    cmp al, 'z' + 1
    je exit
    jmp loop

exit:
    jmp $

times 510-($-$$) db 0
db 0x55, 0xAA