/*
 * \brief  helper functions for Random.
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
#include <base/log.h>
#include <terminal_session/connection.h>


using Genode::int8_t;
using Genode::int16_t;
using Genode::int32_t;
using Genode::int64_t;
using Genode::uint8_t;
using Genode::uint16_t;
using Genode::uint32_t;
using Genode::uint64_t;

namespace Random {

  bool getData(Terminal::Connection& conn, uint8_t count, uint8_t* buffer);

  uint8_t getUint8(Terminal::Connection& conn);
  int8_t getInt8(Terminal::Connection& conn);

  uint16_t getUint16(Terminal::Connection& conn);
  int16_t getInt16(Terminal::Connection& conn);

  uint32_t getUint32(Terminal::Connection& conn);
  int32_t getInt32(Terminal::Connection& conn);
};


bool Random::getData(Terminal::Connection& conn, uint8_t count, uint8_t* buffer) {
  return conn.read(buffer, count) == count;
}


uint8_t Random::getUint8(Terminal::Connection& conn) {
  uint8_t buff[1];
  if (getData(conn, 1, buff))
  {
    return buff[0];
  }
  return 0;
}


int8_t Random::getInt8(Terminal::Connection& conn) {
  uint8_t buff[1];
  if (getData(conn, 1, buff))
  {
    return static_cast<int8_t>(buff[0]);
  }
  return 0;
}


uint16_t Random::getUint16(Terminal::Connection& conn) {
  uint8_t buff[2];
  if (getData(conn, 2, buff))
  {
    return static_cast<uint16_t>(buff[0] + (buff[1] << 8));
  }
  return 0;
}


int16_t Random::getInt16(Terminal::Connection& conn) {
  uint8_t buff[2];
  if (getData(conn, 2, buff))
  {
    return static_cast<int16_t>(buff[0] + (buff[1] << 8));
  }
  return 0;
}


uint32_t Random::getUint32(Terminal::Connection& conn) {
  uint8_t buff[4];
  if (getData(conn, 4, buff))
  {
    return static_cast<uint32_t>(buff[0] + (buff[1] << 8)+
                                 (buff[2] << 16) + (buff[3] << 24));
  }
  return 0;
}


int32_t Random::getInt32(Terminal::Connection& conn) {
  uint8_t buff[4];
  if (getData(conn, 4, buff))
  {
    return static_cast<int32_t>(buff[0] + (buff[1] << 8)+
                                (buff[2] << 16) + (buff[3] << 24));
  }
  return 0;
}


#endif   // _INCLUDE_RANDOM_H_
