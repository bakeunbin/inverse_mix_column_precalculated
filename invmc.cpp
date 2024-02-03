/* 
 * I, Eunbin Bak, am not reliable for anything, but this code
 * is free to use.
 */

#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
// #include <bit>
#include <cstring>
#include <ciso646>
#include <cassert>

extern "C" bool check_aes_ni_support ();
extern "C" void aesni_keygen (const uint8_t key [16], uint8_t object [176]);
extern "C" void full_aesni (uint8_t data [16], const uint8_t object [176]);

#define BLOCK_SIZE 16
#define BLOCK_COUNT 8192
#define VECTOR_SIZE (BLOCK_SIZE * BLOCK_COUNT)
#define AES_128_KEY_OBJ_SIZE (BLOCK_SIZE * 11)

namespace aes::soft
{
	uint8_t gmul (uint8_t a, uint8_t b) noexcept
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
	}
/*
	uint8_t ginv (uint8_t a) noexcept
	{
		uint16_t u1 = 0, u3 = 0x11b, v1 = 1, v3 = a;

		while (v3 != 0) 
		{
			uint16_t t1 = u1, t3 = u3;
			int8_t q = std::countl_zero (v3) - std::countl_zero (u3);

			if (q >= 0) 
			{
				t1 ^= v1 << q;
				t3 ^= v3 << q;
			}

			u1 = v1; u3 = v3;
			v1 = t1; v3 = t3;
		}

		return (uint8_t) (u1 >= 0x100 ? u1 ^ 0x11b : u1);
	}
*/
	const uint8_t inv_s_box [256] =
	{
		0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
		0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
		0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
		0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
		0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
		0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
		0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
		0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
		0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
		0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
		0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
		0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
		0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
		0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
		0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
		0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
	};

	void inv_sb (uint8_t * data) noexcept
	{
		for (int i = 0 ; i < 16 ; i ++)
		{
			// auto s = data [i];
			// data [i] = ginv (std::rotl (s, 1) ^ std::rotl (s, 3) ^ std::rotl (s, 6) ^ 0x05);
			data [i] = inv_s_box [data [i]];
		}
	}
	
	void inv_sr (uint8_t * data) noexcept
	{
		uint8_t temp [16];
		
		memcpy (temp, data, 16);
		
		data [ 0] = temp [ 0], data [ 1] = temp [13], data [ 2] = temp [10], data [ 3] = temp [ 7];
		data [ 4] = temp [ 4], data [ 5] = temp [ 1], data [ 6] = temp [14], data [ 7] = temp [11];
		data [ 8] = temp [ 8], data [ 9] = temp [ 5], data [10] = temp [ 2], data [11] = temp [15];
		data [12] = temp [12], data [13] = temp [ 9], data [14] = temp [ 6], data [15] = temp [ 3];
	}

	// Precaculated Galois Field.
	void inv_mc_pre (uint8_t * data) noexcept
	{
		for (int i = 0 ; i < 4 ; i ++)
		{	// b=2*a, c=4*a, d=9*a
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
			
			data [4 * i + 0] = (d [0] ^ c [0] ^ b[0] ^ a [0]) ^ (d [1] ^ b [1]) ^ (d [2] ^ c [2]) ^ d [3];
			data [4 * i + 1] = d [0] ^ (d [1] ^ c [1] ^ b [1] ^ a [1]) ^ (d [2] ^ b [2]) ^ (d [3] ^ c [3]);
			data [4 * i + 2] = (d [0] ^ c [0]) ^ d [1] ^ (d [2] ^ c [2] ^ b [2] ^ a [2]) ^ (d [3] ^ b [3]);
			data [4 * i + 3] = (d [0] ^ b [0]) ^ (d [1] ^ c [1]) ^ d [2] ^ (d [3] ^ c [3] ^ b [3] ^ a [3]);
		}
	}

	// Inverse Mix Column based on regualr GMul function.
	void inv_mc_gmul (uint8_t * data) noexcept
	{
		for (int i = 0 ; i < 4 ; i ++)
		{
			uint8_t a [4];

			memcpy (a, data + i * 4, 4);
			
			data [i * 4 + 0] = gmul (14, a [0]) ^ gmul (11, a [1]) ^ gmul (13, a [2]) ^ gmul (9, a [3]);
			data [i * 4 + 1] = gmul (9, a [0]) ^ gmul (14, a [1]) ^ gmul (11, a [2]) ^ gmul (13, a [3]);
			data [i * 4 + 2] = gmul (13, a [0]) ^ gmul (9, a [1]) ^ gmul (14, a [2]) ^ gmul (11, a [3]);
			data [i * 4 + 3] = gmul (11, a [0]) ^ gmul (13, a [1]) ^ gmul (9, a [2]) ^ gmul (14, a [3]);
		}
	}

	void ar (uint8_t * input, const uint8_t obj [AES_128_KEY_OBJ_SIZE], int round)
	{
		for (int i = 0 ; i < 16 ; i ++)
		{
			input [i] ^= obj [16 * round + i];
		}
	}
}

