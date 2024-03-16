 #
 #	========================================================
 #
 #	MPCC
 # 	Copyright Mahrouss Logic, all rights reserved.
 #
 # 	========================================================
 #

COMMON_INC=-I../ -I../CompilerKit -I./

ifeq ($(shell uname), "Darwin")
LINK_CC=g++ -std=c++20
else
LINK_CC=x86_64-w64-mingw32-g++ -std=c++20
endif

LINK_SRC=link.cc
LINK_OUTPUT=bin/link.exe
LINK_ALT_OUTPUT=bin/64link.exe
LINK_ALT_3_OUTPUT=bin/i64link.exe
LINK_ALT_2_OUTPUT=bin/32link.exe

PP_SRC=bpp.cc
PP_OUTPUT=bin/bpp.exe

# C Compiler
CC_SRC=cc.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
CC_OUTPUT=bin/cc.exe

# C++ Compiler
CXX_SRC=ccplus.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
CXX_OUTPUT=bin/ccplus.exe

# Assembler
ASM_SRC=64asm.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
ASM_OUTPUT=bin/64asm.exe

IASM_SRC=i64asm.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
IASM_OUTPUT=bin/i64asm.exe

.PHONY: all
all:
	@echo "Use a specific target."

.PHONY: pp
pp:
	$(LINK_CC) $(COMMON_INC) $(PP_SRC) -o $(PP_OUTPUT)

.PHONY: cl
cl:
	$(LINK_CC) $(COMMON_INC) $(CC_SRC) -o $(CC_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(CXX_SRC) -o $(CXX_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(IASM_SRC) -o $(IASM_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(ASM_SRC) -o $(ASM_OUTPUT)

.PHONY: link
link:
	$(LINK_CC) $(COMMON_INC) $(LINK_SRC) -o $(LINK_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_2_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_3_OUTPUT)

.PHONY: help
help:
	@echo "cl - Mahrouss C compiler and Assemblers."
	@echo "pp - Mahrouss Preprocessors."
	@echo "link - Mahrouss Linkers."

.PHONY: clean
clean:
	rm bin/$(MKCDFS_OUTPUT)
	rm bin/$(CC_OUTPUT)
	rm bin/$(PP_OUTPUT)
	rm bin/$(LINK_OUTPUT)

# Last rev 8-1-24
