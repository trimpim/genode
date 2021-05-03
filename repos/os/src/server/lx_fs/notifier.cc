/*
 * \brief  inotify handling for underlying file system.
 * \author Pirmin Duss
 * \date   2020-06-17
 */

/*
 * Copyright (C) 2013-2020 Genode Labs GmbH
 * Copyright (C) 2020 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/log.h>

/* libc includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

/* local includes */
#include "notifier.h"


namespace Lx_fs
{
	enum {
		STACK_SIZE     = 8 * 1024,
		EVENT_SIZE     = (sizeof (struct inotify_event)),
		EVENT_BUF_LEN  = (1024 * (EVENT_SIZE + NAME_MAX + 1)),
		PARALLEL_NOTIFICATIONS = 4,
	};
}


/* do not leak internal function in to global namespace */
namespace
{
	using namespace Lx_fs;

	template <typename T, typename Allocator>
	T *remove_from_list(Genode::List<T> &list, T *node, Allocator &alloc)
	{
		T *next { node->next() };
		list.remove(node);
		destroy(alloc, node);
		return next;
	}


	bool is_dir(char const *path)
	{
		struct stat s;
		int ret = lstat(path, &s);
		if (ret == -1)
			return false;

		if (S_ISDIR(s.st_mode))
			return true;

		return false;
	}


	Path_string get_directory(Path_string const &path)
	{
		Path_string directory;
		if (is_dir(path.string())) {
			// make sure there is a '/' at the end of the path
			if (path.string()[path.length() - 2] == '/')
				directory = path;
			else
				directory = Path_string { path, '/' };
		} else {
			size_t pos = 0;
			for (size_t i = 0; i < path.length() - 1; ++i) {
				if (path.string()[i] == '/')
					pos = i;
			}
			directory = Genode::Cstring { path.string(), pos + 1 };
		}

		return directory;
	}


	Path_string get_filename(Path_string const &path)
	{
		Path_string filename;
		if (is_dir(path.string()))
			return { };

		size_t pos = 0;
		for (size_t i = 0; i < path.length() - 1; ++i) {
			if (path.string()[i] == '/')
				pos = i;
		}

		// if '/' is the last symbol we do not need a filename
		if (pos != path.length() - 2)
			filename = Genode::Cstring { path.string() + pos + 1 };

		return filename;
	}

}  /* anonymous namespace */


Lx_fs::Os_path::Os_path(const char *fullname)
:
	full_path { fullname },
	directory { get_directory(full_path) },
	filename  { get_filename(full_path) }
{ }


bool Lx_fs::Notifier::_watched(char const *path) const
{
	for (Entry const *e = _watched_nodes.first(); e != nullptr; e = e->next()) {
		if (e->path.full_path == path)
			return true;
	}

	return false;
}


void Lx_fs::Notifier::_add_to_watched(char const *fullname)
{
	Os_path path { fullname };
	for (Entry *e = _watched_nodes.first(); e != nullptr; e = e->next()) {
		if (e->path.directory == path.directory) {
			Entry *elem { new (_heap) Entry { e->watch_fd, path } };
			_watched_nodes.insert(elem);
			return;
		}
	}

	auto watch_fd { inotify_add_watch(_fd, path.directory.string(),
	                                  IN_CREATE | IN_DELETE | IN_CLOSE_WRITE) };

	if (watch_fd > 0) {
		Entry *elem { new (_heap) Entry { watch_fd, path } };
		_watched_nodes.insert(elem);
	}
}


int Lx_fs::Notifier::_add_cap(char const *path, Signal_context_capability cap)
{
	for (Entry *e = _watched_nodes.first(); e != nullptr; e = e->next()) {
		if (e->path.full_path == path) {
			Cap_entry *c { new (_heap) Cap_entry { cap } };
			e->add_capabilty(c);
			return e->watch_fd;
		}
	};

	throw File_system::Lookup_failed { };
}


void Lx_fs::Notifier::_add_notify(Signal_context_capability cap)
{
	{
		Mutex::Guard guard { _notify_queue_mutex };

		for (auto const *e = _notify_queue.first(); e != nullptr; e = e->next()) {
			if (e->cap == cap) {
				return;
			}
		}

		auto *entry { new (_heap) Cap_entry { cap } };
		_notify_queue.insert(entry);
	}

	if (!_notify_timer_running) {
		_notify_timer_running = true;
		_notify_timer.trigger_once(5000);
	}
}


