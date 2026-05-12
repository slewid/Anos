[bits 32]

global _start
extern main

section .text

_start:

    ; -------------------------
    ; set up stack
    ; -------------------------
    mov esp, stack_top
    mov ebp, esp

    ; optional: clear direction flag
    cld

    ; -------------------------
    ; call C kernel
    ; -------------------------
    call main

    ; -------------------------
    ; if kernel returns, halt safely
    ; -------------------------
.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384   ; 16 KB stack
stack_top: