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
#include <base/env.h>
#include <base/heap.h>
#include <usb_session/connection.h>
#include <usb_session/usb_session.h>
#include <usb_session/device.h>

/* local includes */
#include "command.h"


namespace Usb
{
	using namespace Genode;

	using Alt_setting              = Usb::Interface::Alt_setting;
	using Device                   = Usb::Device;
	using Device_packet_descriptor = Usb::Device::Packet_descriptor;
	using Device_urb               = Usb::Device::Urb;
	using Endpoint                 = Usb::Endpoint;
	using Iface_packet_descriptor  = Usb::Interface::Packet_descriptor;
	using Iface_urb                = Usb::Interface::Urb;
	using Index                    = Usb::Interface::Index;
	using Interface                = Usb::Interface;
	using Return_value             = Usb::Interface::Packet_descriptor::Return_value;

	using Command                  = Usb::Command_t<char>;

	struct Config
	{
		bool    verbose { true };
	};

	static uint8_t  const usb_recip_interface                { 0x01 };
	static uint8_t  const usb_type_class                     { 0x01 << 5 };
	static uint8_t  const usb_rt_acm                         { usb_type_class | usb_recip_interface };

	static uint16_t const usb_cdc_ctrl_dtr                   { 1 << 0 };
	static uint16_t const usb_cdc_ctrl_rts                   { 1 << 1 };
	static uint16_t const dtr_rts_set                        { usb_cdc_ctrl_dtr | usb_cdc_ctrl_rts };
	static uint8_t  const usb_cdc_line_coding                { 0x20 };
	static uint8_t  const usb_cdc_req_set_control_line_state { 0x22 };

	char const *usb_return_to_string(Return_value value);

	struct Ctl_urb;

	struct Cdc_acm_base;
	struct Cdc_acm_ctl;
	struct Cdc_acm_data;

	class Cdc;
}


struct Usb::Ctl_urb : Device_urb
{
	/**
	 * Unconditionally set control transfer timeout to 1 sec,
	 * otherwise it can block a device forever, as we do not
	 * cancel control transfers yet in this backend.
	 */
	enum { CONTROL_XFER_TIMEOUT = 1000 };

	using Request_type = Usb::Device::Packet_descriptor::Request_type::access_t;

	char const  *_packet;
	size_t       _size;

	/* non copyable */
	Ctl_urb(Ctl_urb const &)                 = delete;
	Ctl_urb(Ctl_urb const &&)                = delete;
	Ctl_urb & operator  = (Ctl_urb const &)  = delete;
	Ctl_urb && operator = (Ctl_urb const &&) = delete;

	Ctl_urb(Device    &device,
	        uint8_t    request,
	        uint8_t    request_type,
	        uint16_t   value,
	        uint16_t   index,
	        size_t     size,
	        char      *packet)
	:
		Device_urb { device, request,
		             (Request_type)request_type,
		             value, index, size,
		             CONTROL_XFER_TIMEOUT },
		_packet    { packet },
		_size      { size }
	{ }
};


struct Usb::Cdc_acm_base
{
	static constexpr size_t const PACKET_STREAM_BUF_SIZE { 2 * (1UL << 20) };   /* size of the USB communication buffer*/
	static constexpr size_t const RING_BUFFER_SIZE       { 4096 };              /* size of the ring buffer */

	Env        &_env;
	Interface  &_iface;
	bool        _verbose;

	Command     _response_buffer        { };
	Mutex       _response_buffer_lock   { };

	Command     _command_buffer         { };

	Cdc_acm_base(Env &env, Interface &iface, bool verbose) :
		_env     { env },
		_iface   { iface },
		_verbose { verbose }
	{ }

	virtual ~Cdc_acm_base() { }

	Command &response_buffer()       { return _response_buffer; }
	Mutex   &response_buffer_lock()  { return _response_buffer_lock; }

	virtual void update() = 0;
};


struct Usb::Cdc_acm_ctl : public Cdc_acm_base
{
	static constexpr size_t const MAX_PACKET_SIZE  { 8 };

	Interface                &_iface;
	Endpoint                  _ep_ctl   { _iface, Endpoint::Direction::IN , Endpoint::Type::IRQ };
	Constructible<Iface_urb>  _irq_urb  { };

