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


namespace
{
	using Genode::Array;
	using Genode::uint8_t;

//	Array<uint8_t, 8> _create_read_array(uint8_t size)
//	{
//		Array<uint8_t, 8> out { };
//		for (uint8_t idx = 0; idx < size; ++idx) {
//			out.add(static_cast<uint8_t>(0x00));
//		}
//		return out;
//	}

	Array<uint8_t, 8> _create_write_array(uint8_t size, uint8_t const *data)
	{
		Array<uint8_t, 8> out { };
		for (uint8_t idx = 0; idx < size; ++idx) {
			out.add(data[idx]);
		}
		return out;
	}
}


class Vfs::Device_file_system : public Vfs::Single_file_system
{
	private:

		struct Vfs_handle : Single_vfs_handle
		{

			I2c::Driver_base  &_driver;
			I2c::Bus_address   _bus_addrss;
			uint8_t            _tx_buffer[32];
			uint8_t            _rx_buffer[32];

			Vfs_handle(Directory_service &ds,
			           File_io_service   &fs,
			           Allocator         &alloc,
			           I2c::Driver_base  &driver,
			           I2c::Bus_address  bus_address)
			:
				Single_vfs_handle { ds, fs, alloc, 0 },
				_driver { driver }, _bus_addrss { bus_address }
			{ }

			Read_result read(Byte_range_ptr const &dst, size_t &out_count) override
			{
log(">>>>>>> ",__func__,"()");
				(void)dst;
				(void)out_count;
				// TODO:
				return READ_OK;
			}

			Write_result write(Const_byte_range_ptr const &src, size_t &out_count) override
			{
log(">>>>>>> ",__func__,"()");

				size_t remaining = src.num_bytes;
				uint8_t const* current = reinterpret_cast<uint8_t const*>(src.start);
				while (remaining > 0) {

					uint8_t count { min(static_cast<uint8_t>(remaining), I2c::Message::MAX_LEN) };
					I2c::Transaction trxn_read_data {
						I2c::Message { I2c::Message::WRITE, _create_write_array(count, current) },
						I2c::Message { I2c::Message::READ,  static_cast<uint8_t>(0x0) },
					};

					_driver.transfer(_bus_addrss, trxn_read_data);
					remaining -= count;
					current   += count;
				}
				out_count = src.num_bytes;
				return Write_result::WRITE_OK;
			}

			// TODO:
			bool read_ready()  const override { return true; }
			bool write_ready() const override { return false; }
		};

		Vfs::Env          &_env;
		I2c::Driver_base  &_driver;
		I2c::Bus_address   _bus_addrss;

	public:

		Device_file_system(Vfs::Env &env, Genode::Xml_node config,
		                   I2c::Driver_base &driver)
		:
			Single_file_system { Vfs::Node_type::TRANSACTIONAL_FILE,
			                     type_name(), Node_rwx::rw(), config },
			_env               { env },
			_driver            { driver },
			_bus_addrss        { .address = config.attribute_value("bus_address", static_cast<uint8_t>(0x00)) }
		{ }

		static char const *type_name() { return "i2c"; }

		char const *type() override { return type_name(); }

		/*********************************
		 ** Directory-service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned,
		                 Vfs::Vfs_handle **out_handle,
		                 Allocator   &alloc) override
		{
			if (!_single_file(path))
				return OPEN_ERR_UNACCESSIBLE;

			try {
				*out_handle = new (alloc)
					Vfs_handle(*this, *this, alloc, _driver, _bus_addrss);
				return OPEN_OK;
			}
			catch (Out_of_ram)  { return OPEN_ERR_OUT_OF_RAM; }
			catch (Out_of_caps) { return OPEN_ERR_OUT_OF_CAPS; }
		}

};


#endif /* _VFS_I2C__DEVICE_FILE_SYSTEM_H_ */
