[org 0x7c00]

mov ax, 65535
call printInt

jmp $

printInt:
    mov cx, 0

.nextDigit:
    mov dx, 0
    mov bx, 10
    div bx

    push dx
    inc cx

    cmp ax, 0
    jne nextDigit

printDigits:
    pop dx

    add dl, '0'
    mov ah, 0x0e
    mov al, dl
    int 0x10
    dec cx
    cmp cx, 0
    jne printDigits

    ret
times 510-($-$$) db 0
dw 0xaa55