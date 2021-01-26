all:
	nasm -f elf64 -o invmc.o invmc.asm
	clang++ -std=c++20 -O3 -o invmc invmc.cpp invmc.o