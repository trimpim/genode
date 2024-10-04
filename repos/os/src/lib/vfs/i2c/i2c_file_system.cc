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

/* Genode includes */
#include <base/log.h>

/* local includes */
#include "i2c_file_system.h"


I2c::Settings Vfs_i2c::Local_factory::_settings_from_config(Xml_node const &config)
{
	return I2c::Settings { config.attribute_value("bus_speed_khz", static_cast<uint16_t>(400)),
	                       config.attribute_value("verbose", false),
	                       config.attribute_value("name", I2c::Name { }) };
}

Vfs_i2c::Local_factory::Local_factory(Vfs::Env &env, Xml_node const &config)
:
	_env      { env },
	_config   { config },
	_settings { _settings_from_config(_config) },
	_driver   { _create_driver_instance(_env.env(), _heap, _env.user(), _settings) }
{
	log(" Settings:");
	log("    bus_speed_khz : ", _settings.bus_speed_khz);
	log("    name          : ", _settings.name);
	log("    verbose       : ", _settings.verbose ? "true" : "false");
}


void Vfs_i2c::Local_factory::apply_config(Genode::Xml_node const &config)
{
	config.for_each_sub_node([] (Xml_node const &device) {
		warning(device);
	});

	// TODO:
}
