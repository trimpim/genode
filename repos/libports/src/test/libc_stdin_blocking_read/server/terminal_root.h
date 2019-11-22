
#pragma once


// Genode includes
#include <base/allocator.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <os/session_policy.h>
#include <root/component.h>

// local includes
#include "terminal_session_component.h"


namespace Libc_stdin_server {

	using namespace Genode;

	class Terminal_root;
}


class Libc_stdin_server::Terminal_root final: public Root_component<Session_component, Single_client>
{
	private:

		Env&                        _env;
		Allocator&                  _alloc;
		Session_component*          _current_session;
		Signal_context_capability   _connected_cap;

		// delete some methods to make the compiler happy
		Terminal_root(const Libc_stdin_server::Terminal_root&)            = delete;
		Terminal_root& operator=(const Libc_stdin_server::Terminal_root&) = delete;

	protected:

		Session_component* _create_session(char const*) override
		{
			_current_session = new (_alloc) Session_component { _env };

			Signal_transmitter(_connected_cap).submit();

			return _current_session;
		}

		void _destroy_session(Session_component *session) override
		{
			Genode::destroy(md_alloc(), session);
			_current_session = nullptr;
		}

	public:

		Terminal_root(Env &env, Allocator &alloc,
		              Signal_context_capability connected_cap) :
			Root_component{&env.ep().rpc_ep(), &alloc},
			_env{env},
			_alloc{alloc},
			_current_session{nullptr},
			_connected_cap{connected_cap}
		{ }

		Session_component* current_session() const { return _current_session; }
};
