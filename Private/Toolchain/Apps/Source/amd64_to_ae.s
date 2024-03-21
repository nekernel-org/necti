#bits 16
#org 0x7c00

export .text BIOSStartup
    mov ax, cx
    cli
    hlt
    jmp 0x7c00