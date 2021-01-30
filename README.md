# Inverse Mix Column with pre-calculated finite field example.
I've found plenty examples of forward Mix Column examples with pre-calculated
Galois field, but I couldn't find one for inverse Mix Column, so I created this repo.
The heart of this example lies in the excerpt below:
```c++
void inv_mix_column (uint8_t data [16])
{
	for (int i = 0 ; i < 4 ; i ++)
	{
		uint8_t a [4], b [4], c [4], d [4], h;

		for (int j = 0 ; j < 4 ; j ++)
		{
			a [j] = data [4 * i + j];
			
			h = (uint8_t) ((signed char) a [j] >> 7);
			b [j] = (a [j] << 1) ^ (0x1b & h);

			h = (uint8_t) ((signed char) b [j] >> 7);
			c [j] = (b [j] << 1) ^ (0x1b & h);

			h = (uint8_t) ((signed char) c [j] >> 7);
			d [j] = (c [j] << 1) ^ (0x1b & h) ^ a [j];
		}
		
		data [4 * i + 0] = (d [0] ^ c [0] ^ b [0] ^ a [0]) ^ (d [1] ^ b [1]) ^ (d [2] ^ c [2]) ^ d [3];
		data [4 * i + 1] = d [0] ^ (d [1] ^ c [1] ^ b [1] ^ a [1]) ^ (d [2] ^ b [2]) ^ (d [3] ^ c [3]);
		data [4 * i + 2] = (d [0] ^ c [0]) ^ d [1] ^ (d [2] ^ c [2] ^ b [2] ^ a [2]) ^ (d [3] ^ b [3]);
		data [4 * i + 3] = (d [0] ^ b [0]) ^ (d [1] ^ c [1]) ^ d [2] ^ (d [3] ^ c [3] ^ b [3] ^ a [3]);
	}
}
```
Essentially, ```d[n]``` is ```9*a[n]```, ```c[n]``` is ```4*a[n]```, and ```b[n]``` is ```2*a[n]```. 
It simply adds and/or substracts (using ```xor```) elements of these arrays to get ```14*a[n]```, ```13*a[n]```, ```11*a[n]```, and ```9*a[n]```.

```signed char``` is useful because we can broadcast an entire 8-bit variable with ```1```s when right shift is applied.

It uses AES-NI's AESIMC instruction to verify the results of both pre-calculator function and regular function.
# Sample result
## Hardware Spec.
```c++
CPU: Intel(R) Core(TM) i7-9750H CPU @ 2.60GHz 2.60
RAM: 16.GB (15.8GB Usable)
OS : Windows 10 Education, 1909, 64-bit operating system, x64-based processor.
     Ubuntu 20.04 LTS, WSL2
```
## Command Lines
```
nasm -f elf64 -o invmc.o invmc.asm
clang++ -std=c++20 -O3 -o invmc invmc.cpp invmc.o
./invmc
```
## Result:
```
Block size  : 16
Block count : 128
Vector size : 2048
Inverse Mix Column (AES-NI): 0.3μs
Inverse Mix Column (Pre): 4.5μs
Inverse Mix Column (GMul): 5.5μs
```
