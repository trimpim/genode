
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <os/reporter.h>
#include <util/xml_node.h>

#include <timer_session/connection.h>

namespace Test
{
	using namespace Genode;

	struct Main;
}


struct Test::Main
{
	using Arch = String<16>;

	enum class Current_deploy { deploy_1, deploy_2, };

	static size_t const US_PER_S { 1000 * 1000  };

	Env                     &env;

	Attached_rom_dataspace   config         { env, "config" };
	Attached_rom_dataspace   deploy_in_1    { env, "deploy_1" };
	Attached_rom_dataspace   deploy_in_2    { env, "deploy_2" };
	Attached_rom_dataspace   state          { env, "state" };

	Expanding_reporter       deploy_out     { env, "config", "deploy_config" };

	Timer::Connection        timer          { env };
	Signal_handler<Main>     timer_sigh     { env.ep(), *this, &Main::handle_timer };

	Current_deploy           current_deploy { Current_deploy::deploy_1 };
	Arch                     arch           { arch_from_config() };
	size_t                   delay_1_s      { delay_1_from_config() };
	size_t                   delay_2_s      { delay_2_from_config() };

	Arch arch_from_config()
	{
		config.update();
		return config.xml().attribute_value("arch", Arch { "unknown" });
	}

	size_t delay_1_from_config()
	{
		config.update();
		return config.xml().attribute_value("delay_1_s", 15);
	}

	size_t delay_2_from_config()
	{
		config.update();
		return config.xml().attribute_value("delay_2_s", 15);
	}

	void handle_timer()
	{
		if (current_deploy == Current_deploy::deploy_1) {
			log("write deploy config 1");

			deploy_out.generate([this] (Xml_generator &xml) {
				xml.attribute("arch", arch);

				deploy_in_1.xml().with_raw_content([&xml] (char const *start, size_t len) {
					xml.append(start, len);
				});
			});
			current_deploy = Current_deploy::deploy_2;
			timer.trigger_once(delay_1_s * US_PER_S);
		} else {
			log("write deploy config 2");

			deploy_out.generate([this] (Xml_generator &xml) {
				xml.attribute("arch", arch);

				deploy_in_2.xml().with_raw_content([&xml] (char const *start, size_t len) {
					xml.append(start, len);
				});
			});
			current_deploy = Current_deploy::deploy_1;
			timer.trigger_once(delay_2_s * US_PER_S);
		}
	}

	Main(Env &env)
	:
		env { env }
	{
		timer.sigh(timer_sigh);

		/* write the first config after 3 seconds */
		timer.trigger_once(3 * US_PER_S);
	}
};


void Component::construct(Genode::Env &env)
{
	static Test::Main main { env };
}
