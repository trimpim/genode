
#include <base/component.h>
#include <base/heap.h>
#include <timer_session/connection.h>

#include "terminal_root.h"


namespace Libc_stdin_server {

	using namespace Genode;

	class Main;
}


class Libc_stdin_server::Main
{
	public:

		Main(Env& env) : _env { env }
		{
			_timer.sigh(_timeout);
			_env.parent().announce(_env.ep().manage(_terminal));

			log("test server started");

		}

		~Main() = default;

	private:

		using Handler = Signal_handler<Main>;

		enum { TEST_DELAY = 1000 * 100, };

		Env&               _env;
		Timer::Connection  _timer     { _env };
		Heap               _heap      { _env.ram(), _env.rm() };
		Handler            _connected { _env.ep(), *this,
		                                &Main::_handle_connected };
		Handler            _timeout   { _env.ep(), *this,
		                                &Main::_handle_timeout };
		Terminal_root      _terminal  { _env, _heap, _connected };

		void _handle_connected()
		{
			log("client connected -> sending data");

			// wait until the client is fully started
			_timer.trigger_once(TEST_DELAY);
		}

		void _handle_timeout()
		{
			String<50> temp { "request;" };
			_terminal.current_session()->write(temp.string(), temp.length());
		}
};


void Component::construct(Genode::Env& env)
{
	static Libc_stdin_server::Main main { env };
}
