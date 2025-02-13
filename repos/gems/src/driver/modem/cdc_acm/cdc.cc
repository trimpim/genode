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



#include "hexdump.h"

/* local includes */
#include "cdc.h"


char const *Usb::usb_return_to_string(Return_value value) {
	switch (value)
	{
		case Return_value::UNHANDLED:
			return "UNHANDLED";
		case Return_value::NO_DEVICE:
			return "NO_DEVICE";
		case Return_value::INVALID:
			return "INVALID";
		case Return_value::TIMEOUT:
			return "TIMEOUT";
		case Return_value::HALT:
			return "HALT";
		case Return_value::OK:
			return "OK";
	}
	return "invalid";
}


void Usb::Cdc::_handle_io()
{
	/* the device got disconnected */
	if (!_device.constructed()) { error("not constructed"); return; }

	State out_state { _state };
	/**
	 * produce out data
	 */
	auto out_fn = [this] (Device_urb &urb, Byte_range_ptr &dst) {

		if (_ctl_urb.constructed() && (&urb == &*_ctl_urb)) {
			warning("(cdc-acm data) fill device OUT-URB","  state = ",_state_to_string());
			Ctl_urb *ctl { reinterpret_cast<Ctl_urb *>(&urb) };
			memcpy(dst.start, ctl->_packet, ctl->_size);
		}
	};

	/**
	 * consume in data
	 */
	auto in_fn  = [] (Device_urb &, Const_byte_range_ptr &src) {
		if (src.num_bytes) {
			/* should never happen */
			warning("(cdc-acm data) empty device IN-URB");
			log("(cdc-acm data)  >>>> ", Genode::Hexdump { src });
		}
	};

	/**
	 * handle clmpleted transfers
	 */
	auto complete_fn = [&out_state, this] (Device_urb &, Device_packet_descriptor::Return_value value) {

		warning("(cdc-acm data) URB complete return_value = ", usb_return_to_string(value),"  state = ",_state_to_string());
		switch(value) {
			case Device_packet_descriptor::OK:

				switch (_state) {
					case SET_RTS_DTR:
						out_state = SET_ENCODING;
						break;
					case SET_ENCODING:
						out_state =  ALT_SETTING;
						break;
					default:
						out_state = READY;
						break;
				}

				break;
			case Device_packet_descriptor::NO_DEVICE: [[fallthrough]];
			default:
				out_state = DISCONECTED;
		};
	};

	switch(_state) {
		case ALT_SETTING:
		case SET_RTS_DTR:
		case SET_ENCODING:
			_device->update_urbs<Device_urb>(out_fn, in_fn, complete_fn);
			break;
		case READY:
			_device->update_urbs<Device_urb>(out_fn, in_fn, complete_fn);

			if (_data_port.constructed()) {
				_data_port->update(); }
			if (_ctl_port.constructed()) {
				_ctl_port->update(); }

			break;
		case DISCONECTED:
			if (_data_port.constructed()) {
				_data_port.destruct(); }
			if (_ctl_port.constructed()) {
				_ctl_port.destruct(); }
			break;
	};

	if (out_state != _state) {
		_set_state(out_state);
	}
}


Genode::String<16> Usb::Cdc::_state_to_string() const
{
	switch(_state) {
		case ALT_SETTING:  return "ALT_SETTING";
		case SET_RTS_DTR:  return "SET_RTS_DTR";
		case SET_ENCODING: return "SET_ENCODING";
		case DISCONECTED:  return "DISCONECTED";
		case READY:        return "READY";
	};
	return "";
}


