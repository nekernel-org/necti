FLAGS=-I./NDKKit -I./ -D__NDK_MODULE__ -I./NDKKit/Sources/Detail -fPIC -I./Private -shared -std=c++20
CC=x86_64-w64-mingw32-g++.exe
OUT=libNDK.dll

.PHONY: build-lib
build-lib:
	$(CC) $(FLAGS) $(wildcard NDKKit/Sources/*.cxx) -o $(OUT)
	@echo "=> OK."
