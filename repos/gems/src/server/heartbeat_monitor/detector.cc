/*
 * \brief  implementation of the detector
 *         The detector can detect different error conditions reported by an
 *         init report.
 * \author Pirmin Duss
 * \date   2020-03-20
 *
 */


#include "detector.h"


namespace Heartbeat_monitor
{
	void print_heartbeat_error(Start_name const &name, Start_name const &slice, bool is_slice, long skipped_heartbeats)
	{
		if (is_slice) {
			warning("unresponsive component '", name, "' in SLICE with label '", slice, "', skipped heartbeats: ", skipped_heartbeats);
		} else {
			warning("unresponsive component '", name, "', skipped heartbeats: ", skipped_heartbeats);
		}
	}

	void print_resource_request(Start_name const &name, Start_name const &slice, bool is_slice, Xml_node const &resource)
	{
		if (is_slice) {
			warning("component '", name, "' in SLICE with label '", slice, "' requested resource of type '", resource.type(), "'");
		} else {
			warning("component '", name, "' requested resource of type '", resource.type(), "'");
		}
	}

	void print_pd_fault(Start_name const &name, Start_name const &slice, bool is_slice)
	{
		if (is_slice) {
			warning("component '", name, "' in SLICE with label '", slice, "', PD fault detected");
		} else {
			warning("component '", name, "', PD fault detected");
		}
	}

	void print_cpu_fault(Start_name const &name, Start_name const &slice, bool is_slice)
	{
		if (is_slice) {
			warning("component '", name, "' in SLICE with label '", slice, "', CPU fault detected");
		} else {
			warning("component '", name, "', CPU fault detected");
		}
	}
}


Genode::Number_of_bytes Heartbeat_monitor::Detector::_ram_of_start_node(const Xml_node& start_node) const
{
	Number_of_bytes ram { };
	start_node.for_each_sub_node("resource", [&ram] (Xml_node const &resource) {
		if (resource.attribute_value("name", String<8>()) == "RAM") {
			ram = resource.attribute_value("quantum", Number_of_bytes { });
			return;
		}
	});
	return ram;
}


void Heartbeat_monitor::Detector::_get_total_resources(Resources& total, Xml_node const &state)
{
	total.number_of_children = 0;
	total.caps_quota = state.sub_node("caps").attribute_value("quota", 0UL);
	total.caps_avail = state.sub_node("caps").attribute_value("avail", 0UL);
	total.caps_assigned = 0;
	total.ram_quota = state.sub_node("ram").attribute_value("quota", Number_of_bytes { });
	total.ram_avail = state.sub_node("ram").attribute_value("avail", Number_of_bytes { });
	total.ram_assigned = 0;

	state.for_each_sub_node("child", [&total] (const Xml_node &child) {
		total.number_of_children++;
		child.with_optional_sub_node("caps", [&total] (const Xml_node &node) {
			total.caps_quota = total.caps_quota + node.attribute_value("quota", 0UL);
			total.caps_avail = total.caps_avail + node.attribute_value("avail", 0UL);
			total.caps_assigned = total.caps_assigned + node.attribute_value("assigned", 0UL);
		});
		child.with_optional_sub_node("ram", [&total] (const Xml_node &node) {
			total.ram_quota = total.ram_quota + node.attribute_value("quota", Number_of_bytes { });
			total.ram_avail = total.ram_avail + node.attribute_value("avail", Number_of_bytes { });
			total.ram_assigned = total.ram_assigned + node.attribute_value("assigned", Number_of_bytes { });
		});
	});
}


