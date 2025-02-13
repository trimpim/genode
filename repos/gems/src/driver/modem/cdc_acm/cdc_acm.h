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
#include <base/mutex.h>
#include <base/env.h>
#include <timer_session/connection.h>
#include <util/string.h>

/* local includes */
#include "command.h"


namespace Usb
{
	using namespace Genode;

	using Command = Usb::Command_t<uint8_t>;

	class Cdc_acm;
};


class Usb::Cdc_acm
{
	private:

		struct Cdc_acm_setup_failed : Exception { };

		/* TODO: move to config */
		static uint8_t  const _ep_in_addr    { 0x81 };
		static uint8_t  const _ep_out_addr   { 0x01 };
		static uint8_t  const _ep_irq_addr   { 0x82 };
		static uint16_t const _vendor_id     { 0x483 };
		static uint16_t const _product_id    { 0x5740 };

		static uint8_t  const _acm_ctrl_dtr  { 0x01 };
		static uint8_t  const _acm_ctrl_rts  { 0x02 };

		Env                          &_env;
		bool                          _verbose;
		struct libusb_device_handle  &_device_handle;

		Timer::Connection             _timer                { _env };
		Signal_context_capability     _response_ready       { };

		Command                       _response_buffer      { };
		Mutex                         _response_buffer_lock { };

		struct libusb_device_handle &_init();
		void _claim_interfaces();
		void _config_device();
		void _read();
		int _write(Command &cmd, size_t offset, size_t to_write);

		/* non copyable */
		Cdc_acm(Cdc_acm const &)                 = delete;
		Cdc_acm(Cdc_acm const &&)                = delete;
		Cdc_acm & operator  = (Cdc_acm const &)  = delete;
		Cdc_acm && operator = (Cdc_acm const &&) = delete;

	public:

		Cdc_acm(Env &env, bool verbose);

		void read_ready_sigh(Signal_context_capability sigh) { _response_ready = sigh; }

		int write_command(Command &cmd);
		int read_response(Command &cmd);
};
