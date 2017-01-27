/*
 * \brief  helper functions for Xsoroshiro_terminal tests.
 * \author Pirmin Duss
 * \date   2016-12-19
 */

/*
 * Copyright (C) 2006-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE_RANDOM_H_
#define _INCLUDE_RANDOM_H_

#include <base/component.h>
#include <terminal_session/connection.h>

using Genode::int8_t;
using Genode::int16_t;
using Genode::int32_t;
using Genode::int64_t;
using Genode::uint8_t;
using Genode::uint16_t;
using Genode::uint32_t;
using Genode::uint64_t;

namespace Xoroshiro_terminal_test {

	bool read(Terminal::Connection& conn, uint8_t count, uint8_t* buffer);

	uint8_t uint8(Terminal::Connection& conn);
	int8_t int8(Terminal::Connection& conn);

	uint16_t uint16(Terminal::Connection& conn);
	int16_t int16(Terminal::Connection& conn);

	uint32_t uint32(Terminal::Connection& conn);
	int32_t int32(Terminal::Connection& conn);

	uint64_t uint64(Terminal::Connection& conn);
	int64_t int64(Terminal::Connection& conn);
};


bool Xoroshiro_terminal_test::read(Terminal::Connection& conn, uint8_t count, uint8_t* buffer) {
	return conn.read(buffer, count) == count;
}


uint8_t Xoroshiro_terminal_test::uint8(Terminal::Connection& conn) {
	uint8_t buff[1];
	if (read(conn, 1, buff))
	{
		return buff[0];
	}
	return 0;
}


int8_t Xoroshiro_terminal_test::int8(Terminal::Connection& conn) {
 uint8_t buff[1];
	if (read(conn, 1, buff))
	{
		return static_cast<int8_t>(buff[0]);
	}
	return 0;
}


uint16_t Xoroshiro_terminal_test::uint16(Terminal::Connection& conn) {
	uint8_t buff[2];
	if (read(conn, 2, buff))
	{
		return (static_cast<uint16_t>(buff[0]) +
		       (static_cast<uint16_t>(buff[1]) << 8));
	}
	return 0;
}


int16_t Xoroshiro_terminal_test::int16(Terminal::Connection& conn) {
	uint8_t buff[2];
	if (read(conn, 2, buff))
	{
		return (static_cast<int16_t>(buff[0]) +
		       (static_cast<int16_t>(buff[1]) << 8));
	}
	return 0;
}


uint32_t Xoroshiro_terminal_test::uint32(Terminal::Connection& conn) {
	uint8_t buff[4];
	if (read(conn, 4, buff))
	{
		return (static_cast<uint32_t>(buff[0]) +
		       (static_cast<uint32_t>(buff[1]) << 8) +
		       (static_cast<uint32_t>(buff[2]) << 16) +
		       (static_cast<uint32_t>(buff[3]) << 24));
	}
	return 0;
}


int32_t Xoroshiro_terminal_test::int32(Terminal::Connection& conn) {
	uint8_t buff[4];
	if (read(conn, 4, buff))
	{
		return (static_cast<int32_t>(buff[0]) +
	           (static_cast<int32_t>(buff[1]) << 8) +
	           (static_cast<int32_t>(buff[2]) << 16) +
	           (static_cast<int32_t>(buff[3]) << 24));
	}
	return 0;
}


uint64_t Xoroshiro_terminal_test::uint64(Terminal::Connection& conn) {
	uint8_t buff[8];
	if (read(conn, 8, buff))
	{
		return (static_cast<uint64_t>(buff[0]) +
		       (static_cast<uint64_t>(buff[1]) << 8) +
		       (static_cast<uint64_t>(buff[2]) << 16) +
		       (static_cast<uint64_t>(buff[3]) << 24) +
		       (static_cast<uint64_t>(buff[4]) << 32) +
		       (static_cast<uint64_t>(buff[5]) << 40) +
		       (static_cast<uint64_t>(buff[6]) << 48) +
		       (static_cast<uint64_t>(buff[7]) << 56));
	}
	return 0;
}


int64_t Xoroshiro_terminal_test::int64(Terminal::Connection& conn) {
	uint8_t buff[8];
	if (read(conn, 8, buff))
	{
		return (static_cast<int64_t>(buff[0]) +
		       (static_cast<int64_t>(buff[1]) << 8)+
		       (static_cast<int64_t>(buff[2]) << 16) +
		       (static_cast<int64_t>(buff[3]) << 24) +
		       (static_cast<int64_t>(buff[4]) << 32) +
		       (static_cast<int64_t>(buff[5]) << 40) +
		       (static_cast<int64_t>(buff[6]) << 48) +
		       (static_cast<int64_t>(buff[7]) << 56));
	}
	return 0;
}


#endif   // _INCLUDE_RANDOM_H_
