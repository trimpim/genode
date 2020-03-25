/*
 * \brief  File system stress tester
 * \author Pirmin Duss
 * \date   2020-03-23
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 * Copyright (C) 2020 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/exception.h>
#include <base/heap.h>
#include <base/log.h>
#include <vfs/simple_env.h>


using namespace Genode;
using namespace Vfs;

using File_name = Genode::String<64>;


namespace Genode {
	void print(Output& out, Vfs::Directory_service::Open_result res)
	{
		using Open_result = Vfs::Directory_service::Open_result;

		switch (res) {
			case Open_result::OPEN_ERR_UNACCESSIBLE:
				print(out, "OPEN_ERR_UNACCESSIBLE");
				break;
			case Open_result::OPEN_ERR_NO_PERM:
				print(out, "OPEN_ERR_NO_PERM");
				break;
			case Open_result::OPEN_ERR_EXISTS:
				print(out, "OPEN_ERR_EXISTS");
				break;
			case Open_result::OPEN_ERR_NAME_TOO_LONG:
				print(out, "OPEN_ERR_NAME_TOO_LONG");
				break;
			case Open_result::OPEN_ERR_NO_SPACE:
				print(out, "OPEN_ERR_NO_SPACE");
				break;
			case Open_result::OPEN_ERR_OUT_OF_RAM:
				print(out, "OPEN_ERR_OUT_OF_RAM");
				break;
			case Open_result::OPEN_ERR_OUT_OF_CAPS:
				print(out, "OPEN_ERR_OUT_OF_CAPS");
				break;
			case Open_result::OPEN_OK:
				print(out, "OPEN_OK");
				break;
		};
	}


	void print(Output& out, Vfs::File_io_service::Write_result res)
	{
		using Write_result = Vfs::File_io_service::Write_result;

		switch (res) {
			case Write_result::WRITE_ERR_AGAIN:
				print(out, "WRITE_ERR_AGAIN");
				break;
			case Write_result::WRITE_ERR_WOULD_BLOCK:
				print(out, "WRITE_ERR_WOULD_BLOCK");
				break;
			case Write_result::WRITE_ERR_INVALID:
				print(out, "WRITE_ERR_INVALID");
				break;
			case Write_result::WRITE_ERR_IO:
				print(out, "WRITE_ERR_IO");
				break;
			case Write_result::WRITE_ERR_INTERRUPT:
				print(out, "WRITE_ERR_INTERRUPT");
				break;
			case Write_result::WRITE_OK:
				print(out, "WRITE_OK");
				break;
		};
	}
}


class Main
{
	private:

		using Write_result = Vfs::File_io_service::Write_result;
		using Path         = Genode::Path<Vfs::MAX_PATH_LEN>;

		struct Test_failed : public Exception { };

		Genode::Env            &_env;
		Heap                    _heap            { _env.ram(), _env.rm() };
		Attached_rom_dataspace  _config_rom      { _env, "config" };
		Xml_node                _config_xml      { _config_rom.xml() };
		Simple_env              _vfs_env         { _env, _heap,
		                                           _config_xml.sub_node("vfs") };
		File_system            &_vfs_root        { _vfs_env.root_dir() };
		Vfs_handle             *_vfs_root_handle { nullptr };

		Main(const Main&)             = delete;
		Main& operator =(const Main&) = delete;

		void _write_file(File_name const &name, size_t const num_bytes,
		                 Write_result const expected_result)
		{
			auto const  mode     { Directory_service::OPEN_MODE_WRONLY +
			                       Directory_service::OPEN_MODE_CREATE };
			Vfs_handle *handle   { nullptr };
			auto const  open_res { _vfs_root.open(name.string(), mode,
			                                      &handle, _heap) };
			if (open_res != Vfs::Directory_service::Open_result::OPEN_OK) {
				error("couldn't open file ", name,
				      " for write (", open_res, ")");
				throw Test_failed { };
			}

			warning(" ====================== ",Hex{handle->status_flags()});

			Attached_ram_dataspace ds { _env.ram(), _env.rm(), num_bytes };
			memset(ds.local_addr<int>(), 0xa55a, ds.size()/sizeof(int));

			Vfs_handle::Guard guard         { handle };
			Vfs::file_size    bytes_written { 0 };

//			volatile bool loop { true };
//			while (loop) { }
			auto const write_res { _vfs_root.write(handle,
			                                       ds.local_addr<const char>(),
			                                       num_bytes, bytes_written) };
			if (expected_result != write_res) {
				error("Write result incorrect: expcted = ", expected_result,
				      " ; actual = ", write_res, " ; file = ", name);
				throw Test_failed { };
			}
		}

	public:

		Main(Genode::Env& env) :
			_env { env }
		{
			auto const res { _vfs_root.opendir("/data/", false,
			                                   &_vfs_root_handle, _heap) };
			if (res != Vfs::Directory_service::Opendir_result::OPENDIR_OK) {
				error("opendir() failed on ", "/data/");
				throw Test_failed { };
			}

			_write_file("/data/test1", 512, Write_result::WRITE_OK);
			_write_file("/data/test2", 512, Write_result::WRITE_OK);
			_write_file("/data/test3", 512, Write_result::WRITE_ERR_IO);


			log("Test success.");
		}

		virtual ~Main()
		{
			_vfs_root.close(_vfs_root_handle);
		}
};


void Component::construct(Genode::Env& env)
{
	static Main main { env };
}