void mixed_precalc (uint8_t * data, const uint8_t obj [176])
{
	aes::soft::ar (data, obj, 0);

	for (int i = 1 ; i < 10; i ++)
	{
		aes::soft::inv_sb (data);
		aes::soft::inv_sr (data);
		aes::soft::inv_mc_pre (data);
		aes::soft::ar (data, obj, i);
	}
	
	aes::soft::inv_sb (data);
	aes::soft::inv_sr (data);
	aes::soft::ar (data, obj, 10);
}

void mixed_gmul (uint8_t * data, const uint8_t obj [176])
{
	aes::soft::ar (data, obj, 0);

	for (int i = 1 ; i < 10; i ++)
	{
		aes::soft::inv_sb (data);
		aes::soft::inv_sr (data);
		aes::soft::inv_mc_gmul (data);
		aes::soft::ar (data, obj, i);
	}
	
	aes::soft::inv_sb (data);
	aes::soft::inv_sr (data);
	aes::soft::ar (data, obj, 10);
}

auto decrypt_full_aesni (const uint8_t obj [AES_128_KEY_OBJ_SIZE], uint8_t (& output) [VECTOR_SIZE], const uint8_t input [VECTOR_SIZE])
{
	memcpy (output, input, VECTOR_SIZE);
	
	auto start = std::chrono::high_resolution_clock::now ();
	
	for (size_t i = 0 ; i < BLOCK_COUNT ; i ++)
		full_aesni (output + BLOCK_SIZE * i, obj);
	
	auto end = std::chrono::high_resolution_clock::now ();
	
	return std::chrono::duration_cast <std::chrono::nanoseconds> (end - start).count ();
}

auto decrypt_mixed_precalc (const uint8_t obj [AES_128_KEY_OBJ_SIZE], uint8_t (& output) [VECTOR_SIZE], const uint8_t input [VECTOR_SIZE])
{
	memcpy (output, input, VECTOR_SIZE);
	
	auto start = std::chrono::high_resolution_clock::now ();
	
	for (size_t i = 0 ; i < BLOCK_COUNT ; i ++)
		mixed_precalc (output + BLOCK_SIZE * i, obj);
	
	auto end = std::chrono::high_resolution_clock::now ();
	
	return std::chrono::duration_cast <std::chrono::nanoseconds> (end - start).count ();
}

auto decrypt_mixed_gmul (const uint8_t obj [AES_128_KEY_OBJ_SIZE], uint8_t (& output) [VECTOR_SIZE], const uint8_t input [VECTOR_SIZE])
{
	memcpy (output, input, VECTOR_SIZE);
	
	auto start = std::chrono::high_resolution_clock::now ();
	
	for (size_t i = 0 ; i < BLOCK_COUNT ; i ++)
		mixed_gmul (output + BLOCK_SIZE * i, obj);
	
	auto end = std::chrono::high_resolution_clock::now ();
	
	return std::chrono::duration_cast <std::chrono::nanoseconds> (end - start).count ();
}

int main (int argc, char * argv [])
{
	if (not check_aes_ni_support ())
	{
		std::cerr << "Cannot test without AES-NI." << std::endl;
		return 1;
	}

	std::random_device rd;
	std::mt19937 mt (rd ());
	std::uniform_int_distribution <uint32_t> dist (0, 255);
	
	// AES-ECB-128
	uint8_t key [BLOCK_SIZE], obj [AES_128_KEY_OBJ_SIZE];
	uint8_t input [VECTOR_SIZE];
	uint8_t fullni [VECTOR_SIZE], precalc [VECTOR_SIZE], gmul [VECTOR_SIZE];

	for (auto & k : key)
	{
		k = (uint8_t) dist (mt);
	}

	aesni_keygen (key, obj);
	
	for (auto & i : input)
	{
		i = (uint8_t) dist (mt);
	}

	auto fullni_t = decrypt_full_aesni (obj, fullni, input);
	auto precalc_t = decrypt_mixed_precalc (obj, precalc, input);
	auto gmul_t = decrypt_mixed_gmul (obj, gmul, input);

	std::cout << "Block size    : " << BLOCK_SIZE << std::endl;
	std::cout << "Block count   : " << BLOCK_COUNT << std::endl;
	std::cout << "Vector size   : " << VECTOR_SIZE << std::endl;
	std::cout << "AES-NI        : " << (fullni_t / 1000.0) << "μs" << std::endl;
	std::cout << "Soft Precalc  : " << (precalc_t / 1000.0) << "μs" << std::endl;
	std::cout << "Soft GMul     : " << (gmul_t / 1000.0) << "μs" << std::endl;

	if (memcmp (fullni, precalc, VECTOR_SIZE) or memcmp (fullni, gmul, VECTOR_SIZE))
	{
		std::cerr << "Test result invalid: results different from AES-NI." << std::endl;
		return 2;
	}

	return 0;
}

