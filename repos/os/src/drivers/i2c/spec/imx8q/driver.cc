/*
 * \brief  Platform specific i2c driver for imx8q_evk
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

/* local includes */
#include "driver.h"
#include "mmio.h"


namespace I2c
{
	using namespace Genode;

	using I2c::Mmio;
}


void I2c::Driver::_start()
{
log(__func__);
	/* input root 90 is 25Mhz target is 400Khz, divide by 64 */
	_mmio->write<Mmio::Freq_divider>(0x2a);
	_mmio->write<Mmio::Status>(0);
	_mmio->write<Mmio::Control>(Mmio::Control::Enable::bits(1));

	while (!_mmio->read<Mmio::Control::Enable>()) { ; }

	_mmio->write<Mmio::Control::Master_slave_select>(Master_mode);

	_busy();

	_mmio->write<Mmio::Control>(Mmio::Control::Tx_rx_select::bits(1)        |
	                            Mmio::Control::Tx_ack_enable::bits(1)       |
	                            Mmio::Control::Irq_enable::bits(1)          |
	                            Mmio::Control::Master_slave_select::bits(1) |
	                            Mmio::Control::Enable::bits(1));
}


void I2c::Driver::_stop()
{
log(__func__);
	_mmio->write<Mmio::Control>(0);
}


void I2c::Driver::_write(Genode::uint8_t value)
{
	_mmio->write<Mmio::Data>(value);

	do { _irq->wait(); }
	while (!_mmio->read<Mmio::Status::Irq>());

	_mmio->write<Mmio::Status::Irq>(0);
	_irq->ack();

	if (_mmio->read<Mmio::Status::Rcv_ack>()) throw No_ack();
}


I2c::Device_name I2c::Driver:: _device_name_from_config(Xml_node const &config) const
{
	size_t const bus_no { config.attribute_value("bus_no", 0UL) };
	return Genode::String<10> { "i2c", bus_no };
}


void I2c::Driver::_init_platform_connection(Xml_node const &config)
{
	using Io_mem_cap = Io_mem_dataspace_capability;
	using Platform::Device_client;

	Device_name const device_name { _device_name_from_config(config) };
	with_xml([this, &device_name] (Xml_node const &node) {

		node.for_each_sub_node("device", [this, &device_name] (Xml_node const &device_node) {

			Device_name name { device_node.attribute_value("name", Device_name()) };

			if (name != device_name) return;

			Device_client device     { acquire_device(name.string()) };
			auto const    iomem_node { device_node.sub_node("io_mem") };
			auto const    id         { iomem_node.attribute_value("id", 0U) };
			Io_mem_cap    mmio_cap   { device.io_mem_dataspace(id) };

			_mmio_ds.construct(_env.rm(), mmio_cap);
			_mmio.construct(reinterpret_cast<addr_t>(_mmio_ds->local_addr<uint16_t>()));

			_irq.construct(_env, device.irq());
		});
	});
}


void I2c::Driver::send(uint8_t addr, const uint8_t *buf, size_t num)
{
	while (true) {
		try {
			_start();

			_write(addr << 1);
			for (Genode::size_t i = 0; i < num; i++)
				_write(buf[i]);
			_stop();
			return;
		} catch(No_ack) { }
		_stop();
	}
}


void I2c::Driver::recv(uint8_t addr, uint8_t *buf, size_t num)
{
	while (true) {
		try {
			_start();

			_write(addr << 1 | 1);

			_mmio->write<Mmio::Control::Tx_rx_select>(0);
			if (num > 1)
				_mmio->write<Mmio::Control::Tx_ack_enable>(0);
			_mmio->read<Mmio::Data>(); /* dummy read */

			for (Genode::size_t i = 0; i < num; i++) {

				do { _irq->wait(); }
				while (!_mmio->read<Mmio::Status::Irq>());

				_mmio->write<Mmio::Status::Irq>(0);

				if (i == num-1) {
					_mmio->write<Mmio::Control::Tx_rx_select>(0);
					_mmio->write<Mmio::Control::Master_slave_select>(0);
					while (_mmio->read<Mmio::Status::Busy>()) ;
				} else if (i == num-2) {
					_mmio->write<Mmio::Control::Tx_ack_enable>(1);
				}

				buf[i] = _mmio->read<Mmio::Data>();
				_irq->ack();
			}

			_stop();
			return;
		} catch(No_ack) {
			Genode::log("no ack");
			 _stop();
		}
	}
}