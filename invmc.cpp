/* 
 * MIT License
 *
 * Copyright (c) 2020 Eunbin Bak
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <cstring>
#include <ciso646>

std::ostream & operator << (std::ostream & os, uint8_t data [16])
{
	for (int i = 0 ; i < 4 ; i ++)
	{
		std::cout << "0x";
		std::cout << std::hex << std::setw (2) << std::setfill ('0') << (int) data [i * 4 + 0] << ", 0x";
		std::cout << std::hex << std::setw (2) << std::setfill ('0') << (int) data [i * 4 + 1] << ", 0x";
		std::cout << std::hex << std::setw (2) << std::setfill ('0') << (int) data [i * 4 + 2] << ", 0x";
		std::cout << std::hex << std::setw (2) << std::setfill ('0') << (int) data [i * 4 + 3];

		if (i < 3)
		{
			std::cout << ", " << std::endl;
		}
	}

	return os;
}

extern "C" bool check_aes_ni_support ();
extern "C" void asm_inv_mix_column (uint8_t data [16]);

// Precaculated Galois Field.
void pre_inv_mix_column (uint8_t data [16])
{
	for (int i = 0 ; i < 4 ; i ++)
	{
		uint8_t a [4], b [4], c [4], d [4], h;

		memcpy (a, data + 4 * i, 4);

		for (int j = 0 ; j < 4 ; j ++)
		{
			// 0xff if a[j]&0x80 != 0, 0x00 otherwise
			h = (uint8_t) ((signed char) a [j] >> 7);
			// b[j] = 2*a[j]
			b [j] = (a [j] << 1) ^ (0x1b & h);

			h = (uint8_t) ((signed char) b [j] >> 7);
			// c[j] = 2*b[j] = 4*a[j]
			c [j] = (b [j] << 1) ^ (0x1b & h);

			h = (uint8_t) ((signed char) c [j] >> 7);
			// d[j] = 2*c[j] + a[j] = 8*a[n] + a[n] = 9*a[n]
			d [j] = (c [j] << 1) ^ (0x1b & h) ^ a [j];
		}
		
		// [d0] = [14 11 13  9] * [a0]
		// [d1] = [ 9 14 11 13] * [a1]
		// [d2] = [13  9 14 11] * [a2]
		// [d3] = [11 13  9 14] * [a3]
		data [4 * i + 0] = (d [0] ^ c [0] ^ b[0] ^ a [0]) ^ (d [1] ^ b [1]) ^ (d [2] ^ c [2]) ^ d [3];
		data [4 * i + 1] = d [0] ^ (d [1] ^ c [1] ^ b [1] ^ a [1]) ^ (d [2] ^ b [2]) ^ (d [3] ^ c [3]);
		data [4 * i + 2] = (d [0] ^ c [0]) ^ d [1] ^ (d [2] ^ c [2] ^ b [2] ^ a [2]) ^ (d [3] ^ b [3]);
		data [4 * i + 3] = (d [0] ^ b [0]) ^ (d [1] ^ c [1]) ^ d [2] ^ (d [3] ^ c [3] ^ b [3] ^ a [3]);
	}
}

// Original with a full Galois Multiplication function.
void gml_inv_mix_column (uint8_t data [16])
{
	auto gmul = [] (uint8_t a, uint8_t b)->uint8_t
	{
		uint8_t c = 0;

		for (int i = 0 ; i < 8 ; i ++)
		{
			if ((b & 1) != 0)
				c ^= a;
			
			uint8_t h = (uint8_t) ((signed char) a >> 7);

			a = (a << 1) ^ (0x1b & h);
			b >>= 1;
		}

		return c;
	};

	for (int i = 0 ; i < 4 ; i ++)
	{
		uint8_t a [4];

		memcpy (a, data + i * 4, 4);
		
		// [d0] = [14 11 13  9] * [a0]
		// [d1] = [ 9 14 11 13] * [a1]
		// [d2] = [13  9 14 11] * [a2]
		// [d3] = [11 13  9 14] * [a3]
		data [i * 4 + 0] = gmul (14, a [0]) ^ gmul (11, a [1]) ^ gmul (13, a [2]) ^ gmul (9, a [3]);
		data [i * 4 + 1] = gmul (9, a [0]) ^ gmul (14, a [1]) ^ gmul (11, a [2]) ^ gmul (13, a [3]);
		data [i * 4 + 2] = gmul (13, a [0]) ^ gmul (9, a [1]) ^ gmul (14, a [2]) ^ gmul (11, a [3]);
		data [i * 4 + 3] = gmul (11, a [0]) ^ gmul (13, a [1]) ^ gmul (9, a [2]) ^ gmul (14, a [3]);
	}
}

int main (int argc, char * argv [])
{
	if (not check_aes_ni_support ())
	{
		std::cerr << "Cannot test without AES-NI." << std::endl;
		return 1;
	}
	
	uint8_t input [16], aesni [16], pre [16], gmul [16];

	std::random_device rd;
	std::mt19937 mt (rd ());
	std::uniform_int_distribution <int> dist (0, 255);

	for (int i = 0 ; i < 16 ; i ++)
		input [i] = (uint8_t) dist (mt);

	memcpy (aesni, input, 16);
	memcpy (pre, input, 16);
	memcpy (gmul, input, 16);

	std::cout << "Test Vector: " << std::endl;
	std::cout << input << std::endl << std::endl;

	auto t0 = std::chrono::high_resolution_clock::now ();
	asm_inv_mix_column (aesni);
	auto t1 = std::chrono::high_resolution_clock::now ();
	pre_inv_mix_column (pre);
	auto t2 = std::chrono::high_resolution_clock::now ();
	gml_inv_mix_column (gmul);
	auto t3 = std::chrono::high_resolution_clock::now ();

	auto duration = std::chrono::duration_cast <std::chrono::nanoseconds> (t1 - t0).count ();

	std::cout << "Inverse Mix Column (AES-NI): " << std::dec << (duration / 1000.0) << "μs" << std::endl;
	std::cout << aesni << std::endl << std::endl;

	duration = std::chrono::duration_cast <std::chrono::nanoseconds> (t2 - t1).count ();

	std::cout << "Inverse Mix Column (Pre): " << std::dec << (duration / 1000.0) << "μs" << std::endl;
	std::cout << pre << std::endl << std::endl;

	duration = std::chrono::duration_cast <std::chrono::nanoseconds> (t3 - t2).count ();

	std::cout << "Inverse Mix Column (GMul): " << std::dec << (duration / 1000.0) << "μs" << std::endl;
	std::cout << gmul << std::endl << std::endl;

	if (memcmp (aesni, pre, 16) != 0)
	{
		std::cerr << "Results of AES-NI and Pre do not match!" << std::endl;
		return 2;
	}

	if (memcmp (pre, gmul, 16) != 0)
	{
		std::cerr << "Results of Pre and GMul do not match!" << std::endl;
		return 3;
	}

	std::cout << "Successful." << std::endl;

	return 0;
}
