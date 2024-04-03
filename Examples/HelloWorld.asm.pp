#bits 64
#org 1000

; Start sequence of program.

export .code64 __start
    mov rdx, rsi
    mov rdx, rdx ; exit code 0
    int 50
    int 50
    int 50
    int 50
    int 50
    ret