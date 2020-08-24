/*
 * \brief  Platform specific i2c driver for imx8q_evk
 * \author Pirmin Duss <pirmin.duss@gapfruit.com>
 * \author Stefan Kalkowski
 * \data   2020-08-24
 */

 /*
  * Copyright (C) 2009-2017 Genode Labs GmbH
  * Copyright (C) 2020 gapfruit AG
  *
  * This file is part of the Genode OS framework, which is distributed
  * under the terms of the GNU Affero General Public License version 3.
  */

#ifndef _DRIVERS__I2C__SPEC__IMX8Q__DRIVERS_H_
#define _DRIVERS__I2C__SPEC__IMX8Q__DRIVERS_H_

/* Genode includes */
#include <base/attached_dataspace.h>
#include <drivers/defs/imx8q_evk.h>
#include <platform_session/connection.h>
#include <irq_session/client.h>
#include <timer_session/connection.h>
#include <util/reconstructible.h>
#include <util/xml_node.h>

/* local includes */
#include "driver_base.h"
#include "irq_handler.h"
#include "mmio.h"

namespace I2c
{
	using namespace Genode;

	using Device_name = Genode::String<10>;

	class Driver;
}


class I2c::Driver : private Platform::Connection,
                    public Driver_base
{
	private:

		class No_ack : Genode::Exception {};

		enum
		{
			Slave_mode  = 0,
			Master_mode = 1
		};

		Env&                               _env;
		Constructible<Attached_dataspace>  _mmio_ds { };
		Constructible<Mmio>                _mmio    { };
		Constructible<Irq_handler>         _irq     { };

		Device_name _device_name_from_config(Xml_node const &config) const;
		void _init_platform_connection(Xml_node const &config);

		void _busy() { while (!_mmio->read<Mmio::Status::Busy>()); }
		void _start();
		void _stop();
		void _write(uint8_t value);

	public:

		Driver(Env &env, Xml_node const &config)
		:
			Platform::Connection { env },
			_env                 { env }
		{
			_init_platform_connection(config);
		}

		void send(uint8_t addr, const uint8_t *buf, size_t num) override;
		void recv(uint8_t addr, uint8_t *buf, size_t num) override;
};

#endif /* _DRIVERS__I2C__SPEC__IMX8Q__DRIVERS_H_ */