bool Heartbeat_monitor::Detector::_assign_remaining_resources(Xml_node const &state, Slice *dynamic_resource_slice)
{
	bool update_init_config = false;

	if (_recalc_remaining_caps || _recalc_remaining_ram) {
		Resources total { };
		_get_total_resources(total, state);
		log("total resources of '", _sibling_init_report_label->prefix(), "': ", total);

		/* the total configured resources without the slice for dynamic resource assignment */
		size_t          total_configured_caps = 0;
		Number_of_bytes total_configured_ram = 0;

		/* we need this from the incomming _init_config, as we don't want to create a weird feedback loop */
		_init_config.xml().for_each_sub_node("start", [this,
		                                               &dynamic_resource_slice,
		                                               &total_configured_caps,
		                                               &total_configured_ram] (Xml_node const &node) {
			/* don't addup the configured resource of the component that gets a dynamic one */
			if (node.attribute_value("name", Start_name { }) != dynamic_resource_slice->name()) {
				auto const caps { node.attribute_value("caps", 0ul) };
				total_configured_caps += caps;
				total_configured_ram = total_configured_ram + _ram_of_start_node(node); /* there is no += in Number_of_bytes */
			}
		});

		/* we want to only calculate the dynamic resources when necessary and not on every state report */
		if (dynamic_resource_slice->dynamic_caps() == 0 || _recalc_remaining_caps) {
			constexpr size_t CAPS_MARGIN = 400;
			size_t const total_caps = (total.caps_avail < CAPS_MARGIN) ? 0 : total.caps_avail - CAPS_MARGIN;

			if (total_caps > total_configured_caps) {
				auto const soft_limit_caps = total_caps - total_configured_caps;

				dynamic_resource_slice->dynamic_caps(soft_limit_caps);
				_recalc_remaining_caps = false;
				update_init_config = true;
			} else {
				error("unable to upgrade dynamic caps of '", dynamic_resource_slice->name(),
				      "': total caps (", total_caps, ") less than total configured caps (", total_configured_caps, ")");
			}
		}
		if (dynamic_resource_slice->dynamic_ram() == 0 || _recalc_remaining_ram) {
			constexpr size_t RAM_MARGIN = 20*1024*1024;
			Number_of_bytes const total_ram = total.ram_avail < RAM_MARGIN ? 0 : total.ram_avail - RAM_MARGIN;

			if (total_ram > total_configured_ram) {
				Number_of_bytes const soft_limit_ram = total_ram - total_configured_ram;

				dynamic_resource_slice->dynamic_ram(soft_limit_ram);
				_recalc_remaining_ram = false;
				update_init_config = true;
			} else {
				error("unable to upgrade dynamic ram of '", dynamic_resource_slice->name(),
				      "': total ram (", total_ram, ") less than total configured ram (", total_configured_ram, ")");
			}
		}
	}

	return update_init_config;
}


void Heartbeat_monitor::Detector::_update_init_config()
{
	_init_config.update();

	_slices.remove_outdated(_init_config.xml());

	bool const verbose_init_config { _init_config.xml().attribute_value("verbose", false) };
	_init_config_reporter.generate([this, verbose_init_config] (Xml_generator &xml) {

		if (verbose_init_config) {
			xml.attribute("verbose", true);
		}

		_init_config.xml().for_each_sub_node([this, &xml] (Xml_node const &node) {

			if (node.type() != "start") {
				node.with_raw_node([&xml] (char const *addr, size_t size) {
					xml.append(addr, size);
				});
			} else {

				auto const       name        { node.attribute_value("name", Start_name { }) };
				auto const       version     { node.attribute_value("version", 0ul) };
				auto             caps        { node.attribute_value("caps", 0ul) };
				auto            *slice_start { _slices.add(node) };
				Number_of_bytes  ram         { _ram_of_start_node(node) };

				if (slice_start->configured_caps() != caps) {
					/* in this case the config has changed and we need to recalc dynamic resource */
					_recalc_remaining_caps = true;
					slice_start->configured_caps(caps);
				}

				if (slice_start->configured_ram() != ram) {
					/* in this case the config has changed and we need to recalc dynamic resource */
					_recalc_remaining_ram = true;
					slice_start->configured_ram(ram);
				}

				if (slice_start->dynamic_caps() != 0) {
					log("upgrade dynamic caps of '", name, "' from ", caps, " to ", slice_start->dynamic_caps());
					caps = slice_start->dynamic_caps();
				}

				if (slice_start->dynamic_ram() != 0) {
					log("upgrade dynamic ram of '", name, "' from ", ram, " to ", slice_start->dynamic_ram());
					ram = slice_start->dynamic_ram();
				}

				xml.node("start", [&caps, &name, &node, &xml, &slice_start, &version, &ram] ( ) {

					xml.attribute("name", name);

					slice_start->update_deploy_version(version);
					slice_start->update_internal_version();

					xml.attribute("version", slice_start->combined_version());

					if (caps > 0)
						xml.attribute("caps", caps);

					if (slice_start->dynamic_ram() == 0) {
						node.with_raw_content([&xml] (char const *addr, size_t size) {
							xml.append(addr, size);
						});
					} else {
						node.for_each_sub_node([&xml, &ram] (Xml_node const &sub_node) {
							bool copy_whole_node = true;
							if (sub_node.has_type("resource")) {
								if (sub_node.attribute_value("name", String<8>()) == "RAM") {
									copy_whole_node = false;
									xml.node("resource", [&xml, &ram] () {
										xml.attribute("name", "RAM");
										xml.attribute("quantum", ram);
									});
								}
							}
							if (copy_whole_node) {
								sub_node.with_raw_node([&] (char const *addr, size_t size) {
									xml.append(addr, size);
								});
							}
						});
					}
				});
			}
		});
	});
	/*
	 * To calculate total assigned resources we need to do this outside the `_init_config_reporter.generate()`
	 * lambda, because when it exceeds its buffer, it gets called again which would result in wrong values.
	 * The other approach (iterating over the resulting xml would be too expensive.
	 */
	size_t          total_configured_caps = 0;
	Number_of_bytes total_configured_ram = 0;
	_slices.for_each([&total_configured_caps, &total_configured_ram] (Slice const &slice) {
		total_configured_caps += (slice.dynamic_caps() != 0) ?
		                          slice.dynamic_caps() :
		                          slice.configured_caps();
		total_configured_ram = total_configured_ram + ((slice.dynamic_ram() != 0) ?
		                                                slice.dynamic_ram() :
		                                                slice.configured_ram());
	});
	log("total assigned resources: caps=", total_configured_caps, ", ram=", total_configured_ram);
}


