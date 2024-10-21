/*
 * \brief  definition of the detector
 *         The detector can detect different error conditions reported by an
 *         init report.
 * \author Pirmin Duss
 * \date   2020-03-20
 *
 */

#pragma once

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <base/registry.h>
#include <os/reporter.h>
#include <util/list.h>
#include <util/string.h>

/* local includes */
#include "slice_list.h"
#include "restart_reporter.h"


namespace Heartbeat_monitor {

	using namespace Genode;

	using Dynamic_resource_slice = Constructible<Start_name>;
	using Sibling_init_report_label = Constructible<Session_label>;

	class Detector;
}


class Heartbeat_monitor::Detector
{
	private:

		struct Resources
		{
			size_t number_of_children { };
			size_t caps_quota { };
			size_t caps_avail { };
			size_t caps_assigned { };
			Number_of_bytes ram_quota { };
			Number_of_bytes ram_avail { };
			Number_of_bytes ram_assigned { };

			void print(Output &out) const
			{
				Genode::print(out, "number_of_children=", number_of_children);
				Genode::print(out, " caps_quota=", caps_quota, ", caps_avail=", caps_avail, ", caps_assigned=", caps_assigned);
				Genode::print(out, " ram_quota=", ram_quota, ", ram_avail=", ram_avail, ", ram_assigned=", ram_assigned);
			}
		};

		constexpr static size_t INITIAL_REPORT_BUFFER_SIZE = 16384;

		Env                       &_env;
		Attached_rom_dataspace    &_init_config;
		Expanding_reporter         _init_config_reporter    { _env, "config", "init.config", { INITIAL_REPORT_BUFFER_SIZE } };
		Signal_handler<Detector>   _init_config_handler     { _env.ep(), *this, &Detector::_update_init_config };
		Slice_list                &_slices;
		State_reporter_interface  &_state_reporter;

		Dynamic_resource_slice    &_dynamic_resource_slice;
		Sibling_init_report_label &_sibling_init_report_label;

		bool                       _recalc_remaining_caps   { false };
		bool                       _recalc_remaining_ram    { false };

		void            _update_init_config();
		Number_of_bytes _ram_of_start_node(Xml_node const &start_node) const;
		void            _get_total_resources(Resources &total, Xml_node const &state);
		bool            _assign_remaining_resources(Xml_node const &state, Slice *dynamic_resource_slice);

	public:

		Detector(Env &env,
		         Attached_rom_dataspace &init_config,
		         Slice_list &slices,
		         State_reporter_interface &state_reporter,
		         Dynamic_resource_slice &dynamic_resource_slice,
		         Sibling_init_report_label &sibling_init_report_label)
		:
			_env                       { env },
			_init_config               { init_config },
			_slices                    { slices },
			_state_reporter            { state_reporter },
			_dynamic_resource_slice    { dynamic_resource_slice },
			_sibling_init_report_label { sibling_init_report_label}
		{
			_init_config.sigh(_init_config_handler);
			_update_init_config();
		}

		virtual ~Detector() = default;

		void handle_state(Xml_node const &state, Session_label const &label);
};
