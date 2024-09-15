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


Vfs_i2c::Local_factory::Local_factory(Vfs::Env &env, Xml_node config)
:
	_env      { env },
	_driver   { _create_driver_instance(_env.env(), _settings) },
	_config   { config }
{
error(__func__,"()  ::  ",__LINE__);
}


void Vfs_i2c::Local_factory::apply_config(Genode::Xml_node const &config)
{
error(__func__,"()  ::  ",__LINE__);
	config.for_each_sub_node([] (Xml_node const &device) {
		warning(device);
	});

	// TODO:
}
