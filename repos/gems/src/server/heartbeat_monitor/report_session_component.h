/*
 * \brief  session component to anounce a report session
 * \author Sid Hussmann
 * \date   2019-12-10
 *
 */

#pragma once


/* Genode includes */
#include <base/rpc_server.h>
#include <base/attached_ram_dataspace.h>
#include <report_session/report_session.h>
#include <util/reconstructible.h>

/* local includes */
#include "detector.h"
#include "free_ram_handler.h"
#include "init_state_reporter.h"


namespace Heartbeat_monitor {

	using namespace Genode;

	enum { BUFFER_SIZE = 4096 };

	class Report_session_component;
}


class Heartbeat_monitor::Report_session_component : public Rpc_object<Report::Session>
{
	private:

		Env&                                _env;
		Session_label                       _label;
		Detector                           &_detector;
		Free_ram_handler                   &_free_ram_handler;
		Attached_ram_dataspace              _io_buffer;
		Constructible<Init_state_reporter>  _init_state_reporter { };


	public:

		Report_session_component(Env& env,
		                         Session_label const &label,
		                         Detector            &detector,
		                         Free_ram_handler    &free_ram_handler,
		                         const size_t         buffer_size) :
			_env               { env },
			_label             { label },
			_detector          { detector },
			_free_ram_handler  { free_ram_handler },
			_io_buffer         { _env.ram(), _env.rm(), buffer_size }
		{
			/*
			 * the label has one of the forms
			 *   - "stage1 -> state"
			 *   - "stage1 -> <SLICE_NAME> -> state"
			 * we only re-report the former
			 */
			Session_label const prefix { label.prefix() };
			if (prefix == prefix.last_element() && label.last_element() == "state") {
				_init_state_reporter.construct(env);
			}
		}

		/********************************
		 ** Report session interface **
		 ********************************/

		Dataspace_capability dataspace() override { return _io_buffer.cap(); }

		void response_sigh(Genode::Signal_context_capability) override { }

		size_t obtain_response() override { return 0; }

		void submit(size_t length) override
		{
			size_t const size = Genode::min(length, _io_buffer.size());
			Xml_node const state { _io_buffer.local_addr<const char>(), size };
			_detector.handle_state(state, _label);
			_free_ram_handler.handle_state(state, _label);
			if (_init_state_reporter.constructed()) {
				_init_state_reporter->handle_state(state);
			}
		}
};
