/*
 * \brief  Test spi driver with a shift register
 * \author Jean-Adrien Domage <jean-adrien.domage@gapfruit.com>
 * \date   2021-04-28
 */

/*
 * Copyright (C) 2013-2021 Genode Labs GmbH
 * Copyright (C) 2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <libc/component.h>

/* i2c includes */
#include <i2c/driver_base.h>

/* libc includes */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>


namespace I2c_test {

	using namespace Genode;

	class Main;
}


namespace {

	union Month {
		uint8_t reg;
		struct {
			uint8_t ones : 4;
			uint8_t tens : 1;
			uint8_t gp6  : 1;
			uint8_t gp7  : 1;
			uint8_t gp8  : 1;
		};
	};

	union Year {
		uint8_t reg;
		struct {
			uint8_t ones : 4;
			uint8_t tens : 4;
		};
	};

	static inline Genode::uint8_t hundreds(unsigned value) {
		return static_cast<Genode::uint8_t>((value / 100) % 10);
	}

	static inline Genode::uint8_t tens(unsigned value) {
		return static_cast<Genode::uint8_t>((value / 10) % 10);
	}

	static inline Genode::uint8_t ones(unsigned value) {
		return static_cast<Genode::uint8_t>(value % 10);
	}
}

class I2c_test::Main
{
	private:

		uint8_t const   MONTH_REGISTER_OFFSET  { 0x5 };
		uint8_t const   REGISTER_OFFSET_YEAR   { 0x6 };

		char    const  *_device_name           { "/dev/i2c/rtc" };

		Env        &_env;
		int         _device_fd;

		inline uint8_t _read_register(uint8_t offset) {

			uint8_t out { 0xff };
			Libc::with_libc([offset, &out, this] {

				if (write(_device_fd, &offset, 1) == -1) { error("write error"); }
				if (read(_device_fd, &out, 1) ==  -1)    { error("read error"); }
			});

			return out;
		}

	public:

		Main(Env &env)
		:
			_env { env }
		{
			open_device();
			if (_device_fd < 0) {
				error("failed to open i2c File");
				exit(1);
			}

			Year  year  { _read_register(REGISTER_OFFSET_YEAR) };
			Month month { _read_register(MONTH_REGISTER_OFFSET) };

			log("  year   (reg) = ", Hex { year.reg, Hex::Prefix::PREFIX, Hex::Pad::PAD });
			log("  year         = ", 2000u + 10u * year.tens + year.ones);

			log("  month  (reg) = ", Hex { month.reg, Hex::Prefix::PREFIX, Hex::Pad::PAD });
			log("  month        = ", 10u * month.tens + month.ones);
		}

		void open_device()
		{
			Libc::with_libc([this] {

				int flags = O_WRONLY | O_CREAT;
				mode_t mode = FREAD | FWRITE ;

				_device_fd = open(_device_name, flags, mode);
			});
		}
};

void Libc::Component::construct(Libc::Env &env)
{
	static I2c_test::Main main { env };
}
