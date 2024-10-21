/*
 * \brief  report amount of free ram and caps
 * \author Pirmin Duss
 * \date   2020-05-13
 *
 */

#pragma once

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <os/reporter.h>
#include <util/string.h>

/* local includes */
#include "slice_list.h"


namespace Heartbeat_monitor {

	using namespace Genode;

	class Free_ram_handler;
}


class Heartbeat_monitor::Free_ram_handler final
{
	private:

		Env                &_env;
		Expanding_reporter  _reporter { _env, "free_resources", "free_resources" };

	public:

		Free_ram_handler(Env &env) :
			_env { env }
		{ }

		virtual ~Free_ram_handler() = default;

		void handle_state(Xml_node const &state, Session_label const &label);
};
