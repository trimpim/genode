/*
 * \brief  Driver that connects the option UART of USB modems
 *         to a terminal session
 * \author Sebastian Sumpf
 * \author Pirmin Duss
 * \author Alice Domage
 * \date   2024-08-07
 */

/*
 * Copyright (C) 2022-2024 Genode Labs GmbH
 * Copyright (C) 2022-2024 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


/* Genode includes */
#include <base/component.h>

#include <terminal_session/connection.h>
#include <timer_session/connection.h>

/* temporary includes */
#include <util/string.h>

/* gapfruit includes */
#include "hexdump.h"

/* local includes */
#include "command.h"
#include "cdc.h"


namespace Uart {

	using namespace Genode;

	using Command = Usb::Command;

	struct Config
	{
		bool    verbose { true };
	};

	class Main;
};


class Uart::Main
{
	private:

		Env &_env;

		Config                               _config                  { };

		Constructible<Usb::Cdc>              _cdc_driver              { };
		Constructible<Terminal::Connection>  _terminal                { };

		Command                              _terminal_read_buffer    { };
		Command                              _usb_read_buffer         { };

		Timer::Connection                    _timer                   { _env };
		Signal_handler<Main>                 _terminal_handler        { _env.ep(), *this, &Main::_read_terminal };
		Signal_handler<Main>                 _usb_con_handler         { _env.ep(), *this, &Main::_usb_connect };
		Signal_handler<Main>                 _usb_handler             { _env.ep(), *this, &Main::_read_usb };

		void _usb_connect();
		void _flush_terminal();
		void _read_terminal();
		void _read_usb();

	public:

		Main(Genode::Env &env) :
			_env(env)
		{
			log("Strating CDC-ACM driver");

			_cdc_driver.construct(_env);
			_cdc_driver->connected_sigh(_usb_con_handler);
			_cdc_driver->read_avail_sigh(_usb_handler);
		}
};


void Uart::Main::_flush_terminal()
{
	char tmp[2] { };
	while (_terminal->avail()) {
		_terminal->read(tmp, 1);
	}
}


void Uart::Main::_read_terminal()
{
	auto     fail_fn { [] (char *, size_t) { error("terminal buffer overflow"); } };
	char     tmp[2]  { };
	Command  cmd     { };

	while (_terminal->avail()) {

		size_t size = _terminal->read(tmp, 1);

		if (!_terminal_read_buffer.try_append(tmp, size, fail_fn)) {
			return;
		}
		cmd.try_append(tmp, size, fail_fn);
	}

	if (_config.verbose) {
		log("(Terminal -> ) received num_bytes=", _terminal_read_buffer.num_bytes(),
		    " data=",                             cmd.debug_str(),
		    " complete=",                         _terminal_read_buffer.complete());
	}


	if (_terminal_read_buffer.complete()) {
		_cdc_driver->cmd_write(_terminal_read_buffer);
		_terminal_read_buffer.clear();
	}
}


void Uart::Main::_usb_connect()
{
	if (!_terminal.constructed()) {
		_terminal.construct(_env);
		_terminal->read_avail_sigh(_terminal_handler);
	}
	_flush_terminal();
}


void Uart::Main::_read_usb()
{
	int res { _cdc_driver->read_response(_usb_read_buffer) };
	if (res < 0) {
		/* nothing to read yet */
		return;
	}

	if (_config.verbose) {
		log("(Terminal <- ) send num_bytes=",_usb_read_buffer.num_bytes(),
		    " data=",                        _usb_read_buffer.debug_str(),
		    " complete=",                    _usb_read_buffer.complete());
	}

	if (_terminal.constructed() && _usb_read_buffer.complete()) {

		/**
		 * do not write too mouch data ot once.
		 */
		size_t const write_max { 40 };
		size_t       remaining { _usb_read_buffer.num_bytes() };
		size_t       offset    { 0 };
		while (remaining) {
			size_t to_write { min(write_max, remaining) };
			size_t written  { _terminal->write(_usb_read_buffer.start() + offset, to_write) };

			remaining -= written;
			offset    += written;
			_timer.msleep(3);
		}
		_usb_read_buffer.clear();
	}

	if (!_usb_read_buffer.complete()) {
		log(_usb_read_buffer.debug_str());
	}
}

void Component::construct(Genode::Env &env)
{
	static Uart::Main uart(env);
}
