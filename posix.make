 #
 #	========================================================
 #
 #	MPCC
 # 	Copyright ZKA Technologies, all rights reserved.
 #
 # 	========================================================
 #

COMMON_INC=-I./Headers -I./ -I./Sources/Detail
LINK_CC=clang++ -std=c++20
LINK_SRC=Sources/link.cxx
LINK_OUTPUT=Output/link
LINK_ALT_OUTPUT=Output/64link
LINK_ALT_3_OUTPUT=Output/i64link
LINK_ALT_2_OUTPUT=Output/32link
LINK_ALT_4_OUTPUT=Output/ppclink

PP_SRC=Sources/bpp.cxx
PP_OUTPUT=Output/bpp

SRC_COMMON=Sources/String.cxx Sources/AssemblyFactory.cxx

# C++ Compiler (AMD64)
AMD64_CXX_SRC=Sources/cplusplus.cxx $(SRC_COMMON)
AMD64_CXX_OUTPUT=Output/cplusplus

# C Compiler (POWER)
64X0_CC_SRC=Sources/64x0-cc.cxx $(SRC_COMMON)
64X0_CC_OUTPUT=Output/64x0-cc

# C Compiler (Our own RISC)
PPC_CC_SRC=Sources/power-cc.cxx $(SRC_COMMON)
PPC_CC_OUTPUT=Output/power-cc

# 64x0 Assembler (Our Own RISC)
ASM_SRC=Sources/64asm.cxx $(SRC_COMMON)
ASM_OUTPUT=Output/64asm

# AMD64 Assembler (Intel CISC)
IASM_SRC=Sources/i64asm.cxx $(SRC_COMMON)
IASM_OUTPUT=Output/i64asm

# Power4 Assembler (IBM RISC)
PPCASM_SRC=Sources/ppcasm.cxx $(SRC_COMMON)
PPCASM_OUTPUT=Output/ppcasm

.PHONY: all
all: pre-processor compiler linker
	@echo "make: done."

.PHONY: pre-processor
pre-processor:
	$(LINK_CC) $(COMMON_INC) $(PP_SRC) -o $(PP_OUTPUT)

.PHONY: compiler
compiler:
	$(LINK_CC) $(COMMON_INC) $(64X0_CC_SRC) -o $(64X0_CC_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(AMD64_CXX_SRC) -o $(AMD64_CXX_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(PPC_CC_SRC) -o $(PPC_CC_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(IASM_SRC) -o $(IASM_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(ASM_SRC) -o $(ASM_OUTPUT)
	$(LINK_CC) $(COMMON_INC) $(PPCASM_SRC) -o $(PPCASM_OUTPUT)

.PHONY: linker
linker:
	$(LINK_CC) $(COMMON_INC) $(LINK_SRC) -o $(LINK_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_2_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_3_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_4_OUTPUT)

.PHONY: help
help:
	@echo "compiler 	- ZKA Technologies Compiler Suite."
	@echo "pre-processor 	- ZKA Technologies Preprocessor Suite."
	@echo "linker 		- ZKA Technologies Linkers."
	@echo "clean 		- Clean objects and executables."

.PHONY: clean
clean:
	rm -f $(64X0_CC_OUTPUT)
	rm -f $(PPC_CC_OUTPUT)
	rm -f $(PP_OUTPUT)
	rm -f $(ASM_OUTPUT)
	rm -f $(IASM_OUTPUT)
	rm -f $(LINK_OUTPUT)
	rm -rf *.obj

# Last rev 8-1-24
