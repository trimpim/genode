/*
 * \brief  report amount of free ram and caps
 * \author Pirmin Duss
 * \date   2020-05-13
 *
 */


/* local includes */
#include "free_ram_handler.h"


void Heartbeat_monitor::Free_ram_handler::handle_state(Xml_node const &state, Session_label const &label)
{
	/*
	 * the label has one of the forms
	 *   - "stage1 -> state"
	 *   - "stage1 -> <SLICE_NAME> -> state"
	 * if the state comes from a SLICE we don't write the state report
	 */
	Start_name const prefix { label.prefix() };
	Start_name const slice  { label.prefix().last_element() };

	if ((prefix != slice) || (!state.has_sub_node("ram") || !state.has_sub_node("caps"))) {
		return;
	}

	_reporter.generate([&state] (Xml_generator& xml) {
		enum { BYTES_PER_MIB = 1024 * 1024, };
		size_t const avail_ram_mb { state.sub_node("ram").attribute_value("avail", Genode::Number_of_bytes { }) / BYTES_PER_MIB };
		size_t const avail_caps   { state.sub_node("caps").attribute_value("avail", 0UL) };
		xml.node("ram", [&avail_ram_mb, &xml] () {
			xml.attribute("available_mb", avail_ram_mb);
		});
		xml.node("caps", [&avail_caps, &xml] () {
			xml.attribute("available", avail_caps);
		});
	});
}
