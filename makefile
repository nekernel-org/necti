FLAGS=-I./NDKKit -I./ -D__NDK_MODULE__ -I./NDKKit/Sources/Detail -fPIC -I./Private -shared -std=c++20 -Wl,--subsystem=17
CC=x86_64-w64-mingw32-g++.exe
OUT=ndk.dll

.PHONY: all
all:
	$(CC) $(FLAGS) $(wildcard NDKKit/Sources/*.cxx) -o $(OUT)
	@echo "[ndk.dll] => OK."
