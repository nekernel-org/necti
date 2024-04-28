# Path: Examples/ExampleCDialect.c
# Language: POWER Assembly (Generated from C)
# Build Date: 2024-4-28







dword export .code64 __ImageStart


	li r3,0x1000

	cmpw r10, r11
	beq import __MPCC_IF_PROC_6099380296 
dword export .code64 __MPCC_IF_PROC_6099380296



	mr r31, r3
	blr


	mr r31, r0
	blr

