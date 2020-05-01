# Inverse Mix Column with pre-calculated finite field example.
I've found plenty examples of forward Mix Column examples with pre-calculated
Galois field, but I couldn't find one for inverse Mix Column, so I created this repo.
The heart of this example lies in the excerpt below:
```
for (int j = 0 ; j < 4 ; j ++)
{
	a [j] = input [4 * i + j];
	
	h = (uint8_t) ((signed char) a [j] >> 7);
	b [j] = (a [j] << 1) ^ (0x1b & h);

	h = (uint8_t) ((signed char) b [j] >> 7);
	c [j] = (b [j] << 1) ^ (0x1b & h);

	h = (uint8_t) ((signed char) c [j] >> 7);
	d [j] = (c [j] << 1) ^ (0x1b & h) ^ a [j];
}
```
It uses AES-NI's AESIMC instruction to verify the results of both pre-calculator function and regular function.
# Sample result
## Hardware Spec.
```c++
CPU: Intel(R) Core(TM) i5-8300H CPU @ 2.30GHz 2.30GHz
RAM: 16.GB (15.8GB Usable)
OS : Windows 10 Home, 64-bit operating system, x64-based processor.
     Ubuntu 18.04 LTS WSL.
```
## Command Lines
```
nasm -f elf64 -o invmc.o invmc.asm
g++ -std=c++17 -O3 -o invmc invmc.cpp invmc.o
```
## Result:
```
Test Vector:
0x0d, 0x60, 0xa7, 0x1f,
0x1e, 0xec, 0x21, 0x5b,
0x9f, 0xa2, 0x7e, 0xce,
0x11, 0x59, 0xd4, 0x2a

Inverse Mix Column (AES-NI): 1μs
0x6e, 0x35, 0x75, 0xfb,
0xb9, 0x23, 0x8d, 0x9f,
0xc5, 0x23, 0x25, 0x4e,
0xc9, 0xfe, 0x7b, 0xfa

Inverse Mix Column (Pre): 0.8μs
0x6e, 0x35, 0x75, 0xfb,
0xb9, 0x23, 0x8d, 0x9f,
0xc5, 0x23, 0x25, 0x4e,
0xc9, 0xfe, 0x7b, 0xfa

Inverse Mix Column (GMul): 1.2μs
0x6e, 0x35, 0x75, 0xfb,
0xb9, 0x23, 0x8d, 0x9f,
0xc5, 0x23, 0x25, 0x4e,
0xc9, 0xfe, 0x7b, 0xfa

Successful.
```
