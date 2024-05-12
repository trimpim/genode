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

#ifndef _VFS_I2C__DEVICE_FILE_SYSTEM_H_
#define _VFS_I2C__DEVICE_FILE_SYSTEM_H_

/* Genode includes */
#include <vfs/single_file_system.h>

/* i2c includes */
#include <i2c/driver_base.h>


namespace Vfs{

	using namespace Genode;

	class Device_file_system_element;
	class Device_file_system;
}


class Vfs::Device_file_system_element : Genode::List<Device_file_system_element>::Element
{
	private:

		Device_file_system *_element;

	public:

		Device_file_system_element(Device_file_system *element)
		:
			_element { element }
		{ }
};


class Vfs::Device_file_system final : public Vfs::Single_file_system
{
	private:

		struct Vfs_handle : Single_vfs_handle
		{

			I2c::Driver_base &_driver;

			Vfs_handle(Directory_service &ds,
			           File_io_service   &fs,
			           Allocator         &alloc,
			           I2c::Driver_base  &driver)
			:
				Single_vfs_handle { ds, fs, alloc, 0 }, _driver { driver }
			{
log(">>>>>>> ",__func__,"()");
			}

			Read_result read(Byte_range_ptr const &dst, size_t &out_count) override
			{
log(">>>>>>> ",__func__,"()");
				(void)dst;
				(void)out_count;
				// TODO:
				return READ_OK;
			}

			Write_result write(Const_byte_range_ptr const &, size_t &) override
			{
log(">>>>>>> ",__func__,"()");
				// TODO:
				return WRITE_ERR_IO;
			}

			// TODO:
			bool read_ready()  const override { return true; }
			bool write_ready() const override { return false; }
		};

		Vfs::Env         &_env;
		I2c::Driver_base &_driver;

	public:

		Device_file_system(Vfs::Env &env, Genode::Xml_node config,
		                   I2c::Driver_base &driver)
		:
			Single_file_system { Vfs::Node_type::TRANSACTIONAL_FILE,
			                     type_name(), Node_rwx::rw(), config },
			_env               { env },
			_driver            { driver }
		{
log(">>>>>>> ",__func__,"()");
log(config);
		}

		static char const *type_name() { return "i2c"; }

		char const *type() override { return type_name(); }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Allocator   &alloc) override
		{
log(">>>>>>> ",__func__,"()");
			if (!_single_file(path))
				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle = new (alloc)
					Vfs_handle(*this, *this, alloc, _driver);
				return OPEN_OK;
			}
			catch (Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

};


#endif /* _VFS_I2C__DEVICE_FILE_SYSTEM_H_ */
