# Path: LibC++/__power64.inc
# Language: CompilerKit POWER Assembly support for GNU.
# Build Date: 2024-6-4

#ifdef __LIBCOMPILER__

#ifdef __ASSEMBLER__

#define lda li
#define sta stw
#define ldw li

#define r0 0
#define r1 1
#define r2 2
#define r3 3
#define r4 4
#define r5 5
#define r6 6
#define r7 7
#define r8 8
#define r9 9
#define r10 10
#define r11 11
#define r12 12
#define r13 13
#define r14 14
#define r15 15
#define r16 16
#define r17 17
#define r18 18
#define r19 19
#define r20 20

#define nop mr 0, 0

#endif

#endif
