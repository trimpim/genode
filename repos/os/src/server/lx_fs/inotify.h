/*
 * \brief  inotify handling for underlying filesystem.
 * \author Pirmin Duss
 * \date   2019-12-04
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


#ifndef _INOTIFY_H_
#define _INOTIFY_H_

/* Genode includes */
#include <base/exception.h>
#include <base/thread.h>
#include <file_system/listener.h>
#include <util/reconstructible.h>


namespace Lx_fs {

	using Genode::Thread;
	using Genode::Env;
	using Genode::Id_space;

	class Init_notify_failed : public Genode::Exception { };

	class Inotify;

	using Inotifier = Genode::Constructible<Inotify>;
	using Listener  = Genode::Constructible<File_system::Listener>;
}


class Lx_fs::Inotify : public Thread
{
	private:

		int _fd { -1 };
		Id_space<Listener>  _open_listener_registry { };

		void entry() override;

	public:

		Inotify(Env& env);

		~Inotify() = default;

		int add_watch(const char*, Listener&);
		void remove_watch(const int fd);
};

#endif  // _INOITIFY_H_