void Heartbeat_monitor::Detector::handle_state(Xml_node const &state, Session_label const &label)
{
	if (state.num_sub_nodes() <= 1) return;

	/*
	 * the session label has one of the forms:
	 *   - case1: "monitored_slice_runtime -> state"
	 *   - case2: "monitored_slice_runtime -> <SLICE_NAME> -> state"
	 *   - case3: "monitored_slice_runtime -> <SLICE_NAME> -> <SUB_INIT> (-> <SUB_INIT>*) -> state"
	 *
	 * The monitored_slice_runtime is the one that can be re-written by the heartbeat_monitor
	 *
	 * if the child that has a skipped heartbeat belongs to a SLICE
	 * we restart the whole slice, otherwise just the child
	 */

	Session_label prefix {label.prefix()};
	Session_label slice  {prefix.last_element()};

	for(; prefix.prefix() != ""; prefix = prefix.prefix())
		slice = prefix.last_element();
	Session_label monitored_slice_runtime = prefix;

	bool const is_slice { slice == monitored_slice_runtime };

	auto fail_name = [&is_slice, &slice] (Start_name const &name) -> Start_name {
		return is_slice ? name : Start_name(slice);
	};

	auto fail_label = [&is_slice, &slice] (Start_name const& name) -> Session_label {
		return is_slice ? Session_label { name } : Session_label { slice, " -> ", name };
	};

	auto fail_from_resource = [] (Xml_node const &ressource) -> Failure {
		return ressource.type() == "ram" ? Failure::Ram_request : Failure::Caps_request;
	};

	state.for_each_sub_node("child", [this, &is_slice, &slice, &fail_label,
	                                  &fail_name, &fail_from_resource] (const Xml_node &child) {

		Start_name const name { child.attribute_value("name", Start_name{ }) };

		/* check for skipped heartbeats */
		if (child.has_attribute("skipped_heartbeats")) {
			auto const skipped_heartbeats = child.attribute_value("skipped_heartbeats", 0l);

			if (skipped_heartbeats) {

				print_heartbeat_error(name, slice, is_slice, skipped_heartbeats);

				if (skipped_heartbeats >= 3 )
					_slices.mark_restart(fail_name(name), fail_label(name), Failure::Heartbeat);
			}
		}

		/*
		 * check for resource requests this is enabled by setting
		 * `child_ram` and `child_caps` attributes in the `report`
		 * node of the init config.
		 */
		child.for_each_sub_node([this, &is_slice, &name, &slice,
		                         &fail_label, &fail_name, &fail_from_resource] (Xml_node const &ressource) {

			if (ressource.has_attribute("requested")) {

				print_resource_request(name, slice, is_slice, ressource);

				_slices.mark_restart(fail_name(name), fail_label(name), fail_from_resource(ressource));
			}
		});

		/* check for fault in the PD */
		if (child.attribute_value("pd_fault", false)) {
			print_pd_fault(name, slice, is_slice);
			_slices.mark_restart(fail_name(name), fail_label(name), Failure::Seg_fault);
		}

		/* check for CPU faults */
		if (child.attribute_value("cpu_fault", false)) {
			print_cpu_fault(name, slice, is_slice);
			_slices.mark_restart(fail_name(name), fail_label(name), Failure::Cpu_fault);
		}
	});

	/* update the target config if needed */
	if (_slices.restart_required()) {
		_update_init_config();
	} else {
		/* upgrade resources for the sub-system that has dynamic resources */
		if (_sibling_init_report_label.constructed()) {
			bool const sibling { label == *_sibling_init_report_label };

			if (sibling) {
				if (!_slices.empty() && _dynamic_resource_slice.constructed()) {
					auto* dynamic_resource_slice = _slices.find(_dynamic_resource_slice);
					if (dynamic_resource_slice) {
						if (_assign_remaining_resources(state, dynamic_resource_slice)) {
							_update_init_config();
						}
					} else {
						warning("SLICE '", _dynamic_resource_slice, "' for dynamic resource assignment not found");
						_update_init_config();
					}
				}
			}
		}
	}

	/* update the state report */
	_state_reporter.report_state();
}
