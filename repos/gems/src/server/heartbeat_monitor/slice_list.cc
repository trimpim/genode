/*
 * \brief  list to manage packets to start
 * \author Pirmin Duss
 * \date   2020-04-22
 *
 */


/* local includes */
#include "slice_list.h"


Heartbeat_monitor::Slice_list::~Slice_list()
{
	while (Slice* e = _slices.first()) {
		_slices.remove(e);
		destroy(_alloc, e);
	}
}


Heartbeat_monitor::Slice *Heartbeat_monitor::Slice_list::add(Xml_node const &start_node)
{
	auto  const  name  { start_node.attribute_value("name", Start_name { }) };
	Slice       *slice { find(name) };
	if (!slice) {
		slice = new (_alloc) Slice { _alloc, name };
		_slices.insert(slice);
	}
	return slice;
}


Heartbeat_monitor::Slice *Heartbeat_monitor::Slice_list::find(Session_label const &name)
{
	for (auto *e = _slices.first(); nullptr != e; e = e->next()) {
		if (name == e->name()) {
			return e;
		}
	}

	return nullptr;
}


bool Heartbeat_monitor::Slice_list::restart_required() const
{
	for (auto *e = _slices.first(); nullptr != e; e = e -> next()) {
		if (e->restart()) {
			return true;
		}
	}

	return false;
}


void Heartbeat_monitor::Slice_list::remove_outdated(Xml_node const &config)
{
	/* check for nodes that are no longer in config */
	for (auto *e = _slices.first(); nullptr != e; ) {

		bool found { false };
		config.for_each_sub_node("start", [&e, &found] (Xml_node const &start) {

			if (start.attribute_value("name", Start_name { }) == e->name()) {
				found = true;
			}
		});

		auto* next { e->next() };
		if (!found) {
			_slices.remove(e);
			destroy(_alloc, e);
		}
		e = next;
	}
}


void Heartbeat_monitor::Slice_list::mark_restart(Start_name const &name, Session_label const &label, Failure kind)
{
	for (auto *e = _slices.first(); nullptr != e; e = e->next()) {

		if (name == e->name()) {
			e->mark_restart(label, kind);
			return;
		}

		/* determine containing slice */
		if (e->name().length() <= label.length()) {

			if (strcmp(label.string(), e->name().string(), label.length()) == 0) {
				e->mark_restart(label, kind);
				return;
			}
		}
	}

	warning("restart requested for a component '", label, "' that isn't running.");
}
