/**
 * \brief  merger for block devices
 * \author Pirmin Duss
 * \date   2018-04-20
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/log.h>
#include <block/component.h>
#include <os/session_policy.h>
#include <util/xml_node.h>
#include <os/reporter.h>


namespace Block_raid0 {
	class Factory;
	class Root_multiple_clients;
	class Main;
}


struct Block_raid0::Factory : Driver_factory
{
	long device_num;

	Block_raid0::Driver *create()
	{
		return Ahci_driver::claim_port(device_num);
	}

	void destroy(Block_raid0::Driver *)
	{
		Ahci_driver::free_port(device_num);
	}

	Factory(long device_num) : device_num(device_num) { }
};


class Session_component : public Block_raid0::Session_component
{
	public:

		Session_component(Block_raid0::Driver_factory &driver_factory,
		                  Genode::Entrypoint    &ep,
		                  Genode::Region_map    &rm,
		                  Genode::size_t         buf_size,
		                  bool                   writeable)
		: Block_raid0::Session_component(driver_factory, ep, rm, buf_size, writeable) { }

		Block_raid0::Driver_factory &factory() { return _driver_factory; }
};


class Block_raid0::Root_multiple_clients : public Root_component< ::Session_component>,
                                     public Ahci_root
{
	private:

		Genode::Env       &_env;
		Genode::Allocator &_alloc;
		Genode::Xml_node   _config;

	protected:

		::Session_component *_create_session(const char *args)
		{
			return nullptr;
			//return session;
		}

		void _destroy_session(::Session_component *session)
		{
		}

	public:

		Root_multiple_clients(Genode::Env &env, Genode::Allocator &alloc,
		                      Genode::Xml_node config)
		:
			Root_component(&env.ep().rpc_ep(), &alloc),
			_env(env), _alloc(alloc), _config(config)
		{ }

		Genode::Entrypoint &entrypoint() override { return _env.ep(); }

		void announce() override
		{
			_env.parent().announce(_env.ep().manage(*this));
		}
};


struct Block_raid0::Main
{
	Genode::Env  &env;
	Genode::Heap  heap { env.ram(), env.rm() };

	Genode::Attached_rom_dataspace config { env, "config" };

	Genode::Constructible<Genode::Reporter> reporter { };

	Block_raid0::Root_multiple_clients root;

	Signal_handler<Main> device_identified {
		env.ep(), *this, &Main::handle_device_identified };

	Main(Genode::Env &env)
	: env(env), root(env, heap, config.xml())
	{
	}

	void handle_device_identified()
	{
	}
};


void Component::construct(Genode::Env &env) { static Block_raid0::Main server(env); }
