# Path: Examples/ExampleCDialect.c
# Language: MultiProcessor Assembly (Generated from C)
# Build Date: 2024-3-27

dword export .text main


	lda r2,0x1000

	lda r12, import __MPCC_IF_PROC_462562513200
	#r12 = Code to jump on, r11 right cond, r10 left cond.
	beq r10, r11, r12
dword export .text __MPCC_IF_PROC_462562513200


	ldw r19,  r2
	jlr


	ldw r19,  57
	jlr

