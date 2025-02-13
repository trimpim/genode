/*
 * \brief  Driver for USB Communication Device Class (CDC)
 *         subclass Abstract Control Model (ACM)
 * \author Pirmin Duss
 * \date   2025-01-17
 */

/*
 * Copyright (C) 2022-2024 Genode Labs GmbH
 * Copyright (C) 2022-2024 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


#pragma once

/* Genode includes */
#include <base/log.h>
#include <util/string.h>

/* local includes */
#include "hexdump.h"


namespace Usb
{
	enum {
		DEBUG_STRING_SIZE = 128,
		BUFFER_SIZE       = 8192,
	};

	using Debug_string = Genode::String<DEBUG_STRING_SIZE>;

	using namespace Genode;

	template <typename T>
	class Command_t;
}


template <typename T>
class Usb::Command_t
{
	private:

		T        _buffer[BUFFER_SIZE] { 0 };
		size_t   _num_bytes           { 0 };

		class Local_output : public Output
		{
			private:

				/*
				 * Noncopyable
				 */
				Local_output(Local_output const &);
				Local_output &operator = (Local_output const &);

			public:

				char * const _buf;

				size_t _num_chars = 0;

				/**
				 * Return true if '_buf' can fit at least one additional 'char'.
				 */
				bool _capacity_left() const { return BUFFER_SIZE - _num_chars - 1; }

				void _append(char c) { _buf[_num_chars++] = c; }

				Local_output(char *buf) : _buf(buf) { }

				size_t num_chars() const { return _num_chars; }

				void out_char(char c) override { if (_capacity_left()) _append(c); }

				void out_string(char const *str, size_t n) override
				{
					while (n-- > 0 && _capacity_left() && *str)
						_append(*str++);
				}
		};

		/* Noncopyable */
		Command_t(Command_t const &)                 = delete;
		Command_t(Command_t const &&)                = delete;
		Command_t & operator  = (Command_t const &)  = delete;
		Command_t && operator = (Command_t const &&) = delete;

	public:

		Command_t() { }

		size_t      num_bytes() const { return _num_bytes; }
		T          *start()           { return _buffer; }

		template <typename FN>
		bool try_append(Command_t &in, FN fail_fn)
		{
			if ((_num_bytes + in.num_bytes()) > BUFFER_SIZE) {
error("overflow!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				fail_fn(in);
				return false;
			}

			memcpy(&_buffer[_num_bytes], in.start(), in.num_bytes());
			_num_bytes += in.num_bytes();

			return true;
		}

		template <typename FN>
		bool try_append(T *ptr, size_t count, FN fail_fn)
		{
			if ((_num_bytes + count) > BUFFER_SIZE) {
				fail_fn(ptr, count);
				return false;
			}

			memcpy(&_buffer[_num_bytes], ptr, count);
			_num_bytes += count;

			return true;
		}

		template <typename FN>
		bool try_append(T const *ptr, size_t count, FN fail_fn)
		{
			if ((_num_bytes + count) > BUFFER_SIZE) {
				fail_fn(ptr, count);
				return false;
			}

			memcpy(&_buffer[_num_bytes], ptr, count);
			_num_bytes += count;

			return true;
		}

		Debug_string debug_str() const
		{
			return Debug_string { Hexdump { _buffer, min(_num_bytes, 32U), " ", false },
			                      _num_bytes > 32 ? "..." : "" };
		}

		void clear()
		{
			_num_bytes = 0;
			memset(_buffer, 0, sizeof(_buffer));
		}

		bool complete() const
		{
			/**
			 * packets used in the communication with the LORA concentrators
			 * attached via USB have the following  format:
			 *
			 *    | rng | payload_size_high | payload_size_low | request_type | payload-if-any |
			 *
			 *    rng               : random value 0..255
			 *    payload_size_high : high byte of payload size (payload.size >> 8)
			 *    payload_size_low  : low byte of payload size (payload.size & 0xff)
			 *    request_type      : type of request (currently 0x40 .. 0x46 are used)
			 *    payload-if-any    : payload.size bytes of additional data
			 *
			 *    A package is complete if it is 4 (the header) + payload.size bytes long.
			 *    We do not verivy the request type here to prevent protocol changes breaking
			 *    the driver.
			 */
			struct Sz { uint8_t h; uint8_t l; };
			struct Header
			{
				T rng;
				T payload_size_high;
				T payload_size_low;
				T request_type;
			} __attribute__((packed));

			if (_num_bytes < 4) return false;

			Header const *header       = reinterpret_cast<Header const *>(&(_buffer[0]));
			uint16_t      payload_size = (uint8_t)header->payload_size_low |
			                             ((uint8_t)header->payload_size_high << 8U);

			if ((payload_size > 0) && (_num_bytes < sizeof(Header) + payload_size)) return false;

			return true;
		}

		template <typename FN>
		bool for_each(FN fn)
		{
			for (size_t idx = 0 ; idx < _num_bytes ; ++idx) {
				bool res { fn(_buffer[idx]) };
				if (!res) return res;
			}
			return true;
		}
};
