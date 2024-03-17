@bits 64 
@org 0x1000

export .text main
    mov rcx, r8
    mov rax, rcx
    
    retf

export .text foo
    mov rdx, rcx
    mov rax, rcx

    retf