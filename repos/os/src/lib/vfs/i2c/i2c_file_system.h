/*
 *  \brief  VFS plugin for i2c
 *  \author Pirmin Duss
 *  \date   2024-03-29
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VFS_I2C__I2C_FILE_SYSTEM_H_
#define _VFS_I2C__I2C_FILE_SYSTEM_H_

/* Genode includes */
#include <base/heap.h>
#include <os/reporter.h>
#include <os/vfs.h>
#include <vfs/dir_file_system.h>
//#include <vfs/env.h>

/* i2c includes */
#include <i2c/driver_base.h>

/* local includes */
#include "device_file_system.h"


namespace Vfs_i2c {

	using namespace Genode;
	using namespace Vfs;

	struct Local_factory;

	class I2c_file_system;
}


struct Vfs_i2c::Local_factory : File_system_factory, Watch_response_handler
{
	Vfs::Env                 &_env;
	Xml_node                  _config;
	Heap                      _heap       { _env.env().ram(), _env.env().rm() };
	I2c::Settings             _settings;
	I2c::Driver_base         &_driver;
	Vfs::Device_file_system   _device     { _env, _device_config(_config), _driver };

	Xml_node _device_config(Xml_node config)
	{
		return config.sub_node("device");
	}

	I2c::Settings _settings_from_config(Xml_node const &config);

	Local_factory(Vfs::Env &env, Xml_node const &config);

	Vfs::File_system *create(Vfs::Env&, Xml_node) override
	{
	// TODO:

		return &_device;
	}

	I2c::Settings const &settings() const { return _settings; }

	void apply_config(Xml_node const &config);

	void watch_response() override { }
};


class Vfs_i2c::I2c_file_system final : public Local_factory,
                                       public Dir_file_system
{
	private:

		using Config = String<200>;

		static Config _config(Xml_node node)
		{
			using Name = String<64>;

			char buf[Config::capacity()] { };

//			Xml_node device { node.sub_node("device") };
			Xml_generator xml(buf, sizeof(buf), "config", [node, &xml] () {
				xml.node("bus", [node, &xml] {
					node.for_each_sub_node([&xml] (Xml_node device) {
						xml.node("device", [device, &xml] {
							xml.attribute("name",        device.attribute_value("name",        Name { }));
							xml.attribute("bus_address", device.attribute_value("bus_address", 0x00lu));
						});
					});
				});
//				xml.attribute("name", device.attribute_value("name", Name { }));
//				xml.attribute("bus_address", device.attribute_value("bus_address", 0x00lu));
//				xml.attribute("name", "i2c");
//				node.for_each_sub_node("device", [&] (Xml_node node) {
//					xml.node("device", [&] () {
//						xml.attribute("name", node.attribute_value("name", Name { }));
//						xml.attribute("bus_address", node.attribute_value("bus_address", 0x00lu));
//					});
//				});
			});
			return Config { Cstring(buf) };
		}

	public:

		I2c_file_system(Vfs::Env &vfs_env, Genode::Xml_node node)
		:
			Local_factory        { vfs_env, node },
			Vfs::Dir_file_system { vfs_env, node, *this }
		{ }

		void apply_config(Xml_node const &config) override
		{
			Local_factory::apply_config(config);
		}
};


#endif /* _VFS_I2C__I2C_FILE_SYSTEM_H_*/
