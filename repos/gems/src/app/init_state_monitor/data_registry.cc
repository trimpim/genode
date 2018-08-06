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
#include <os/reporter.h>

/* local includes */
#include "data_registry.h"


void Init_state_monitor::Data_registry::_for_each_child(Xml_generator& xml,
                                                        const bool show_caps)
{
	for_each([&] (Entry* e) {

		xml.node("float", [&] () {
			_insert_name_attribute(xml);
			_insert_vbox_left_attribute(xml);

			xml.node("hbox", [&] () {
				_insert_name_attribute(xml);

				_child_ram(xml, e);

				if (show_caps) {

					_child_caps(xml, e);
				}

				xml.node("float", [&] () {
					_insert_name_attribute(xml);
					_insert_vbox_left_attribute(xml);
					xml.node("label", [&] () {
						xml.attribute("text", e->label);
						xml.attribute("color", "#ffffff");
						xml.attribute("align", "left");
					});
				});
			});
		});
	});
}


void Init_state_monitor::Data_registry::_child_ram(Xml_generator& xml,
                                                   const Entry* entry)
{
	xml.node("float", [&] () {
		_insert_name_attribute(xml);
		_insert_vbox_left_attribute(xml);
		xml.node("vbox", [&] () {
			_insert_name_attribute(xml);
			xml.node("float", [&] () {
				_insert_name_attribute(xml);
				_insert_vbox_left_attribute(xml);
				xml.node("bar", [&] () {
					size_t percent = entry->average_ram() * 100 / entry->_ram_quota;
					size_t rest    = entry->average_ram() * 10000 / entry->_ram_quota -
					                 (percent * 100);
					xml.attribute("color", "#00ff000");
					xml.attribute("textcolor", "#f000f0");
					xml.attribute("percent", percent);
					xml.attribute("width", 200);
					xml.attribute("height", 20);
					xml.attribute("text", String<50>(string(percent, rest),
                                           " / ", entry->_ram_quota));
				});
			});
			/*
			xml.node("float", [&] () {
				_insert_name_attribute(xml);
				_insert_vbox_left_attribute(xml);
				xml.node("bar", [&] () {
					size_t percent = entry->last_ram() * 100 / entry->_ram_quota;
					size_t rest    = entry->last_ram() * 10000 / entry->_ram_quota -
					                 (percent * 100);
					xml.attribute("color", "#00bb000");
					xml.attribute("textcolor", "#f000f0");
					xml.attribute("percent", percent);
					xml.attribute("width", 200);
					xml.attribute("height", 20);
					xml.attribute("text", string(percent, rest));
				});
			});
			*/
		});
	});
}


void Init_state_monitor::Data_registry::_child_caps(Xml_generator& xml,
                                                    const Entry* entry)
{
	xml.node("float", [&] () {
		_insert_name_attribute(xml);
		_insert_vbox_left_attribute(xml);
		xml.node("vbox", [&] () {
			_insert_name_attribute(xml);
			xml.node("float", [&] () {
				_insert_name_attribute(xml);
				_insert_vbox_left_attribute(xml);
				xml.node("bar", [&] () {
					size_t percent = entry->average_caps() * 100 / entry->_cap_quota;
					size_t rest    = entry->average_caps() * 10000 / entry->_cap_quota -
					                 (percent * 100);
					xml.attribute("color", "#00ffff0");
					xml.attribute("textcolor", "#f000f0");
					xml.attribute("percent", percent);
					xml.attribute("width", 200);
					xml.attribute("height", 20);
					xml.attribute("text", String<50>(string(percent, rest),
                                           " / ", entry->_cap_quota));
				});
			});
			/*
			xml.node("float", [&] () {
				_insert_name_attribute(xml);
				_insert_vbox_left_attribute(xml);
				xml.node("bar", [&] () {
					size_t percent = entry->last_caps() * 100 / entry->_cap_quota;
					size_t rest    = entry->last_caps() * 10000 / entry->_cap_quota -
					                 (percent * 100);
					xml.attribute("color", "#00bbbb0");
					xml.attribute("textcolor", "#f000f0");
					xml.attribute("percent", percent);
					xml.attribute("width", 200);
					xml.attribute("height", 20);
					xml.attribute("text", string(percent, rest));
				});
			});
			*/
		});
	});
}


