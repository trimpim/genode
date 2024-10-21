/*
 * \brief  report the state of the companion/sibling init state
 * \author Sid Hussmann
 * \date   2023-04-11
 *
 */

#pragma once

/* Genode includes */
#include <base/heap.h>
#include <base/session_label.h>
#include <os/reporter.h>


namespace Heartbeat_monitor {

	using namespace Genode;

	class Init_state_reporter;
}


class Heartbeat_monitor::Init_state_reporter
{
	private:

		Expanding_reporter  _reporter;

	public:

		Init_state_reporter(Env &env) :
			_reporter { env, "state", "init.state" }
		{ }

		virtual ~Init_state_reporter() = default;

		void handle_state(Xml_node const &state)
		{
			_reporter.generate(state);
		}
};
