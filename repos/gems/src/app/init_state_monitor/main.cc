/*
 * \brief  Application to show RAM and CAP usage of components.
 * \author Pirmin Duss
 * \date   2018-07-24
 */

/*
 * Copyright (C) 2015-2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <os/reporter.h>

/* local includes */
#include "data_registry.h"

namespace Init_state_monitor {

	class Main;

	using namespace Genode;
}


class Init_state_monitor::Main
{
	private:

		Env&                   _env;
		Heap                   _heap          { _env.ram(), _env.rm() };
		Data_registry          _data          { _heap };

		bool                   _monitor_caps  { false };

		Attached_rom_dataspace _config        { _env, "config" };

		Attached_rom_dataspace _state         { _env, "init_state" };
		Signal_handler<Main>   _state_handler { _env.ep(), *this,
		                                        &Main::_handle_state };

		Expanding_reporter     _dialog        { _env, "dialog", "dialog" };

		void _handle_state();

	public:

		Main(Genode::Env& env) : _env{env} {
			_state.sigh(_state_handler);

			_config.update();
			_monitor_caps = _config.xml().attribute_value("monitor_caps", false);
		}

};


void Init_state_monitor::Main::_handle_state()
{
	_state.update();

	Xml_node state = _state.xml();

	if (!state.has_sub_node("ram")) {
		return;
	}

	Number_of_bytes init_ram_quota = state.sub_node("ram").attribute_value("quota", Number_of_bytes{});
	Number_of_bytes init_ram_used  = state.sub_node("ram").attribute_value("used",  Number_of_bytes{});

	size_t init_caps_quota = state.sub_node("caps").attribute_value("quota", 0UL);
	size_t init_caps_used  = state.sub_node("caps").attribute_value("used",  0UL);

	_data.update_init(init_ram_quota, init_caps_quota, init_ram_used, init_caps_used);

	state.for_each_sub_node("child", [&] (Xml_node node) {
		Label name = node.attribute_value("name", Label{});
		
		Number_of_bytes ram_quota = node.sub_node("ram").attribute_value("quota", Number_of_bytes{});
		Number_of_bytes ram_used  = node.sub_node("ram").attribute_value("used",  Number_of_bytes{});

		size_t cap_quota { 0 };
		size_t caps_used { 0 };

		if (_monitor_caps) {
			cap_quota = node.sub_node("caps").attribute_value("quota", 0UL);
			caps_used = node.sub_node("caps").attribute_value("used",  0UL);
		}

		_data.update(name, ram_quota, cap_quota, ram_used, caps_used);
	});

	_dialog.generate([&] (Xml_generator& xml) { _data.generate(xml, _monitor_caps); });
}


void Component::construct(Genode::Env &env)
{
	static Init_state_monitor::Main main(env);
}
