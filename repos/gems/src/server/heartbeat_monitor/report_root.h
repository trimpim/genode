/*
 * \brief  root component to anounce a report session
 * \author Sid Hussmann
 * \date   2019-12-10
 *
 */

#pragma once


/* Genode includes */
#undef PAGE_SIZE
#include <base/allocator.h>
#include <root/component.h>

/* local includes */
#include "detector.h"
#include "free_ram_handler.h"
#include "report_session_component.h"


namespace Heartbeat_monitor {

	using namespace Genode;

	class Report_root;
}


class Heartbeat_monitor::Report_root final: public Root_component<Report_session_component>
{
	private:

		Env                &_env;
		Allocator          &_alloc;
		Detector           &_detector;
		Free_ram_handler   &_free_ram_handler;

	protected:

		Report_session_component* _create_session(char const *args) override
		{
			Session_label const label       { label_from_args(args) };
			size_t        const buffer_size { Arg_string::find_arg(args, "buffer_size").aligned_size() };
			Genode::log("init state report session created with label='", label, "' buffer size=", buffer_size);
			return new (_alloc) Report_session_component { _env,
			                                               label,
			                                               _detector,
			                                               _free_ram_handler,
			                                               buffer_size };
		}

		void _destroy_session(Report_session_component *session) override
		{
			Genode::destroy(_alloc, session);
		}

	public:

		Report_root(Env              &env,
		            Allocator        &alloc,
		            Detector         &detector,
		            Free_ram_handler &free_ram_handler) :
			Root_component     { &env.ep().rpc_ep(), &alloc },
			_env               { env },
			_alloc             { alloc },
			_detector          { detector },
			_free_ram_handler  { free_ram_handler }
		{ }
};
