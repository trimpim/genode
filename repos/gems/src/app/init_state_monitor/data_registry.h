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
#include "ring_buffer.h"


namespace Init_state_monitor {

	class Data_registry;
	class Main;

	using namespace Genode;

	typedef String<256> Label;
}


class Init_state_monitor::Data_registry
{
	private:

		struct Entry : Genode::List<Entry>::Element
		{
			enum { ELEMENTS = 50 };

			Label  label;

			Number_of_bytes                         _ram_quota { };
			size_t                                  _cap_quota { };

			Ring_buffer<Number_of_bytes, ELEMENTS>  _ram       { };
			Ring_buffer<size_t, ELEMENTS>           _caps      { };

			Entry(Label lbl) : label{lbl} { }

			void update(const Number_of_bytes ram_quota, const size_t cap_quota,
			            const Number_of_bytes ram_used,  const size_t caps_used)
			{
				_ram_quota = ram_quota;
				_cap_quota = cap_quota;
				_ram.push(ram_used);
				_caps.push(caps_used);
			}

			Number_of_bytes average_ram() const {

				size_t value { 0 };
				for (size_t i=0; i<_ram.data_points(); ++i) {
					value += _ram.lookup(i);
				}

				return Number_of_bytes { value / _ram.data_points() };
			}

			size_t average_caps() const {

				size_t value { 0 };
				for (size_t i=0; i<_caps.data_points(); ++i) {
					value += _caps.lookup(i);
				}

				return Number_of_bytes { value / _ram.data_points() };
			}

			Number_of_bytes last_ram() const {

				return _ram.lookup(_ram.data_points());
			}

			size_t last_caps() const {

				return _caps.lookup(_caps.data_points());
			}

		};

		Allocator&  _alloc;
		List<Entry> _entries { };

		Number_of_bytes _ram_quota { };
		Number_of_bytes _ram       { };
		size_t          _cap_quota { };
		size_t          _caps      { };

		Entry *_lookup(const Label& label) {

			for (Entry* e = _entries.first(); e; e = e->next())
				if (e->label == label)
					return e;

			return nullptr;
		}

		void _for_each_child(Xml_generator&, const bool);

		void _child_ram (Xml_generator&, const Entry*);
		void _child_caps(Xml_generator&, const Entry*);

		void _init_ram  (Xml_generator&);
		void _init_caps (Xml_generator&);

		inline void _insert_vbox_strecth_attribute(Xml_generator& xml) {
			xml.attribute("west", "yes");
			xml.attribute("east", "yes");
		}

		inline void _insert_vbox_left_attribute(Xml_generator& xml) {
			xml.attribute("west", "yes");
		}

		static inline void _insert_name_attribute(Xml_generator& xml) {
			static size_t id{0};
			xml.attribute("name", id++);
		}

	public:

		Data_registry(Genode::Allocator& alloc) : _alloc{alloc} { }

		void update_init(const Number_of_bytes ram_quota, const size_t caps_quota,
		                 const Number_of_bytes ram_used, const size_t caps_used) { 

			_ram_quota = ram_quota;
			_ram       = ram_used;
			_cap_quota = caps_quota;
			_caps      = caps_used;
		}

		void update(const Label label,
		            const Number_of_bytes ram_quota, const size_t caps_quota,
		            const Number_of_bytes ram_used, const size_t caps_used) { 

			Entry* entry = _lookup(label);

			if (!entry) {
				entry = new (_alloc) Entry{label};
				_entries.insert(entry);
			}

			entry->update(ram_quota, caps_quota, ram_used, caps_used);
		}

		Genode::String<8> string(unsigned percent, unsigned rest) {
			return Genode::String<8> (percent < 10 ? "  " : (percent < 100 ? " " : ""),
			                          percent, ".", rest < 10 ? "0" : "", rest, "%");
		}

		template <typename FN>
		void for_each(FN const &fn) {

			for (Entry* e = _entries.first(); e; e = e->next())
				fn(e);
		}

		void generate(Xml_generator&, const bool);

};

