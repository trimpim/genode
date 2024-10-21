/*
 * \brief  Component that resets an init config if heartbeats were skipped
 * \author Sid Hussmann
 * \date   2019-12-10
 *
 * Component that monitors the state of an init and tries to recover from errors.
 */


/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/log.h>
#include <os/reporter.h>

/* local includes */
#include "free_ram_handler.h"
#include "detector.h"
#include "report_root.h"
#include "restart_reporter.h"
#include "slice_list.h"


namespace Heartbeat_monitor {

	using namespace Genode;

	class Main;
}


class Heartbeat_monitor::Main
{
	private:

		Env                      &_env;
		Heap                      _heap                  { _env.ram(), _env.rm() };
		Attached_rom_dataspace    _config                { _env, "config" };
		Attached_rom_dataspace    _init_config           { _env, "init.config" };
		Slice_list                _slices                { _heap };
		Restart_reporter          _state_reporter        { _env, _slices };
		Dynamic_resource_slice    _dynamic_slice         { };
		Sibling_init_report_label _sibling_init_report   { };
		Detector                  _detector              { _env, _init_config, _slices, _state_reporter, _dynamic_slice, _sibling_init_report };
		Free_ram_handler          _free_ram_handler      { _env };
		Report_root               _init_reports          { _env, _heap, _detector, _free_ram_handler };

	public:

		Main(Env& env) :
			_env{env}
		{
			_env.parent().announce(_env.ep().manage(_init_reports));

			_config.update();
			if (_config.valid()) {
				if (_config.xml().has_attribute("remaining_resources_for")) {
					Session_label label = _config.xml().attribute_value("remaining_resources_for", String<160> { });
					_dynamic_slice.construct(label.last_element());
					_sibling_init_report.construct(label.prefix(), " -> state");
				}
			}
		}

		virtual ~Main() = default;
};


void Component::construct(Genode::Env &env) {
	static Heartbeat_monitor::Main main(env);
}
