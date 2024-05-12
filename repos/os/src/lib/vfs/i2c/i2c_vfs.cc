/*
 *  \brief  VFS plugin interface implementation
 *  \author Alice Domage
 *  \author Pirmin Duss
 *  \date   2022-06-28
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <vfs/file_system_factory.h>
#include <vfs/single_file_system.h>
#include <util/string.h>

/* Local includes */
#include "i2c_file_system.h"


/**************************
 ** VFS plugin interface **
 **************************/

extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	struct Factory : Vfs::File_system_factory
	{
		Vfs::File_system *create(Vfs::Env &vfs_env,
		                         Genode::Xml_node node) override
		{
			try {
				return new (vfs_env.alloc()) Vfs_i2c::I2c_file_system { vfs_env, node };
			} catch (...) {
				Genode::error("could not create 'i2c vfs'");
			}
			return nullptr;
		}
	};

	static Factory factory;
	return &factory;
}
