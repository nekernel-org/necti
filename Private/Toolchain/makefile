 #
 #	========================================================
 #
 #	MPCC
 # 	Copyright 2024, Mahrouss Logic, all rights reserved.
 #
 # 	========================================================
 #

COMMON_INC=-I../ -I../CompilerKit

LINK_CC=g++ -std=c++20

LINK_SRC=ld.cc
LINK_OUTPUT=bin/ld
LINK_ALT_OUTPUT=bin/64ld
LINK_ALT_2_OUTPUT=bin/32ld

PP_SRC=bpp.cc
PP_OUTPUT=bin/bpp

# BC Compiler
CC_SRC=bccl.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
CC_OUTPUT=bin/bccl

# C++ Compiler
CXX_SRC=ccplus.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
CXX_OUTPUT=bin/ccplus

# Assembler
ASM_SRC=64asm.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
ASM_OUTPUT=bin/64asm

IASM_SRC=i64asm.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
IASM_OUTPUT=bin/i64asm

.PHONY: all
all: cl
	@echo "[make] done build."

.PHONY: cl
cl: ld
	$(LINK_CC) $(COMMON_INC) $(PP_SRC) -o $(PP_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(CC_SRC) -o $(CC_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(CXX_SRC) -o $(CXX_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(IASM_SRC) -o $(IASM_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(ASM_SRC) -o $(ASM_OUTPUT)

.PHONY: ld
ld:
	$(LINK_CC) $(COMMON_INC) $(LINK_SRC) -o $(LINK_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_2_OUTPUT)

.PHONY: help
help:
	@echo "cxx - C compiler driver"
	@echo "ld - Linker"

.PHONY: clean
clean:
	rm bin/$(MKCDFS_OUTPUT)
	rm bin/$(CC_OUTPUT)
	rm bin/$(PP_OUTPUT)
	rm bin/$(LINK_OUTPUT)

# Last rev 8-1-24
