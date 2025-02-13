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

/* Genode includes */
#include <base/log.h>
#include <libc/component.h>

/* libusb includes */
#include <libusb.h>

/* local includes */
#include "cdc_acm.h"
#include "hexdump.h"


struct libusb_device_handle &Usb::Cdc_acm::_init()
{
	/**
	 * This produces a compiler warning, because it returns a pointer.
	 * Unfortunately this can't be resolved, as the type
	 * `libusb_device_handle` is incomplete. It uses *pthread_mutex_t, which
	 * is only forward declared in the public interface of libc.
	 */
	struct libusb_device_handle *handle = Libc::with_libc([this] () {

		/**
		 * Initialize libusb
		 */
		int rc = libusb_init(nullptr);
		if (rc < 0) {
			error("Error initializing libusb: ", libusb_error_name(rc));
			throw Cdc_acm_setup_failed { };
		}

		if (_verbose) {
			libusb_set_debug(nullptr, 2); /* also print warnings */
		} else {
			libusb_set_debug(nullptr, 1); /* print only errors */
		}

		struct libusb_device_handle *handle { libusb_open_device_with_vid_pid(NULL, _vendor_id, _product_id) };
		if (!handle) {
			error("Error finding USB device");
			throw Cdc_acm_setup_failed { };
		}
		return handle;
	});

	return *handle;
}


void Usb::Cdc_acm::_claim_interfaces()
{
	/**
	 * claim the bulk transfer interfaces (IN / OUT)
	 */
	const uint8_t num_bulk_interfaces { 2 };
	for (uint8_t idx = 0; idx < num_bulk_interfaces ; ++idx) {

		int rc = libusb_claim_interface(&_device_handle, idx);
		if (rc < 0) {
			error("Error claiming interface ", idx, ": ", libusb_error_name(rc));
			throw Cdc_acm_setup_failed { };
		}
	}

}


void Usb::Cdc_acm::_config_device()
{
	Libc::with_libc([this] () {
		/* Start configuring the device:
		 * - set line state
		 */
		int rc { libusb_control_transfer(&_device_handle, 0x21, 0x22, _acm_ctrl_dtr | _acm_ctrl_rts, 0, nullptr, 0, 0) };
		if (rc < 0) {
			error("Error during control transfer: ", libusb_error_name(rc));
			throw Cdc_acm_setup_failed { };
		}

		/* - set line encoding: here 115200 8N1
		 * 115200 = 0x1c200 ~> 0x00, 0xc2, 0x01, 0x00 in little endian
		 */
		unsigned char encoding[] = { 0x00, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x08 };
		rc = libusb_control_transfer(&_device_handle, 0x21, 0x20, 0, 0, encoding, sizeof(encoding), 0);
		if (rc < 0) {
			error("Error during control transfer: ", libusb_error_name(rc));
			throw Cdc_acm_setup_failed { };
		}
	});
}


void Usb::Cdc_acm::_read()
{
	int ret = Libc::with_libc([this] () {
		/**
		 * To receive characters from the device initiate a bulk_transfer to the
		 * Endpoint with address ep_in_addr.
		 */
		int     actual_length { 0 };
		uint8_t buf[0x80]     { 0 };

		 while(actual_length == 0) {

			int rc = libusb_bulk_transfer(&_device_handle, _ep_in_addr, buf, 40, &actual_length, 1000);
			if (rc == LIBUSB_ERROR_TIMEOUT) {
				error("timeout (",actual_length,") ", libusb_error_name(rc));
				return -1;
			} else if (rc < 0) {
				error("Error while reading response ", libusb_error_name(rc));
				return -1;
			}

			if (actual_length > 0) {
				Mutex::Guard guard { _response_buffer_lock };
				bool ret { _response_buffer.try_append(buf, actual_length,
				                          [] (uint8_t *, size_t) { error("receive buffer overflow"); }) };
				if (!ret) {
					error("kkkkkk");
					return -1;
				}
			}

			_timer.msleep(5);
		}
//error(__func__,"()  ::  ",__LINE__);
		return actual_length;
	});

	if (ret < 0) {
		error("kkkkkk");
		// TODO: how to handle this?
	}

	if (_response_buffer.complete()) {
		Signal_transmitter(_response_ready).submit();
	}
}


int Usb::Cdc_acm::_write(Command &cmd, size_t offset, size_t to_write)
{
	return Libc::with_libc([&cmd, offset, to_write, this] () {
		int actual_length { 0 };
		int rc = libusb_bulk_transfer(&_device_handle,
		                              _ep_out_addr, cmd.start() + offset,
		                              to_write, &actual_length, 0);
		if (rc < 0) {
			error("Error while sending command ", libusb_error_name(rc));
			return -1;
		}

warning("  >>> written  >  ",to_write);
		return rc;
	});
}


Usb::Cdc_acm::Cdc_acm(Env &env, bool verbose) :
	_env            { env },
	_verbose        { verbose },
	_device_handle  { _init() }
{
	_config_device();
	_claim_interfaces();
}


int Usb::Cdc_acm::write_command(Command &cmd)
{
	/**
	 * To send a char to the device simply initiate a bulk_transfer to the
	 * Endpoint with address ep_out_addr.
	 */

	/**
	 * do not write too mouch data ot once.
	 */
	size_t const write_max { 40 };   /* TODO from config (USB related)*/
	size_t       remaining { cmd.num_bytes() };
	size_t       offset    { 0 };
	int          rc        { 0 };
	while (remaining) {
		size_t to_write       { min(write_max, remaining) };
		rc = _write(cmd, offset, to_write);

		if (rc < 0) {
			// TODO:
			error("000000000000000000000000");
		}
		remaining -= to_write;
		offset    += to_write;

		_timer.msleep(5);
	}

	if (cmd.num_bytes() > 1000) {
		_timer.msleep(100);
		_read();
	} else {
		_read();
	}

	return rc;
}


int Usb::Cdc_acm::read_response(Command &cmd)
{
	size_t actual_length { 0 };
	{
		Mutex::Guard guard { _response_buffer_lock };
		actual_length = _response_buffer.num_bytes();

		cmd.try_append(_response_buffer, [] (Command &) { /* this can't be reached */ });
		_response_buffer.clear();
	}

//error("  >>> response  >  ",cmd.num_bytes());
	return actual_length;
}
