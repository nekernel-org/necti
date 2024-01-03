 #
 #	========================================================
 #
 #	C++Kit
 # 	Copyright Western Company, all rights reserved.
 #
 # 	========================================================
 #

LINK_CC=g++ -std=c++20
LINK_INC=-I../ -I../C++Kit
LINK_SRC=ld.cxx
LINK_OUTPUT=bin/ld

PP_SRC=cpp.cxx
PP_OUTPUT=bin/cpp

CC2_OUTPUT=bin/cppfront
CC2_SRC=cc2/source/cppfront.cpp

CC_SRC=ccplus.cxx ../C++Kit/StdKit/*.cpp ../C++Kit/AsmKit/*.cpp
CC_OUTPUT=bin/ccplus

CC_SRC=cc.cxx ../C++Kit/StdKit/*.cpp ../C++Kit/AsmKit/*.cpp
CC_OUTPUT=bin/cc

CXX_SRC=ccplus.cxx ../C++Kit/StdKit/*.cpp ../C++Kit/AsmKit/*.cpp
CXX_OUTPUT=bin/ccplus

MASM_SRC=masm.cxx ../C++Kit/StdKit/*.cpp ../C++Kit/AsmKit/*.cpp
MASM_OUTPUT=bin/masm

.PHONY: all
all: cxx
	@echo "[make] done build"

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