 #
 #	========================================================
 #
 #	C++Kit
 # 	Copyright Mahrouss Logic, all rights reserved.
 #
 # 	========================================================
 #

LINK_CC=g++ -std=c++20
LINK_INC=-I../ -I../CompilerKit
LINK_SRC=ld.cc
LINK_OUTPUT=bin/ld
LINK_ALT_OUTPUT=bin/mld

PP_SRC=bpp.cc
PP_OUTPUT=bin/bpp

CC2_OUTPUT=bin/cppfront
CC2_SRC=cc2/source/cppfront.cpp

# BC Compiler
CC_SRC=bccl.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
CC_OUTPUT=bin/bccl

# C++ Compiler
CXX_SRC=ccplus.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
CXX_OUTPUT=bin/ccplus

# MP-UX Assembly
MASM_SRC=masm.cc ../CompilerKit/StdKit/*.cc ../CompilerKit/AsmKit/*.cc
MASM_OUTPUT=bin/masm

.PHONY: all
all: cxx
	@echo "[make] done build."

.PHONY: cxx
cxx: ld
	$(LINK_CC) $(LINK_INC) $(PP_SRC) -o $(PP_OUTPUT)
	$(LINK_CC) $(LINK_INC) $(CC_SRC) -o $(CC_OUTPUT)
	$(LINK_CC) $(LINK_INC) $(CC2_SRC) -o $(CC2_OUTPUT)
	$(LINK_CC) $(LINK_INC) $(CXX_SRC) -o $(CXX_OUTPUT)
	$(LINK_CC) $(LINK_INC) $(MASM_SRC) -o $(MASM_OUTPUT)

.PHONY: ld
ld:
	$(LINK_CC) $(LINK_INC) $(LINK_SRC) -o $(LINK_OUTPUT)
	cp $(LINK_OUTPUT) $(LINK_ALT_OUTPUT)

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