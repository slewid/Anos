[org 0x7e00]
[BITS 16]

mov ah, 0x0e
mov bx, string

print:
    mov al, [bx]
    cmp al, 0
    je exit
    int 0x10
    inc bx
    jmp print

string:
    db "Hello, World!", 0

exit:
    jmp $

times 510-($-$$) db 0
db 0x55, 0xAA