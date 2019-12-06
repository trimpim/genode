
#include <base/component.h>
#include <base/attached_rom_dataspace.h>


namespace Test_lx_fs_notify
{
	using namespace Genode;

	class Main;
};


class Test_lx_fs_notify::Main
{
	private:

		Env&                   _env;
		Signal_handler<Main>   _update_handler { _env.ep(), *this, &Main::_update };
		Attached_rom_dataspace _test_rom       { _env, "test.txt" };

		void _update()
		{
			log("Test successful.");
		}

	public:

		Main(Env& env) : _env { env }
		{
			log("wait for file change");
			_test_rom.sigh(_update_handler);
		}
};


void Component::construct(Genode::Env& env)
{
	static Test_lx_fs_notify::Main main { env };
}
