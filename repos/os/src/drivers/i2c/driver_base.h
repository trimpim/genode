/*
 * \brief  Interface to instantiate a platform specific i2c driver
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

#ifndef _DRIVERS__I2C__DRIVER_BASE_H
#define _DRIVERS__I2C__DRIVER_BASE_H

/* Genode includes */
#include <base/env.h>
#include <util/xml_node.h>

namespace I2c
{
	using namespace Genode;

	class Driver_base;
}


class I2c::Driver_base
{
	public:

		virtual ~Driver_base() = default;

		/**
		 * send data to a device on the bus.
		 *
		 * \param   address    address of the client to communicate with
		 * \param   buf	       buffer containing bytes to send
		 * \param   num        number of bytes to send
		 */
		virtual void send(uint8_t addr, const uint8_t *buf, size_t num) = 0;

		/**
		 * receive data from a device on the bus.
		 *
		 * \param   address    address of the client to communicate with
		 * \param   buf        buffer to store received data in to
		 * \param   num        size of the buffer
		 */
		virtual void recv(uint8_t addr, uint8_t *buf, size_t num) = 0;
};

#endif /* _DRIVERS__I2C__DRIVER_BASE_H */