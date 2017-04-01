/*
 * \brief  this server provides pseudo random numbers generated
 *         by the xoroshiro algorythm.
 * \author Pirmin Duss
 * \date   2016-11-08
 */

/*
 * Copyright (C) 2008-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/component.h>
#include <base/log.h>
#include <base/heap.h>
#include <root/component.h>

#include <base/rpc_server.h>
#include <util/arg_string.h>
#include <util/xml_node.h>
#include <terminal_session/terminal_session.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>

#include "xoroshiro.h"

namespace Xsoroshiro_terminal {
	class Session_component;
	class Root_component;
	struct Main;

	using namespace Terminal;
	using namespace Genode;
}


class Xsoroshiro_terminal::Session_component : public Rpc_object<Terminal::Session, Session_component>
{
	public:

		Session_component(Env& env, uint64_t seed) :
			_io_buffer(env.ram(), env.rm(), IO_BUFFER_SIZE),
			_xoroshiro{seed}
    {
			log(" seeding pseudo random with seed=", seed);
		}

		virtual ~Session_component() = default;

		Session::Size size() override { return Session::Size(0, 0); }

		bool avail() override { return true; }

		size_t _read(size_t dst_len)
		{
			unsigned       num_bytes = 0;
			unsigned char *dst       = _io_buffer.local_addr<unsigned char>();
			Genode::size_t dst_size  = Genode::min(_io_buffer.size(), dst_len);
			do {
				uint64_t tmp = _xoroshiro.get();
				for (size_t i=0; (i<8)&&(num_bytes<dst_size); ++i) {
					dst[num_bytes++] = (tmp >> (i*8)) & 0xFF;
				}
			} while (num_bytes < dst_size);

			return num_bytes;
		}

		size_t _write(size_t num_bytes)
		{
			warning("write to xoroshiro_terminal not supported.");
			return 0;
		}

		Dataspace_capability _dataspace()
		{
			return _io_buffer.cap();
		}

		void read_avail_sigh(Signal_context_capability sigh) override { }

		void connected_sigh(Signal_context_capability sigh) override
		{
			Signal_transmitter(sigh).submit();
		}

		size_t read(void *buf, size_t) { return 0; }
		size_t write(void const *buf, size_t) { return 0; }

	private:

		enum { IO_BUFFER_SIZE = 4096 };

		Attached_ram_dataspace _io_buffer;
		Xoroshiro _xoroshiro;
};


class Xsoroshiro_terminal::Root_component : public Genode::Root_component<Session_component>
{
	public:

		Root_component(Env& env, Allocator &alloc) :
			Genode::Root_component<Session_component>(env.ep(), alloc),
			_env(env)
		{
			try {
				Attached_rom_dataspace config(env, "config");
				_seed = config.xml().sub_node("seed").attribute_value("value", 42UL);
			} catch (Rom_connection::Rom_connection_failed) {
				_seed = 42;
			}
		}

	protected:

		Session_component *_create_session(const char *args)
		{
			return new (md_alloc()) Session_component(_env, _seed);
		}

	private:

		Env& _env;
		uint64_t _seed;
};


class Xsoroshiro_terminal::Main
{
	public:

		Main(Env &env) : _env(env)
		{

			/*
			 * Create a RPC object capability for the root interface and
			 * announce the service to our parent.
			 */
			env.parent().announce(env.ep().manage(root));
		}

	private:

		Env& _env;

		/*
		 * A sliced heap is used for allocating session objects - thereby we
		 * can release objects separately.
		 */
		Sliced_heap sliced_heap { _env.ram(),  _env.rm() };

		Xsoroshiro_terminal::Root_component root { _env, sliced_heap };
};


void Component::construct(Genode::Env &env) { static Xsoroshiro_terminal::Main main(env); }
