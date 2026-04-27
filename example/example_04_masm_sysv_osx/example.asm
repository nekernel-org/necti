%bits 64
public_segment .code64 main

; This example shows how to write "Hello World" to stdout using
; OS X syscall interface in x86-64 assembly (NeKernel syntax).

mov [rsp+0],  0x48
mov [rsp+1],  0x65
mov [rsp+2],  0x6C
mov [rsp+3],  0x6C
mov [rsp+4],  0x6F
mov [rsp+5],  0x20
mov [rsp+6],  0x57
mov [rsp+7],  0x6F
mov [rsp+8],  0x72
mov [rsp+9],  0x6C
mov [rsp+10], 0x64
mov [rsp+11], 0x0A

mov rax, 0x2000004
mov rdi, 0x01
lea rsi, [rsp]
mov rdx, 0x0C
syscall

mov rax, 0x2000001
xor rdi, rdi
syscall

ret