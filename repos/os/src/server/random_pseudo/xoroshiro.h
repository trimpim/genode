/**
 * \brief  Linux random emulation code
 * \author Josef Soentgen
 * \date   2016-10-19
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/fixed_stdint.h>
#include <util/string.h>


using Genode::uint64_t;

/*
 * Xoroshiro128+ written in 2014-2016 by Sebastiano Vigna (vigna@acm.org)
 *
 * (see http://xoroshiro.di.unimi.it/xorshift128plus.c and
 *      http://xoroshiro.di.unimi.it/splitmix64.c)
 */

struct Xoroshiro
{
	Xoroshiro(uint64_t seed) : seed(seed)
	{
		s[0] = splitmix64();
		s[1] = splitmix64();
	}

	uint64_t get()
	{
		uint64_t const s0 = s[0];
		uint64_t       s1 = s[1];
		uint64_t const result = s0 + s1;

		s1 ^= s0;

		s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
		s[1] = rotl(s1, 36);

		return result;
	}

	private:

		uint64_t seed;

		uint64_t s[2];

		static uint64_t rotl(uint64_t const x, int k) {
			return (x << k) | (x >> (64 - k));
		}

		uint64_t splitmix64()
		{
			uint64_t z = (seed += __UINT64_C(0x9E3779B97F4A7C15));
			z = (z ^ (z >> 30)) * __UINT64_C(0xBF58476D1CE4E5B9);
			z = (z ^ (z >> 27)) * __UINT64_C(0x94D049BB133111EB);
			return z ^ (z >> 31);
		}
};
