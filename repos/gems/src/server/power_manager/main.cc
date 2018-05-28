/*
 * \brief  Component to control the runtime of the sculpt systerm
 * \author Pirmin duss
 * \date   2018-05-28
 */

/* Genode includes */
#include <base/log.h>
#include <base/component.h>
#include <base/attached_rom_dataspace.h>

namespace Power_manager {
	using namespace Genode;
	class Main;
};

class Power_manager::Main {
	
	private:
		Env& _env;

		Attached_rom_dataspace _config{_env, "config"};
		Signal_handler<Main>   _config_handler{_env.ep(), *this, &Main::_config_changed};

		Attached_rom_dataspace _acpi_ac_report{_env, "acpi_ac"};
		Signal_handler<Main>   _acpi_ac_handler{_env.ep(), *this, &Main::_acpi_changed};

		Attached_rom_dataspace _acpi_battery_report{_env, "acpi_battery"};
		Signal_handler<Main>   _acpi_battery_handler{_env.ep(), *this, &Main::_acpi_changed};

		Attached_rom_dataspace _acpi_ec_report{_env, "acpi_ec"};
		Signal_handler<Main>   _acpi_ec_handler{_env.ep(), *this, &Main::_acpi_changed};

		void _config_changed() {
		}

		void _acpi_changed() {
		}

	public:
		Main(Genode::Env& env) :
			_env{env}
		{
			_config.sigh(_config_handler);
			_acpi_ac_report.sigh(_acpi_ac_handler);
		}
};

void Component::construct(Genode::Env& env) {
	static Power_manager::Main main{env};
}
