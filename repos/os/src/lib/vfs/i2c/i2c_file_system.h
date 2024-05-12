/*
 *  \brief  VFS plugin TODO:
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
#include <os/reporter.h>
//#include <vfs/env.h>
#include <vfs/dir_file_system.h>
#include <os/vfs.h>

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
//	using Device_list = List<Device_file_system_element>;

	Vfs::Env                 &_env;
	I2c::Settings             _settings;
	I2c::Driver_base         &_driver;
	Xml_node                  _config;
	Vfs::Device_file_system   _device { _env, _config, _driver };

	Local_factory(Vfs::Env &env, Xml_node config);

	Vfs::File_system *create(Vfs::Env&, Xml_node node) override
	{
	// TODO:
	log(">>>>>>> ",__func__,"()");
	log(node);

		return &_device;
	}

	void apply_config(Xml_node const &config);

	void watch_response() override { }
};


class Vfs_i2c::I2c_file_system final : public Local_factory,
                                       public Dir_file_system
{
	private:

		using Config = String<200>;

		uint16_t  _bus_speed_khz;
		bool      _verbose;

		static Config _config(Xml_node node)
		{
			char buf[Config::capacity()] { };

			Xml_generator xml(buf, sizeof(buf), "dir", [&] () {
				xml.attribute("name", "i2c");
				using Name = String<64>;
				node.for_each_sub_node("device", [&] (Xml_node node) {
					xml.node("dir", [&] () {
						xml.attribute("name", node.attribute_value("name", Name { }));
					});
				});
			});
			return Config { Cstring(buf) };
		}

	public:

		I2c_file_system(Vfs::Env &vfs_env, Genode::Xml_node node)
		:
			Local_factory        { vfs_env, node },
			Vfs::Dir_file_system { vfs_env, Xml_node { _config(node).string() }, *this },
			_bus_speed_khz       { node.attribute_value("bus_speed_khz", static_cast<uint16_t>(400)) },
			_verbose             { node.attribute_value("verbose", false) }
		{
			warning("====================================================");
			log(node);
			warning("====================================================");
		}
};


#endif /* _VFS_I2C__I2C_FILE_SYSTEM_H_*/