void Usb::Cdc::_set_state(State new_state)
{
	if (new_state == _state) return;

	_state = new_state;

	if (_config.verbose)
		log("(cdc-acm     ) state=", _state_to_string());

	switch(_state) {
		case ALT_SETTING:

			_data_interface.construct(*_device,
			                          _data_interface_index,
			                          Cdc_acm_data::PACKET_STREAM_BUF_SIZE);
			_data_interface->sigh(_io_handler);
			_alt_settings_data.construct(*_device, *_data_interface);
			_data_port.construct(_env, *_data_interface, _config, _read_avail_sigh);

			_control_interface.construct(*_device,
			                             _ctl_interface_index,
			                             Cdc_acm_data::PACKET_STREAM_BUF_SIZE);
			_control_interface->sigh(_io_handler);
			_alt_settings_ctl.construct(*_device, *_control_interface);
			_ctl_port.construct(_env, *_control_interface, _config);

			_handle_io();
			break;
		case SET_RTS_DTR:

			_device.construct(_connection, _heap, _env.rm());
			_device->sigh(_io_handler);

			_set_dtr_rts();
			_handle_io();
			break;
		case SET_ENCODING:
			_set_transfer_encoding();
			_handle_io();
			break;
		case READY:

			if (_data_port.constructed())
				_data_port->update();
			if (_ctl_port.constructed())
				_ctl_port->update();

			if (_connected_sigh.valid())
				Signal_transmitter(_connected_sigh).submit();
			break;
		case DISCONECTED:
				_data_port.destruct();
				_data_interface.destruct();
				_alt_settings_data.destruct();

				_ctl_port.destruct();
				_control_interface.destruct();
				_alt_settings_ctl.destruct();

				_device->dissolve_all_urbs<Device_urb>([this](auto &){
					_alt_settings_data.destruct(); });
				_device->dissolve_all_urbs<Device_urb>([this](auto &){
					_alt_settings_ctl.destruct(); });

				_device.destruct();
			break;
	};
}


void Usb::Cdc::_handle_device_state()
{
	_connection.with_xml([this] (auto &xml) {
		xml.with_sub_node("device", [this] (auto const /*device*/) {

			if (_config.verbose) log("(cdc-acm     ) Device present");

			if (_state == DISCONECTED)  _set_state(SET_RTS_DTR);
		},
		[this] () {
			if (_config.verbose) {
				log("(cdc-acm     ) No device"); }
			_set_state(DISCONECTED);
			// TODO: clena up data buffers (USB / Terminal (send signal to Main))
		});
	});
}


Genode::size_t Usb::Cdc::cmd_write(Command &cmd)
{
	if (_data_port.constructed()) {
		return _data_port->write(cmd);
	} else {
		warning("(cdc-acm     ) No device, discarding=", cmd.debug_str());
	}
	return cmd.num_bytes();
}


void Usb::Cdc::_set_dtr_rts()
{
	_ctl_urb.construct(*_device,
	                   usb_cdc_req_set_control_line_state,  /* request */
	                   usb_rt_acm,                          /* request type */
	                   static_cast<uint16_t>(dtr_rts_set),  /* value */
	                   static_cast<uint16_t>(0),            /* index */
	                   static_cast<size_t>(0),              /* data size */
	                   nullptr);                            /* pointer to data */

	_handle_io();
}


void Usb::Cdc::_set_transfer_encoding()
{
	/**
	 * set line encoding: here 115200 8N1
	 * 115200 = 0x1c200 ~> 0x00, 0xc2, 0x01, 0x00 in little endian
	 */
	uint8_t encoding[7] = { 0x00, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x08 };

	_ctl_urb.construct(*_device,
	                   usb_cdc_line_coding,                    /* request */
	                   usb_rt_acm,                             /* request type */
	                   static_cast<uint16_t>(0),               /* value */
	                   static_cast<uint16_t>(0),               /* index */
	                   static_cast<size_t>(sizeof(encoding)),  /* size of the data */
	                   (char *)&encoding[0]);                          /* pointer to the data */

	_handle_io();
}


int Usb::Cdc::read_response(Command &cmd)
{
	size_t actual_length { 0 };
	{
		if (_data_port.constructed()) {
			Mutex::Guard guard { _data_port->response_buffer_lock() };
			actual_length = _data_port->response_buffer().num_bytes();

			cmd.try_append(_data_port->response_buffer(), [] (Command &) { /* this can't be reached */ });
			_data_port->response_buffer().clear();
		}
	}

	return actual_length;
}


void Usb::Cdc_acm_ctl::update()
{
warning("Usb::Cdc_acm_ctl::",__func__,"()  ::  ",__LINE__);
	/* received data */
	auto in_fn {
		[this] (Iface_urb &urb, Const_byte_range_ptr &src) {
			warning("Usb::Cdc_acm_ctl::",__func__,"()  ::  ",__LINE__,"  in_fn");
			(void)this; (void)urb; (void)src;
		}
	};

	/* send out data */
	auto out_fn {
		[this] (Iface_urb &urb, Byte_range_ptr &dst) {
			warning("Usb::Cdc_acm_ctl::",__func__,"()  ::  ",__LINE__,"  out_fn");
			(void)this; (void)urb; (void)dst;
		}
	};

	/* complete requests */
	auto complete_fn {
		[this] (Iface_urb &urb, Usb::Interface::Packet_descriptor::Return_value value) {
			warning("Usb::Cdc_acm_ctl::",__func__,"()  ::  ",__LINE__,"  complete_fn");

			if (_irq_urb.constructed() && &urb == &*_irq_urb) {
				if (_verbose)
					log("(cdc-acm ctl ) IRQ URB Completed with return_value=", usb_return_to_string(value));
				_irq_urb.construct(_iface, _ep_ctl,
				                  Usb::Interface::Packet_descriptor::IRQ,
				                  Cdc_acm_ctl::MAX_PACKET_SIZE);
			} else {
				error("Usb::Cdc_acm_ctl::",__func__,"()  ::  ",__LINE__,"  complete_fn");
			}
		}
	};

	_iface.update_urbs<Iface_urb>(out_fn, in_fn, complete_fn);
}


