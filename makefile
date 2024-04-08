 #
 #	========================================================
 #
 #	MPCC
 # 	Copyright Mahrouss Logic, all rights reserved.
 #
 # 	========================================================
 #

COMMON_INC=-I./Headers -I./ -I./Sources/Detail

ifeq ($(shell uname), "Darwin")
LINK_CC=g++ -std=c++20
else
LINK_CC=x86_64-w64-mingw32-g++ -std=c++20 -Xlinker -s
endif

WINRES=windres
LINK_SRC=Sources/link.cc
LINK_OUTPUT=Output/link.exe
LINK_ALT_OUTPUT=Output/64link.exe
LINK_ALT_3_OUTPUT=Output/i64link.exe
LINK_ALT_2_OUTPUT=Output/32link.exe

PP_SRC=Sources/bpp.cc
PP_OUTPUT=Output/bpp.exe

SRC_COMMON=Sources/String.cc Sources/AsmKit.cc

# C Compiler (PowerPC)
64X0_CC_SRC=Sources/64x0-cc.cc $(SRC_COMMON)
64X0_CC_OUTPUT=Output/64x0-cc.exe

# C Compiler
PPC_CC_SRC=Sources/ppc-cc.cc $(SRC_COMMON)
PPC_CC_OUTPUT=Output/ppc-cc.exe

# 64x0 Assembler
ASM_SRC=Sources/64asm.cc $(SRC_COMMON)
ASM_OUTPUT=Output/64asm.exe

# AMD64 Assembler
IASM_SRC=Sources/i64asm.cc $(SRC_COMMON)
IASM_OUTPUT=Output/i64asm.exe

# PowerPC Assembler
PPCASM_SRC=Sources/ppcasm.cc $(SRC_COMMON)
PPCASM_OUTPUT=Output/ppcasm.exe

.PHONY: all
all: pre-processor compiler linker
	@echo "make: done."

.PHONY: pre-processor
pre-processor:
	$(LINK_CC) $(COMMON_INC) $(PP_SRC) -o $(PP_OUTPUT)

.PHONY: compiler
compiler:
	$(WINRES) i64asm.rsrc -O coff -o i64asm.obj
	$(WINRES) 64asm.rsrc -O coff -o 64asm.obj
	$(WINRES) ppcasm.rsrc -O coff -o ppcasm.obj
	$(WINRES) 64x0-cc.rsrc -O coff -o 64x0-cc.obj
	$(WINRES) ppc-cc.rsrc -O coff -o ppc-cc.obj
	$(LINK_CC) $(COMMON_INC) 64x0-cc.obj $(64X0_CC_SRC) -o $(64X0_CC_OUTPUT)
	$(LINK_CC) $(COMMON_INC) ppc-cc.obj $(PPC_CC_SRC) -o $(PPC_CC_OUTPUT)
	$(LINK_CC) $(COMMON_INC) i64asm.obj $(IASM_SRC) -o $(IASM_OUTPUT)
	$(LINK_CC) $(COMMON_INC) 64asm.obj $(ASM_SRC) -o $(ASM_OUTPUT)
	$(LINK_CC) $(COMMON_INC) ppcasm.obj $(PPCASM_SRC) -o $(PPCASM_OUTPUT)

.PHONY: linker
linker:
	$(WINRES) link.rsrc -O coff -o link.obj
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
	rm -f $(64X0_CC_OUTPUT)
	rm -f $(PPC_CC_OUTPUT)
	rm -f $(PP_OUTPUT)
	rm -f $(ASM_OUTPUT)
	rm -f $(IASM_OUTPUT)
	rm -f $(LINK_OUTPUT)

# Last rev 8-1-24
