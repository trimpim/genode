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

#include "inotify.h"

/* Genode includes */
#include <base/log.h>

/* linux includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>


namespace Lx_fs
{
	enum {
		STACK_SIZE     = 4096,
		EVENT_SIZE     = (sizeof (struct inotify_event)),
		EVENT_BUF_LEN  = (1024 * (EVENT_SIZE + 16))
	};
}


Lx_fs::Inotify::Inotify(Env& env) :
	Thread { env, "inotify", STACK_SIZE }
{
	_fd = inotify_init();

	if (0 > _fd) {
		throw Init_notify_failed { };
	}

	start();
}


void Lx_fs::Inotify::entry()
{
	char   buffer[EVENT_BUF_LEN];
	while (true) {
		Genode::error("    >>>>>> read ");
		ssize_t length { read(_fd, buffer, EVENT_BUF_LEN) };
		ssize_t  pos    { 0 };

		Genode::error("    >>>>>> length ",length);

		while (pos < length) {
			struct inotify_event* event = (struct inotify_event*) &buffer[pos];

			Genode::warning("    >>>>>> ",event,"   mask       = ",Genode::Hex{event->mask});
			Genode::warning("    >>>>>> ",event,"   IN_ISDIR   = ",Genode::Hex{IN_ISDIR});
			Genode::warning("    >>>>>> ",event,"   IN_MODIFY  = ",Genode::Hex{IN_MODIFY});
			Genode::warning("    >>>>>> ",event,"   IN_CREATE  = ",Genode::Hex{IN_CREATE});
			Genode::warning("    >>>>>> ",event,"   IN_DELETE  = ",Genode::Hex{IN_DELETE});
			Genode::warning("    >>>>>> ",event,"   IN_OPEN    = ",Genode::Hex{IN_OPEN});
			Genode::warning("    >>>>>> ",event,"   IN_IGNORED = ",Genode::Hex{IN_IGNORED});

			if ((event->mask & IN_MODIFY) && !(event->mask & IN_ISDIR)) {
				Genode::error("file modified ", Genode::Cstring { event->name });
			}
			pos += EVENT_SIZE + event->len;
		}
	}
}


int Lx_fs::Inotify::add_watch(const char* path, Listener&)
{
	Genode::warning("  >> name:",path);
	return inotify_add_watch(_fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);
}


void Lx_fs::Inotify::remove_watch(const int fd)
{
   inotify_rm_watch(_fd, fd);
}


/*
//actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.
  while ( i < length ) {
  struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
  if ( event->len ) {
      if ( event->mask & IN_CREATE ) {
        if ( event->mask & IN_ISDIR ) {
          printf( "New directory %s created.\n", event->name );
        }
        else {
          printf( "New file %s created.\n", event->name );
        }
      }
      else if ( event->mask & IN_DELETE ) {
        if ( event->mask & IN_ISDIR ) {
          printf( "Directory %s deleted.\n", event->name );
        }
        else {
          printf( "File %s deleted.\n", event->name );
        }
      }
    }
    i += EVENT_SIZE + event->len;
  }
  //removing the “/tmp” directory from the watch list.
   inotify_rm_watch( fd, wd );

  //closing the INOTIFY instance
   close( fd );
*/