void Init_state_monitor::Data_registry::_init_ram(Xml_generator& xml)
{
	xml.node("float", [&] () {
		_insert_name_attribute(xml);
		_insert_vbox_left_attribute(xml);
		xml.node("bar", [&] () {
			size_t percent = _ram * 100 / _ram_quota;
			size_t rest    = _ram * 10000 / _ram_quota -
			                 (percent * 100);
			xml.attribute("color", "#44bb000");
			xml.attribute("textcolor", "#f000f0");
			xml.attribute("percent", percent);
			xml.attribute("width", 200);
			xml.attribute("height", 28);
			xml.attribute("text", String<50>(string(percent, rest),
                                       " / ", _ram_quota));
		});
	});
}


void Init_state_monitor::Data_registry::_init_caps(Xml_generator& xml)
{
	xml.node("float", [&] () {
		_insert_name_attribute(xml);
		_insert_vbox_strecth_attribute(xml);
		xml.node("bar", [&] () {
			size_t percent = _caps * 100 / _cap_quota;
			size_t rest    = _caps * 10000 / _cap_quota -
			                 (percent * 100);
			xml.attribute("color", "#44bbbb0");
			xml.attribute("textcolor", "#f000f0");
			xml.attribute("percent", percent);
			xml.attribute("width", 200);
			xml.attribute("height", 28);
			xml.attribute("text", String<50>(string(percent, rest),
                                       " / ", _cap_quota));
		});
	});
}


void Init_state_monitor::Data_registry::generate(Xml_generator& xml, const bool show_caps)
{
	xml.node("vbox", [&] () {
		_insert_name_attribute(xml);

		xml.node("hbox", [&] () {
			_insert_name_attribute(xml);
			xml.node("float", [&] () {
				_insert_name_attribute(xml);
				_insert_vbox_left_attribute(xml);
				xml.node("bar", [&] () {
					xml.attribute("width", 200);
					xml.attribute("height", 28);
					xml.attribute("percent", 100);
					xml.attribute("text", "ram");
					xml.attribute("color", "#333333");
					xml.attribute("textcolor", "#ffffff");
					xml.attribute("align", "center");
				});
			});

			if (show_caps) {
				xml.node("float", [&] () {
					_insert_name_attribute(xml);
					_insert_vbox_left_attribute(xml);
					xml.node("bar", [&] () {
						xml.attribute("width", 200);
						xml.attribute("height", 28);
						xml.attribute("percent", 100);
						xml.attribute("text", "caps");
						xml.attribute("color", "#444444");
						xml.attribute("textcolor", "#ffffff");
						xml.attribute("align", "center");
					});
				});
			}

			xml.node("float", [&] () {
				_insert_name_attribute(xml);
				_insert_vbox_left_attribute(xml);
				xml.node("bar", [&] () {
					xml.attribute("width", 200);
					xml.attribute("height", 28);
					xml.attribute("percent", 100);
					xml.attribute("text", "label");
					xml.attribute("color", "#333333");
					xml.attribute("textcolor", "#ffffff");
					xml.attribute("align", "left");
				});
			});
		});

		xml.node("hbox", [&] () {
			_insert_name_attribute(xml);

			_init_ram(xml);

			if (show_caps) {
				_init_caps(xml);
			}

			xml.node("float", [&] () {
				_insert_name_attribute(xml);
				_insert_vbox_left_attribute(xml);
				xml.node("label", [&] () {
					xml.attribute("text", "init");
					xml.attribute("color", "#ffffff");
					xml.attribute("align", "left");
				});
			});

		});

		_for_each_child(xml, show_caps);
	});
}
