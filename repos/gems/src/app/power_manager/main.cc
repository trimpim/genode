/*
 * \brief  Component to control the runtime of the sculpt systerm
 * \author Pirmin duss
 * \date   2018-05-28
 */

/* Genode includes */
#include <base/log.h>
#include <base/component.h>
#include <base/attached_rom_dataspace.h>

/* internal includes */
#include "xml_parsing.h"

namespace Power_manager {
	using namespace Genode;
	class Main;
};


/* ADA information exchange */
#include "ada_interface.h"

class Power_manager::Main {
	
	private:
		Env& _env;

		Attached_rom_dataspace _config{_env, "config"};
		Signal_handler<Main>   _config_handler{_env.ep(), *this, &Main::_config_changed};

		Attached_rom_dataspace _acpi_ac_report{_env, "acpi_ac"};
		Signal_handler<Main>   _acpi_ac_handler{_env.ep(), *this, &Main::_acpi_ac_changed};

		Attached_rom_dataspace _acpi_battery_report{_env, "acpi_battery"};
		Signal_handler<Main>   _acpi_battery_handler{_env.ep(), *this, &Main::_acpi_battery_changed};

		Attached_rom_dataspace _acpi_ec_report{_env, "acpi_ec"};
		Signal_handler<Main>   _acpi_ec_handler{_env.ep(), *this, &Main::_acpi_ec_changed};

		Attached_rom_dataspace _acpi_lid_report{_env, "acpi_lid"};
		Signal_handler<Main>   _acpi_lid_handler{_env.ep(), *this, &Main::_acpi_lid_changed};

		void _config_changed() {
		}

		void _acpi_ac_changed() {
			_acpi_ac_report.update();
			parse_acpi_ac_report(_acpi_ac_report.xml());
		}

		void _acpi_battery_changed() {
			_acpi_battery_report.update();
			parse_acpi_battery_report(_acpi_battery_report.xml());
		}

		void _acpi_ec_changed() {
			_acpi_ec_report.update();
			parse_acpi_ec_report(_acpi_ec_report.xml());
		}

		void _acpi_lid_changed() {
			_acpi_lid_report.update();
			parse_acpi_lid_report(_acpi_lid_report.xml());
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