Genode::size_t Usb::Cdc_acm_data::write(Command &cmd)
{
	if (_out_urb.constructed()) {
		warning("(cdc-acm data) OUT-URB already in-flight discarding ", cmd.num_bytes(), " bytes on out end-point.");
		return 0;
	}

	/* memorize command to filter unwanted replies from the modem */
	_last_request      = cmd.start()[0];
	_response_received = false;

	/* clear in buffer */
	{
		Mutex::Guard guard { _response_buffer_lock };
		_response_buffer.clear();
	}


	_out_urb.construct(_iface, _ep_out,
	                   Usb::Interface::Packet_descriptor::BULK,
	                   MAX_PACKET_SIZE);

	_command_buffer.try_append(cmd, [] (Command &) { /* this can't be reached */ });

	update();
	return cmd.num_bytes();
}


void Usb::Cdc_acm_data::update()
{
	using Return_value = Usb::Interface::Packet_descriptor::Return_value;

	auto in_fn {
		[this] (Iface_urb &, Const_byte_range_ptr &src) {

			if (src.num_bytes < 4) { error("222222"); throw 2; }

log("   in: ", Hexdump { src });
			char const response { src.start[0] };
			if (response == _last_request) {

				size_t x = ((uint8_t)src.start[1]  << 8) + (uint8_t)src.start[2];

				{
					Mutex::Guard guard { _response_buffer_lock };
					_response_buffer.try_append(src.start, x+4, [] (char const *, size_t) { error("IN-URB data overflow"); });
				}

				if (_verbose) {
					log("(cdc-acm data) received num_bytes=", src.num_bytes,
					    " data=",                             _response_buffer.debug_str(),
					    " complete=",                         _response_buffer.complete());
				}
				_response_received = true;

				if (_read_avail_sigh.valid() && _response_received) {
					Signal_transmitter { _read_avail_sigh }.submit();
					_response_received = false;
				} else {
					warning("packet not yet complete");
				}
			} else {
				warning("response (", Hex { response }, ") doesn't match request (", Hex { _last_request }, ")");
			}
		}
	};

	/* send out data (none on this interface */
	auto out_fn {
		[this] (Iface_urb&, Byte_range_ptr &dst) {
			memcpy(dst.start, _command_buffer.start(), _command_buffer.num_bytes());
			if (_verbose) {
				log("(cdc-acm data) sending num_bytes=", _command_buffer.num_bytes(),
				    " data=",                            _command_buffer.debug_str() );
			}
			_command_buffer.clear();
		}
	};

	/* complete requests */
	auto complete_fn {
		[this] (Iface_urb &urb, Return_value value) {

			if (_in_urb.constructed() && (&urb == &*_in_urb)) {

				if (_verbose)
  					log("(cdc-acm data) IN-URB Completed with return_value=", usb_return_to_string(value));

				if (_read_avail_sigh.valid() && _response_received) {
					Signal_transmitter { _read_avail_sigh }.submit();
					_response_received = false;
				} else {
					warning("packet not yet complete");
				}

				_in_urb.construct(_iface, _ep_in,
				                  Usb::Interface::Packet_descriptor::BULK,
				                  Cdc_acm_data::MAX_PACKET_SIZE);

			} else if (_out_urb.constructed() && (&urb == &*_out_urb)) {

				if (_verbose)
					log("(cdc-acm data) OUT-URB Completed with return_value=", usb_return_to_string(value));

				_out_urb.destruct();
			} else {
				error("(cdc-acm data) UNKNOWN URB has finished return_value=", usb_return_to_string(value));
			}
		}
	};

	_iface.update_urbs<Iface_urb>(out_fn, in_fn, complete_fn);
}
