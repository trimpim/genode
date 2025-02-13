#pragma once


#include <base/output.h>
#include <base/stdint.h>
#include <util/string.h>


namespace Genode {

	class Hexdump;
	class Hexdump_volatile;
}


class Genode::Hexdump
{
	public:

		using Seperator = Genode::String<4>;

	private:

		const uint8_t* const _buffer;
		size_t         const _size;
		Seperator      const _seperator;
		bool           const _line_breaks;

	public:

		Hexdump(void const *buffer, size_t length)
		:
			_buffer(static_cast<const uint8_t*>(buffer)),
			_size(length),
			_seperator(" "),
			_line_breaks(false)
		{ }

		Hexdump(Byte_range_ptr const &in)
		:
			Hexdump { reinterpret_cast<const uint8_t*>(in.start), in.num_bytes }
		{ }

		Hexdump(Const_byte_range_ptr const &in)
		:
			Hexdump { reinterpret_cast<const uint8_t*>(in.start), in.num_bytes }
		{ }

		Hexdump(void const *buffer, size_t length, Seperator seperator, bool line_breaks)
		:
			_buffer(static_cast<const uint8_t*>(buffer)),
			_size(length),
			_seperator(seperator),
			_line_breaks(line_breaks)
		{ }

		Hexdump(Byte_range_ptr const &in, Seperator seperator, bool line_breaks)
		:
			Hexdump { reinterpret_cast<const uint8_t*>(in.start),
			          in.num_bytes, seperator, line_breaks }
		{ }

		Hexdump(Const_byte_range_ptr const &in, Seperator seperator, bool line_breaks)
		:
			Hexdump { reinterpret_cast<const uint8_t*>(in.start),
			          in.num_bytes, seperator, line_breaks }
		{ }

		void print(Genode::Output &out) const
		{
			using Genode::print;

			for (size_t offset = 0; offset < _size; ++offset) {
				print(out, Hex(_buffer[offset], Hex::OMIT_PREFIX, Hex::PAD), _seperator);
				if (((offset+1) % 16 == 0) && (offset+1 < _size)) {
					if (_line_breaks) print(out, "\n");
				}
			}
		}
};


class Genode::Hexdump_volatile
{
	public:

		using Seperator = Genode::String<4>;

	private:

		uint8_t const volatile * const _buffer;
		size_t                   const _size;
		Seperator                const _seperator;
		bool                     const _line_breaks;

	public:

		Hexdump_volatile(void const volatile *buffer, size_t length)
		:
			_buffer(static_cast<uint8_t const volatile *>(buffer)),
			_size(length),
			_seperator(" "),
			_line_breaks(false)
		{ }

		Hexdump_volatile(void const volatile *buffer, size_t length, Seperator seperator, bool line_breaks)
		:
			_buffer(static_cast<uint8_t const volatile*>(buffer)),
			_size(length),
			_seperator(seperator),
			_line_breaks(line_breaks)
		{ }

		void print(Genode::Output &out) const
		{
			using Genode::print;

			for (size_t offset = 0; offset < _size; ++offset) {
				print(out, Hex(_buffer[offset], Hex::OMIT_PREFIX, Hex::PAD), _seperator);
				if (((offset+1) % 16 == 0) && (offset+1 < _size)) {
					if (_line_breaks) print(out, "\n");
				}
			}
		}
};
