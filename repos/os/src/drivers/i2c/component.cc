/*
 * \brief  i2c dirver
 * \author Pirmin Duss <pirmin.duss@gapfruit.com>
 * \data   2020-08-24
 */

 /*
  * Copyright (C) 2009-2017 Genode Labs GmbH
  * Copyright (C) 2020 gapfruit AG
  *
  * This file is part of the Genode OS framework, which is distributed
  * under the terms of the GNU Affero General Public License version 3.
  */


/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/heap.h>
#include <util/reconstructible.h>
#include <timer_session/connection.h>

/* local includes */
#include "driver.h"


namespace I2c
{
	using namespace Genode;

	class Main;
}


class I2c::Main
{
	private:

		Env                     &_env;
		Attached_rom_dataspace   _config { _env, "config" };
		Constructible<Driver>    _driver { };

		// for test only
		Timer::Connection        _timer  { _env };
		Signal_handler<Main>     _timer_handler { _env.ep(), *this, &Main::_handle_timer };

		enum { DELAY  = 200000 };  // 0.2 seconds

		void _handle_timer()
		{
			uint8_t data[2] { 0x11, 0x44 };
			uint8_t addr    { 0x50 };

			_driver->send(addr, data, sizeof(data));

			_timer.trigger_once(DELAY);
		}

	public:

		Main(Env& env) : _env { env }
		{
			_config.update();
			_driver.construct(_env, _config.xml());

			_timer.sigh(_timer_handler);
			_timer.trigger_once(DELAY * 20);
		}
};


void Component::construct(Genode::Env &env) { static I2c::Main main(env); }