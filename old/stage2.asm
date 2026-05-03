[org 0x7e00]
[BITS 16]

mov bx, string
loop:
    mov ah, 0
    int 0x16

    cmp al, 13
    je endInput

    cmp bx, string + 9
    jge loop

    mov [bx], al
    inc bx

    mov ah, 0x0e
    int 0x10
    jmp loop

endInput:
    mov byte [bx], 0
    jmp printLn

printLn:
    mov ah, 0x0e
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10
    mov bx, string
    jmp print

print:
    mov al, [bx]
    cmp al, 0
    je exit

    int 0x10
    inc bx
    jmp print

string:
    times 10 db 0 

exit:
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10

    mov bx, string
    jmp loop

times 510-($-$$) db 0
db 0x55, 0xAA