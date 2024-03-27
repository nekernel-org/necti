#bits 64
#org 1000

; Start sequence of program.

%def gdtBase 0x1000
%def gdtLimit 0x100

export .data64 GDT
    .number gdtBase
    .number gdtLimit

export .code64 __start
    mov rcx, 47 ; exit program
    mov rdx, 0x0 ; exit code 0
    int 50
    int 50
    int 50
    int 50
    int 50
    ret