void Lx_fs::Notifier::_process_notify()
{
	Mutex::Guard guard { _notify_queue_mutex };
	_notify_timer_running = false;

	/**
	 * limit amount of watch events sent at the same time,
	 * to prevent an overflow of the packet ack queue of the
	 * File_system session.
	 */
	int cnt { 0 };
	Cap_entry *e { _notify_queue.first() };
	for (; e != nullptr && cnt < PARALLEL_NOTIFICATIONS; ++cnt) {
		Signal_transmitter {e->cap}.submit();
		e = remove_from_list(_notify_queue, e, _heap);
	}

	if (_notify_queue.first() != nullptr) {
		_notify_timer_running = true;
		_notify_timer.trigger_once(5000);
	}
}


Lx_fs::Notifier::Notifier(Env &env)
:
	Thread { env, "inotify", STACK_SIZE },
	_env   { env }
{
	_notify_timer.sigh(_notify_handler);
	_fd = inotify_init();

	if (0 > _fd) {
		throw Init_notify_failed { };
	}

	start();
}


Lx_fs::Notifier::~Notifier()
{
	/* do not notify the elements */
	for (auto *e = _notify_queue.first(); e != nullptr; ) {
		e = remove_from_list(_notify_queue, e, _heap);
	}

	for (auto *e = _watched_nodes.first(); e != nullptr; ) {
		e = remove_from_list(_watched_nodes, e, _heap);
	}

	close(_fd);
}


Lx_fs::Notifier::Entry *Lx_fs::Notifier::_remove_node(Entry *node)
{
	int    watch_fd   { node->watch_fd };
	Entry *next       { remove_from_list(_watched_nodes, node, _heap) };
	bool   nodes_left { false };

	Entry const *e { _watched_nodes.first() };
	for (; e != nullptr && !nodes_left; e = e->next()) {
		nodes_left = e->watch_fd == watch_fd;
	}

	if (!nodes_left) {
		inotify_rm_watch(_fd, watch_fd);
	}

	return next;
}


void Lx_fs::Notifier::_handle_modify_file(inotify_event *event)
{
	for (Entry *e = _watched_nodes.first(); e != nullptr; e = e->next()) {
		if (e->watch_fd == event->wd && (e->path.filename == event->name || e->path.is_dir())) {
			e->notify_all([this] (Signal_context_capability cap) {
				_add_notify(cap);
			});
		}
	}
}


void Lx_fs::Notifier::_remove_empty_watches()
{
	for (Entry *e = _watched_nodes.first(); e != nullptr; ) {
		if (e->empty()) {
			e = _remove_node(e);
		} else {
			e = e->next();
		}
	}
}


void Lx_fs::Notifier::entry()
{
	struct inotify_event *event { nullptr };

	char buffer[EVENT_BUF_LEN] { 0 };
	while (true) {
		ssize_t const length { read(_fd, buffer, EVENT_BUF_LEN) };
		ssize_t       pos    { 0 };

		while (pos < length) {
			event = reinterpret_cast<struct inotify_event*>(&buffer[pos]);

			/* file modified? */
			if (event->mask & IN_CLOSE_WRITE) {
				Mutex::Guard guard { _watched_nodes_mutex };
				_handle_modify_file(event);
			}

			/* watch descriptor no longer watched */
			else if (event->mask & IN_DELETE) {
				Mutex::Guard guard { _watched_nodes_mutex };
				_handle_modify_file(event);
			}

			else if (event->mask & IN_Q_OVERFLOW) {
				Genode::error("Linux event queue overflow");
			}

			pos += EVENT_SIZE + event->len;
		}
	}
}


int Lx_fs::Notifier::add_watch(const char* path, Signal_context_capability cap)
{
	Mutex::Guard guard { _watched_nodes_mutex };

	if (!_watched(path)) {
		_add_to_watched(path);
	}

	return _add_cap(path, cap);
}


void Lx_fs::Notifier::remove_watch(char const *path, Signal_context_capability cap)
{
	{
		Mutex::Guard  guard { _notify_queue_mutex };
		auto         *e     { _notify_queue.first() };
		while (e != nullptr) {
			if (e->cap == cap) {
				auto *tmp { e };
				e = e->next();
				_notify_queue.remove(tmp);
				destroy(_heap, tmp);
			} else {
				e = e->next();
			}
		}
	}

	Mutex::Guard guard { _watched_nodes_mutex };

	for (Entry *e = _watched_nodes.first(); e != nullptr; e = e->next()) {
		if (e->path.full_path == path) {
			e->remove_capability(_heap, cap);
		}
	}

	_remove_empty_watches();
}
