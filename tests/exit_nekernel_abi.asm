#bits 64
#org 0x40000000

public_segment .code64 __ImageStart
    xor rax, rax
    mov rcx, 1 ;; syscall id
    mov rdx, 0 ;; arg1
    syscall ;; exit
    mov rax, rbx
    ret