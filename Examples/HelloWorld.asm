#bits 64
#org 1000

; Start sequence of program.

%def gdtBase 0x1000
%def gdtLimit 0x100

export .code64 __start
    mov rdx, rcx ; exit program
    mov rdx, rdx ; exit code 0
    int 50
    int 50
    int 50
    int 50
    int 50
    ret
