/*
 * \brief  list to manage packets to start
 * \author Pirmin Duss
 * \date   2020-04-22
 *
 */

#pragma once

/* Genode includes */
#include <base/heap.h>
#include <base/session_label.h>
#include <util/list.h>
#include <util/string.h>
#include <util/xml_node.h>


namespace Heartbeat_monitor {

	using namespace Genode;

	using Start_name = Genode::String<128>;

	enum class Failure {
		Caps_request = 1,
		Cpu_fault    = 2,
		Heartbeat    = 4,
		Ram_request  = 8,
		Seg_fault    = 16,
	};

	struct Failed_component;
	class Slice;
	class Slice_list;
}


struct Heartbeat_monitor::Failed_component : public List<Failed_component>::Element
{
	Session_label const label;
	size_t              caps_count      { 0 };
	size_t              cpu_count       { 0 };
	size_t              heartbeat_count { 0 };
	size_t              ram_count       { 0 };
	size_t              segfault_count  { 0 };

	Failed_component(Session_label const &label) :
		label { label }
	{ }
};


class Heartbeat_monitor::Slice : public List<Slice>::Element
{
	private:

		Allocator              &_alloc;
		Start_name const        _name;
		size_t                  _deploy_version;
		size_t                  _internal_version;
		bool                    _restart;
		size_t                  _configured_caps;
		size_t                  _dynamic_caps;
		Number_of_bytes         _configured_ram;
		Number_of_bytes         _dynamic_ram;
		List<Failed_component>  _failed { };

		Failed_component *_find(Session_label const &label)
		{
			for (auto *e = _failed.first(); e != nullptr; e = e->next()) {
				if (e->label == label) {
					return e;
				}
			}
			return nullptr;
		}

		void _add_or_update_failed(Session_label const &label, Failure kind)
		{
			Failed_component *fail { _find(label) };
			if (!fail) {
				fail = new (_alloc) Failed_component { label };
				_failed.insert(fail);
			}
			switch (kind) {
				case Failure::Caps_request:
					++fail->caps_count;
					break;
				case Failure::Cpu_fault:
					++fail->cpu_count;
					break;
				case Failure::Heartbeat:
					++fail->heartbeat_count;
					break;
				case Failure::Ram_request:
					++fail->ram_count;
					break;
				case Failure::Seg_fault:
					++fail->segfault_count;
					break;
			}
		}

	public:

		Slice(Allocator &alloc, Start_name const &name) :
			_alloc            { alloc },
			_name             { name },
			_deploy_version   { 0 },
			_internal_version { 0 },
			_restart          { false },
			_configured_caps  { 0 },
			_dynamic_caps     { 0 },
			_configured_ram   { 0 },
			_dynamic_ram      { 0 }
		{ }

		Start_name name()             const { return _name; }
		size_t     deploy_version()   const { return _deploy_version; }
		size_t     internal_version() const { return _internal_version; }
		size_t     combined_version() const { return _deploy_version + _internal_version; }
		bool       restart()          const { return _restart; }

		void update_deploy_version(size_t deploy_version)
		{
			_deploy_version = deploy_version;
		}

		void update_internal_version()
		{
			if (_restart) {
				_restart = false;
				++_internal_version;
			}
		}

		void mark_restart(Session_label const &label, Failure kind)
		{
			_add_or_update_failed(label, kind);
			_restart = true;
		}

		template<typename FUNC>
		void for_each(FUNC fun)
		{
			for (auto e = _failed.first(); e != nullptr; e = e->next()) {
				fun(*e);
			}
		}

		size_t          configured_caps() const                    { return _configured_caps; }
		void            configured_caps(size_t const caps)         { _configured_caps = caps; }
		Number_of_bytes configured_ram() const                     { return _configured_ram; }
		void            configured_ram(Number_of_bytes const &ram) { _configured_ram = ram; }

		/**
		 * Get the dynamic assigned caps of this Slice object.
		 * Maximum one Slice object in Slice_list should hold a value other than 0.
		 */
		size_t dynamic_caps() const { return _dynamic_caps; }

		/**
		 * Set the dynamic caps of this Slice object.
		 * Maximum one Slice object in Slice_list should hold a value other than 0.
		 */
		void dynamic_caps(size_t const caps) { _dynamic_caps = caps; }

		/**
		 * Get the dynamic assigned RAM of this Slice object.
		 * Maximum one Slice object in Slice_list should hold a value other than 0.
		 */
		Number_of_bytes dynamic_ram() const { return _dynamic_ram; }

		/**
		 * Set the dynamic RAM of this Slice object.
		 * Maximum one Slice object in Slice_list should hold a value other than 0.
		 */
		void dynamic_ram(Number_of_bytes const &ram) { _dynamic_ram = ram; }
};


class Heartbeat_monitor::Slice_list
{
	private:

		Allocator            &_alloc;
		Genode::List<Slice>   _slices { };

	public:

		Slice_list(Allocator &alloc) :
			_alloc { alloc }
		{ }

		virtual ~Slice_list();

		Slice *add(Xml_node const &);
		Slice *find(Session_label const &);

		bool restart_required() const;

		void remove_outdated(Xml_node const &);
		void mark_restart(Start_name const &, Session_label const &, Failure);

		bool empty() const { return _slices.first() == nullptr; }

		template <typename FUNC>
		void for_each(FUNC func)
		{
			for (auto e = _slices.first(); e != nullptr; e = e->next()) {
				func(*e);
			}
		}
};