	Cdc_acm_ctl(Env &env, Interface &iface, Config &config) :
		Cdc_acm_base { env, iface, config.verbose },
		_iface       { iface }
	{
		_irq_urb.construct(_iface, _ep_ctl,
		                  Usb::Interface::Packet_descriptor::IRQ,
		                  MAX_PACKET_SIZE);
	}

	virtual ~Cdc_acm_ctl()
	{
		_iface.dissolve_all_urbs<Iface_urb>([this] (Iface_urb &urb) {
			if (_irq_urb.constructed() && &urb == &*_irq_urb) {
				_irq_urb.destruct(); }
		});
	}

	void update() override;
};


struct Usb::Cdc_acm_data : public Cdc_acm_base
{
	static constexpr size_t const MAX_PACKET_SIZE  { 40 };

	Env                       &_env;
	Interface                 &_iface;

	Endpoint                   _ep_in              { _iface, Endpoint::Direction::IN , Endpoint::Type::BULK };
	Endpoint                   _ep_out             { _iface, Endpoint::Direction::OUT, Endpoint::Type::BULK };

	Constructible<Iface_urb>   _in_urb             { };
	Constructible<Iface_urb>   _out_urb            { };

	uint8_t                    _last_request       { 0xff };
	bool                       _response_received  { false };

	Signal_context_capability  _read_avail_sigh;

	Cdc_acm_data(Env &env, Interface &iface, Config &config, Signal_context_capability read_vail) :
		Cdc_acm_base     { env, iface, config.verbose },
		_env             { env },
		_iface           { iface },
		_read_avail_sigh { read_vail }
	{
		/* prepare to receive data */
		_in_urb.construct(_iface, _ep_in,
		                  Usb::Interface::Packet_descriptor::BULK,
		                  MAX_PACKET_SIZE);
	}

	virtual ~Cdc_acm_data()
	{
		_iface.dissolve_all_urbs<Iface_urb>([this] (Iface_urb &urb) {
			if (_in_urb.constructed() && &urb == &*_in_urb) {
				_in_urb.destruct(); }
			if (_out_urb.constructed() && &urb == &*_out_urb) {
				_out_urb.destruct(); }
		});
	}

	size_t write(Command &cmd);
	void update() override;
};


class Usb::Cdc
{
	private:

		enum State {
			DISCONECTED,
			ALT_SETTING,
			SET_RTS_DTR,
			SET_ENCODING,
			READY,
		};

		static constexpr uint8_t const  _alt_setting            { 0x0 };
		static constexpr Index const    _ctl_interface_index    { 0x0, _alt_setting };
		static constexpr Index const    _data_interface_index   { 0x1, _alt_setting };

		Env                            &_env;
		Heap                            _heap                   { _env.ram(), _env.rm() };
		Config                          _config                 { };
		State                           _state                  { DISCONECTED };

		Usb::Connection                 _connection             { _env };
		Constructible<Device>           _device                 { };
		Constructible<Interface>        _control_interface      { };
		Constructible<Interface>        _data_interface         { };
		Constructible<Alt_setting>      _alt_settings_ctl       { };
		Constructible<Alt_setting>      _alt_settings_data      { };
		Constructible<Cdc_acm_ctl>      _ctl_port               { };
		Constructible<Cdc_acm_data>     _data_port              { };

		Signal_context_capability       _connected_sigh         { };
		Signal_context_capability       _read_avail_sigh        { };

		Signal_handler<Cdc>             _io_handler             { _env.ep(), *this, &Cdc::_handle_io };
		Signal_handler<Cdc>             _device_state_handler   { _env.ep(), *this, &Cdc::_handle_device_state };

		Constructible<Ctl_urb>          _ctl_urb                { };

		void _handle_io();
		String<16> _state_to_string() const;
		void _set_state(State new_state);
		void _handle_device_state();
		void _set_dtr_rts();
		void _set_transfer_encoding();

	public:

		Cdc(Env & env) :
			_env   { env }
		{
			_connection.sigh(_device_state_handler);
		}

		bool ready() const { return _state == READY; }

		size_t cmd_write(Command &cmd);
		int read_response(Command &cmd);

		void connected_sigh(Signal_context_capability sigh)  { _connected_sigh = sigh; }
		void read_avail_sigh(Signal_context_capability sigh)
		{
			_read_avail_sigh = sigh;

			if (_data_port.constructed()) {
				_data_port->_read_avail_sigh = sigh;
			}
		}
};

