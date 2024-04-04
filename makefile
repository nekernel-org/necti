 #
 #	========================================================
 #
 #	MPCC
 # 	Copyright Mahrouss Logic, all rights reserved.
 #
 # 	========================================================
 #

COMMON_INC=-IHeaders -I./

ifeq ($(shell uname), "Darwin")
LINK_CC=g++ -std=c++20
else
LINK_CC=x86_64-w64-mingw32-g++ -std=c++20
endif

LINK_SRC=Sources/link.cc
LINK_OUTPUT=Output/link.exe
LINK_ALT_OUTPUT=Output/64link.exe
LINK_ALT_3_OUTPUT=Output/i64link.exe
LINK_ALT_2_OUTPUT=Output/32link.exe

PP_SRC=Sources/bpp.cc
PP_OUTPUT=Output/bpp.exe

SRC_COMMON=Sources/String.cc Sources/AsmKit.cc

# C Compiler
CC_SRC=Sources/cc.cc $(SRC_COMMON)
CC_OUTPUT=Output/cc.exe

# 64x0 Assembler
ASM_SRC=Sources/64asm.cc $(SRC_COMMON)
ASM_OUTPUT=Output/64asm.exe

# AMD64 Assembler
IASM_SRC=Sources/i64asm.cc $(SRC_COMMON)
IASM_OUTPUT=Output/i64asm.exe

.PHONY: all
all: pre-processor compiler linker
	@echo "make: done."

.PHONY: pre-processor
pre-processor:
	$(LINK_CC) $(COMMON_INC) $(PP_SRC) -o $(PP_OUTPUT)

.PHONY: compiler
compiler:
	windres i64asm.rsrc -O coff -o i64asm.obj
	windres 64asm.rsrc -O coff -o 64asm.obj
	$(LINK_CC) $(COMMON_INC) $(CC_SRC) -o $(CC_OUTPUT)
	$(LINK_CC) $(COMMON_INC) i64asm.obj $(IASM_SRC) -o $(IASM_OUTPUT)
	$(LINK_CC) $(COMMON_INC) 64asm.obj $(ASM_SRC) -o $(ASM_OUTPUT)

.PHONY: linker
linker:
	windres link.rsrc -O coff -o link.obj
	$(LINK_CC) $(COMMON_INC) link.obj $(LINK_SRC) -o $(LINK_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_2_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_3_OUTPUT)

.PHONY: help
help:
	@echo "Compiler 	- Mahrouss Compilers."
	@echo "Preprocessor 	- Mahrouss Preprocessors."
	@echo "linker 		- Mahrouss Linkers."
	@echo "clean 		- Clean garbage."

.PHONY: clean
clean:
	rm -f $(CC_OUTPUT)
	rm -f $(PP_OUTPUT)
	rm -f $(ASM_OUTPUT)
	rm -f $(IASM_OUTPUT)
	rm -f $(LINK_OUTPUT)

# Last rev 8-1-24
