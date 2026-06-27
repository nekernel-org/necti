# Linking Nectar on Linux:

## Notice:

You will need:

`ld -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc inner.o -o a.out`

As The Linux Kernel will **not** let you load dynamic libraries without a loader.
