/*
 * \brief  Test program for the xoroshiro_terminal
 * \author Pirmin Duss
 * \date   -----------
 */

/*
 * Copyright (C) 2008-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/component.h>
#include <base/log.h>
#include <terminal_session/connection.h>

#include "helper.h"


Genode::size_t Component::stack_size() { return 64*1024; }

using Genode::uint8_t;
using Genode::int8_t;
using Genode::uint16_t;
using Genode::int16_t;
using Genode::uint32_t;
using Genode::int32_t;
using Genode::uint64_t;
using Genode::int64_t;

void Component::construct(Genode::Env &env)
{
	Terminal::Connection lRandom(env);

	const auto cnt = 3;
	char tmp[16];

	for (auto i=0; i<cnt; ++i) {
		int8_t i8val = Xoroshiro_terminal_test::int8(lRandom);
		Genode::log("getting a random int8_t   : ", static_cast<int>(i8val));
	}

	for (auto i=0; i<cnt; ++i) {
		uint8_t ui8val = Xoroshiro_terminal_test::uint8(lRandom);
		Genode::log("getting a random uint8_t  : ", static_cast<uint16_t>(ui8val));
	}

	for (auto i=0; i<cnt; ++i) {
		int16_t i16val = Xoroshiro_terminal_test::int16(lRandom);
		Genode::log("getting a random int16_t  : ", i16val);
	}

	for (auto i=0; i<cnt; ++i) {
		uint16_t ui16val = Xoroshiro_terminal_test::uint16(lRandom);
		Genode::log("getting a random uint16_t : ", ui16val);
	}

	for (auto i=0; i<cnt; ++i) {
		int32_t i32val = Xoroshiro_terminal_test::int32(lRandom);
		Genode::log("getting a random int32_t  : ", i32val);
	}

	for (auto i=0; i<cnt; ++i) {
		uint32_t ui32val = Xoroshiro_terminal_test::uint32(lRandom);
		Genode::log("getting a random uint32_t : ", ui32val);
	}

	for (auto i=0; i<cnt; ++i) {
		lRandom.read(&tmp, sizeof(int64_t));
		int64_t i64val = Xoroshiro_terminal_test::int64(lRandom);
		Genode::log("getting a random int64_t  : ", i64val);
	}

	for (auto i=0; i<cnt; ++i) {
		lRandom.read(&tmp, sizeof(uint64_t));
		uint64_t ui64val = Xoroshiro_terminal_test::uint64(lRandom);
		Genode::log("getting a random uint64_t : ", ui64val);
	}

	Genode::log("xoroshiro_terminal test completed");
}
