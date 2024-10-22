
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/heap.h>
#include <timer_session/connection.h>
#include <vfs/simple_env.h>

namespace Test
{
	using namespace Genode;

	struct Main;
}

struct Test::Main
{

	Env                    &env;
	Attached_rom_dataspace  config     { env, "config" };
	size_t                  wait_time  { time_from_config() };

	Timer::Connection       timer      { env };
	Signal_handler<Main>    timer_sigh { env.ep(), *this, &Main::handle_timer };

	Heap                    heap       { env.ram(), env.rm() };
	Vfs::Simple_env         vfs_env    { env, heap, config.xml().sub_node("vfs") };

	void handle_timer()
	{
		log("PD-faulter will fault now");

		Attached_rom_dataspace ds      { env, "config" };
		ds.update();

		int *fault_addr { nullptr };
		*fault_addr = ds.xml().attribute_value("test", 555);
	}

	size_t time_from_config()
	{
		config.update();
		return config.xml().attribute_value("wait_time", static_cast<size_t>(2000));
	}

	Main(Env &env)
	:
		env { env }
	{
		log("PD-faulter started");

		timer.sigh(timer_sigh);
		timer.trigger_once(wait_time * 1000);
	}
};

void Component::construct(Genode::Env &env)
{
	static Test::Main main { env };
}
