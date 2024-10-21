/*
 * \brief  definition of restart information reporter
 * \author Pirmin Duss
 * \date   2020-04-22
 *
 */

#pragma once

/* Genode includes */
#include <base/registry.h>
#include <os/reporter.h>
#include <util/string.h>
#include <util/xml_node.h>

/* local includes */
#include "slice_list.h"


namespace Heartbeat_monitor {

	using namespace Genode;

	struct State_reporter_interface
	{
		virtual ~State_reporter_interface() = default;

		virtual void report_state() = 0;
	};

	class Restart_reporter : public State_reporter_interface
	{
		private:

			Env                &_env;
			Slice_list         &_packages;
			Expanding_reporter  _restart_reporter { _env, "restart_info", "restart_info" };

		public:

			Restart_reporter(Env &env, Slice_list &packages) :
				_env      { env },
				_packages { packages }
			{ }

			void report_state() override;
	};
}
